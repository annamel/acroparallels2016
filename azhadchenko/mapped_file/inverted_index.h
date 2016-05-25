enum State_ii {
    EMPTYII = 0,
    READYII = 1,
    BUSYII = 2
};

struct Ii_element {
    void* item;
    size_t pos_inside;
    struct Ii_element* next;
    char state;
};

struct Block {
    size_t count;
    struct Ii_element data[0];
};

struct Ii_manager {
    struct Block** data;

    size_t block_count;
    size_t block_size;
    size_t block_max;
};

#define INIT_BLOCK_COUNT 16

struct Inverted_index {
    struct Ii_manager* manager;
    struct Ii_element* data[0];
};



struct Ii_manager* init_ii_m();
ssize_t destruct_ii_m(struct Ii_manager* manager);
struct Ii_element* allocate_element(struct Ii_manager* manager, void* item, size_t pos_inside);
ssize_t destruct_element(struct Ii_manager* manager, struct Ii_element* item);
struct Inverted_index* init_ii(size_t size);
ssize_t destruct_ii(struct Inverted_index* ii);
ssize_t add_item(struct Inverted_index* ii, void* item, size_t start, size_t until);
ssize_t delete_item(struct Inverted_index* ii, void* item, size_t start, size_t until);
