enum State_cm {
    EMPTYCM = 0,
    READYCM = 1,
    BUSYCM = 2
};

struct Chunk {
    void* ptr;
    int offset;
    int size;
    int refcount;
    char state; //Need to be tested if it is required to add 3 bytes
};

struct Spool {
    unsigned char count;
    struct Chunk data[0];
};

struct Pool {
    struct Spool** data;

    unsigned char spool_size;
    int spool_count;
    int spool_max;
};


#define INIT_SPOOL_COUNT 16


struct Pool* init_pool();
ssize_t destruct_pool(struct Pool* pool);
struct Chunk* allocate_chunk(struct Pool* pool, void* ptr, off_t offset, size_t size);
ssize_t deref_chunk(struct Pool* pool, struct Chunk* item);
