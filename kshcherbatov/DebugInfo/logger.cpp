//
// Created by kir on 12.03.16.
//

#include "logger.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

struct ring_buff_t *Logging_system = NULL;

#define LOG_MSG_MAX_AMOUNT 128
#define FLUSH_THREAD_SLEEP_TIME 10000
#define LOG_FILENAME_MAX_LEN 255

#define DEBUG_MSG_RING_BUFF_INTERNAL

#ifdef DEBUG_MSG_RING_BUFF_INTERNAL
#define DOUT(fmt, args...) do {fprintf (stdout, fmt, ## args);} while (0)
#else
#define DOUT(fmt, args...)
#endif

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


enum SEM_CONTROLS {
    SEM_CONTROL_FLUSH,
    SEM_CONTROL_INITIAL_SYSTEM,
    SEM_CONTROL_PS_NUM,
    SEM_CONTROL_HAS_DAEMON,
    SEM_CONTROL_DAEMON_IS_RESTARTING,
    SEM_CONTROL_SIZE
};

enum SEM_ACTIONS {
    SEM_ACTION_CATCH = -1,
    SEM_ACTION_WAIT0 = 0,
    SEM_ACTION_RELAX = 1
};


struct log_msg_t {
    bool is_ready_to_flush;
    char msg[LOG_MSG_MAX_SIZE];
};

struct ring_buff_shared_t {
    uint32_t msg_buff_write_index;
    uint32_t msg_buff_flushstart_index;
    log_msg_t msg_buff[LOG_MSG_MAX_AMOUNT];
};

struct ring_buff_t {
    int shared_fd;
    int logfile_fd;
    int sems_fd;
    pthread_mutex_t flush_buffer_thread_shouldwork_mtx;
    struct ring_buff_shared_t *shared;
};


struct ring_buff_t *ring_buff_construct(const char log_filename[]);
void ring_buff_destruct(struct ring_buff_t *ring_buff);
static void ring_buff_destruct_all(struct ring_buff_t *ring_buff);
static inline void ring_buff_fork_daemon(struct ring_buff_t *ring_buff, bool first_init);
static inline void ring_buff_validate_daemon(struct ring_buff_t *ring_buff);
static inline int sem_act(const int sem_id, const unsigned short num, const short int arg,  const short flag);
static inline bool ring_buff_isok(const struct ring_buff_t *ring_buff);
static int ring_buff_flush_to_file(struct ring_buff_t *ring_buff);
int ring_buff_push_msg(struct ring_buff_t *ring_buff, const char msg[]);


struct ring_buff_t *ring_buff_construct(const char log_filename[]) {
    DOUT("# %d: Called ring_buff_create( \"%s\" )\n", getpid(), log_filename);
    assert(log_filename);

    if (strlen(log_filename) >= LOG_FILENAME_MAX_LEN)
        handle_error("Unsupported log_filename length");

    struct ring_buff_t *ring_buff = (struct ring_buff_t *)malloc(sizeof(struct ring_buff_t));
    if (!ring_buff) {
        free((void *)ring_buff);
        handle_error("Failed space allocating");
    }

    if ((ring_buff->logfile_fd = open(log_filename, O_WRONLY|O_CREAT|O_SYNC, 0666)) < 0) {
        free((void *)ring_buff);
        handle_error("Failed open to write log file");
    }

    key_t key;
    if((key = ftok(log_filename, 0)) < 0) {
        close(ring_buff->logfile_fd);
        free((void *)ring_buff);
        handle_error("Failed ftok");
    }

    bool mem_were_attached = false;
    if ((ring_buff->shared_fd = shmget(key, sizeof(struct ring_buff_shared_t), 0666|IPC_CREAT|IPC_EXCL)) < 0)
    if (errno != EEXIST) {
        close(ring_buff->logfile_fd);
        free((void *)ring_buff);
        handle_error("Failed shmget");
    } else {
        if ((ring_buff->shared_fd = shmget(key, sizeof(struct ring_buff_shared_t), 0)) < 0) {
            close(ring_buff->logfile_fd);
            free((void *)ring_buff);
            handle_error("Failed shmget");
        } else
            mem_were_attached = true;
    }

    if (0)
    {
        ring_buff_destruct_all(ring_buff);
        exit(EXIT_FAILURE);
    }


    if ((ring_buff->shared = (struct ring_buff_shared_t *)shmat(ring_buff->shared_fd, NULL, 0))
        == ((struct ring_buff_shared_t *)-1)) {
        shmctl(ring_buff->shared_fd, IPC_RMID, NULL);
        close(ring_buff->logfile_fd);
        free((void *)ring_buff);
        handle_error("Failed shmat");
    }

    if ((ring_buff->sems_fd = semget(key, SEM_CONTROL_SIZE, 0666|IPC_CREAT)) < 0) {
        shmdt(ring_buff->shared);
        shmctl(ring_buff->shared_fd, IPC_RMID, NULL);
        close(ring_buff->logfile_fd);
        free((void *)ring_buff);
        handle_error("Failed semget");
    }

    sem_act(ring_buff->sems_fd, SEM_CONTROL_PS_NUM, SEM_ACTION_RELAX, SEM_UNDO);

    if (!mem_were_attached) {
        DOUT("# %d:\t Memory was CREATED. Initialize it.\n", getpid());

        ring_buff_fork_daemon(ring_buff, true);
    } else {
        DOUT("# %d:\t Memory was ATTACHED.\n", getpid());
    }

    // SEM_CONTROL_INITIAL_SYSTEM unlocks in ring_buff_fork_daemon()
    sem_act(ring_buff->sems_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_CATCH, SEM_UNDO);
    DOUT("# %d:\t looks good!\n", getpid());
    sem_act(ring_buff->sems_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_RELAX, 0);

    DOUT("# %d:\t __ring_buff_construct is going to exit\n", getpid());
    return ring_buff;
}

static inline void ring_buff_validate_daemon(struct ring_buff_t *ring_buff) {
    if ((sem_act(ring_buff->sems_fd, SEM_CONTROL_HAS_DAEMON, SEM_ACTION_WAIT0, IPC_NOWAIT) == 0) &&
            sem_act(ring_buff->sems_fd, SEM_CONTROL_DAEMON_IS_RESTARTING, SEM_ACTION_CATCH, IPC_NOWAIT) == 0) {
        ring_buff_fork_daemon(ring_buff, false);
    }
}

static void ring_buff_fork_daemon(struct ring_buff_t *ring_buff, bool first_init) {
    assert(ring_buff);

    int pid = fork();
    if (pid < 0) {
        ring_buff_destruct_all(ring_buff);
        handle_error("Failed fork");
    } else if (pid == 0) {
        DOUT("# %d:\t Start Logger daemon\n", getpid());

        sem_act(ring_buff->sems_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_CATCH, IPC_NOWAIT);

        if (first_init) {
            ring_buff->shared->msg_buff_flushstart_index = 0;
            ring_buff->shared->msg_buff_write_index = 0;
            for (register int i = 0; i < LOG_MSG_MAX_AMOUNT; i++)
                ring_buff->shared->msg_buff[i].is_ready_to_flush = false;

            sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_RELAX, 0);
        }

        sem_act(ring_buff->sems_fd, SEM_CONTROL_DAEMON_IS_RESTARTING, SEM_ACTION_RELAX, 0);
        sem_act(ring_buff->sems_fd, SEM_CONTROL_HAS_DAEMON, SEM_ACTION_RELAX, SEM_UNDO);

        sem_act(ring_buff->sems_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_RELAX, 0);

        // wait while other processes exists
        int end_loop_condition; // succeeded, sem_ect returns 0
        do {
            if (ring_buff_flush_to_file(ring_buff) < 0) {
                perror("Failed write log");
                break;
            }
            usleep(FLUSH_THREAD_SLEEP_TIME);
            end_loop_condition = sem_act(ring_buff->sems_fd, SEM_CONTROL_PS_NUM, SEM_ACTION_WAIT0, IPC_NOWAIT);
        } while (end_loop_condition != 0);

        DOUT("# %d:\t Daemon is currently not useful\n", getpid());

        ring_buff_flush_to_file(ring_buff);
        ring_buff_destruct_all(ring_buff);

        DOUT("# %d:\t Daemon process is going exit\n", getpid());

        exit(EXIT_SUCCESS);
    }
}

static inline int sem_act(const int sem_id, const unsigned short num, const short int arg,  const short flag) {
    assert(num < SEM_CONTROL_SIZE);

    struct sembuf sops;
    sops.sem_op = arg;
    sops.sem_flg = flag;
    sops.sem_num = num;

    if (semop(sem_id, &sops, 1) < 0)
        return errno;

    return 0;
}

static inline bool ring_buff_isok(const struct ring_buff_t *ring_buff) {
    assert(ring_buff);
    return (ring_buff->shared_fd >= 0)  && (ring_buff->sems_fd >= 0) &&
           (ring_buff->shared->msg_buff_write_index < LOG_MSG_MAX_AMOUNT) &&
           (ring_buff->shared->msg_buff_flushstart_index < LOG_MSG_MAX_AMOUNT);
}

static int ring_buff_flush_to_file(struct ring_buff_t *ring_buff) {
    //DOUT("# %d: Called ring_buff_flush_to_file( [ %p ] )\n", getpid(), (void*)ring_buff);
    assert(ring_buff);
    assert(ring_buff_isok(ring_buff));

    struct ring_buff_shared_t *shared = ring_buff->shared;

    int logfile_fd = ring_buff->logfile_fd;
    uint32_t msg_buff_index = shared->msg_buff_flushstart_index;

    while (true) {
        bool is_ready_to_flush;
        sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_CATCH, SEM_UNDO);

        is_ready_to_flush = shared->msg_buff[msg_buff_index].is_ready_to_flush;
        if (is_ready_to_flush)
            shared->msg_buff[msg_buff_index].is_ready_to_flush = false;

        sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_RELAX, 0);

        const char *msg = shared->msg_buff[msg_buff_index].msg;
        msg_buff_index = (msg_buff_index + 1) % LOG_MSG_MAX_AMOUNT;
        if (!is_ready_to_flush)
            break;

        size_t str_len = strlen(msg);

        if (write(logfile_fd, msg, str_len) < (int)str_len) {
            perror("write");
            return -1;
        }
    }

    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_CATCH, SEM_UNDO);
    shared->msg_buff_flushstart_index = msg_buff_index;
    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_RELAX, 0);

    return 0;
}

void ring_buff_destruct(struct ring_buff_t *ring_buff) {
    DOUT("# %d: Called ring_buff_destruct( [ %p ] )\n", getpid(), (void*)ring_buff);
    if (!ring_buff)
        return;
    // destruct just a single program, not a daemon

    int sem_fd = ring_buff->sems_fd;
    int logfile_fd = ring_buff->logfile_fd;
    sem_act(sem_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_CATCH, IPC_NOWAIT);

    shmdt(ring_buff->shared);
    ring_buff->shared = NULL;
    ring_buff->sems_fd = -1;
    ring_buff->shared_fd = -1;
    ring_buff->logfile_fd = -1;
    free((void *)ring_buff);
    close(logfile_fd);

    sem_act(sem_fd, SEM_CONTROL_INITIAL_SYSTEM, SEM_ACTION_RELAX, 0);
}

static void ring_buff_destruct_all(struct ring_buff_t *ring_buff) {
    DOUT("# %d: Called ring_buff_destruct_all( [ %p ] )\n", getpid(), (void*)ring_buff);
    assert(ring_buff);

    shmdt(ring_buff->shared);
    ring_buff->shared = NULL;
    shmctl(ring_buff->shared_fd, IPC_RMID, NULL);
    close(ring_buff->logfile_fd);
    semctl(ring_buff->sems_fd, 0, IPC_RMID, 0);
    ring_buff->sems_fd = -1;
    ring_buff->shared_fd = -1;

    free((void *) ring_buff);
}

int ring_buff_push_msg(struct ring_buff_t *ring_buff, const char msg[]) {
    //DOUT("# %d: Called ring_buff_push_msg( [ %p ], \"%s\" )\n", getpid(), (void*)ring_buff, msg);
    assert(msg);
    assert(ring_buff);
    assert(ring_buff_isok(ring_buff));

    ring_buff_validate_daemon(ring_buff);

    struct ring_buff_shared_t *shared = ring_buff->shared;

    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_CATCH, SEM_UNDO);
    uint32_t msg_buff_used_local = shared->msg_buff_write_index;
    shared->msg_buff_write_index = (shared->msg_buff_write_index + 1) % LOG_MSG_MAX_AMOUNT;

    shared->msg_buff[msg_buff_used_local].is_ready_to_flush = false;
    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_RELAX, 0);

    strncpy(shared->msg_buff[msg_buff_used_local].msg, msg, LOG_MSG_MAX_SIZE);

    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_CATCH, SEM_UNDO);
    shared->msg_buff[msg_buff_used_local].is_ready_to_flush = true;
    sem_act(ring_buff->sems_fd, SEM_CONTROL_FLUSH, SEM_ACTION_RELAX, 0);

    return 0;
}


void __init_log_system(int argc, char * argv[]) {
    Logging_system = ring_buff_construct(LOG_FILENAME);
    DOUT("# %d: __init_log_system finished\n", getpid());
    assert(Logging_system);
}

void __deinit_log_system(void) {
    ring_buff_destruct(Logging_system);
    Logging_system = NULL;
    DOUT("# %d: __deinit_log_system finished\n", getpid());
}
