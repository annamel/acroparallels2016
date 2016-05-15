#ifndef common_types_h
#define common_types_h

#define DEFAULT_HASH_TABLE_SIZE 1024
#define DEFAULT_PAGE_SIZE 1024
#define DEFAULT_ARRAY_SIZE 1024
#define LOG_FILE_BY_DEFAULT "log.txt"
#define AMOUNT_OF_OPTIONS 5

#include <sys/types.h>
#include <stdint.h>

typedef struct hash_table hash_table_t;
typedef struct chunk chunk_t;
typedef struct ch_pool ch_pool_t;
typedef struct element list_element;
typedef struct elem elem_t;
typedef struct list list_t;
typedef chunk_t* data_t;

static const char *log_types[AMOUNT_OF_OPTIONS] = {"DEBUG:: ",
                                            "INFO:: ",
                                            "WARNING:: ",
                                            "ERROR:: ",
                                            "FATAL:: "};

struct chunk {
    off_t index;
    off_t length;
    unsigned int ref_counter;
    void *data;
    ch_pool_t *ch_pool;
    size_t chunk_size_min;
};

struct ch_pool {
    int fd;
    int prot;
    size_t arrays_cnt;
    chunk_t **pool;
    hash_table_t *h_table;
    list_t *list_zero_ref_count;
    list_t *list_of_free_chunks;
    off_t file_size;
    size_t chunk_size_min;
    int fdd;
};

struct hash_table {
    unsigned int size;
    unsigned is_initialized;
    size_t chunk_size_min;
    list_element** table;
};

struct element {
    list_element* next;
    list_element* prev;
    data_t data;
};

struct elem {
    elem_t *next;
    elem_t *prev;
    data_t data;
};

struct list {
    elem_t *first;
    elem_t *end;
    unsigned int size;
};

#endif

