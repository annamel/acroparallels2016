#define ERROR(MESSAGE, type) \
            do { fprintf(stderr, MESSAGE "\n"); return (type)-1;} while(0)

enum CONSTS {
    ALREADY_EXIST = -1,
    HEAD = -1
};

struct Node {
    struct Node* previous;
    struct Node* next;
    void* item;
};

struct Node* node_init(void* value) {

    struct Node* node = (struct Node*)calloc(1, sizeof(struct Node));
    if(!node)
        ERROR("Out of memory", void*);

    node -> item = value;

    return node;
}

int node_add(struct Node* current, struct Node* item) {

    if(!current || !item)
        return 0;

    while(current -> next)
        current = current -> next;

    current -> next = item;
    item -> previous = current;

    return 0;
}

struct Node* search_node(struct Node* current, void* item) {

    if(!current || !item)
        return 0;

    while(current -> item != item) {
        if(!(current -> next))
            break;

        current = current -> next;
    }

    return current;
}

int node_destroy(struct Node* node) {

    if(!node)
        return 0;


    if(node -> previous != (void*)HEAD && node -> previous != 0)
        node -> previous -> next = node -> next;

    if(node -> next)
        node -> next -> previous = node -> previous;

    free(node);

    return 0;
}



int node_destroy_recursive(struct Node* node) {

    if(!node)
        return 0;

    node_destroy_recursive(node -> next);

    free(node);

    return 0;
}


struct HashTable{
    int table_size;
    struct Node** lists;
    uint32_t (*hash_func)(int);
    int (*get_signature)(void*);
};

struct HashTable* Hash_init(int table_size,
                     uint32_t (*hash_func)(int), int (*get_signature)(void*) ) {

    if(!hash_func || !get_signature)
        ERROR("Incorrect arguments", void*);

    struct HashTable* table = (struct HashTable*)calloc(1, sizeof(struct HashTable));
    if(!table)
        ERROR("Out of memory", void*);

    table -> lists = (struct Node**)calloc(table_size, sizeof(struct Node*));
    if(!table -> lists) {
        free(table);
        ERROR("Out of memory", void*);
    }

    table -> hash_func = hash_func;
    table -> get_signature = get_signature;
    table -> table_size = table_size;

    return table;
}

int Hash_destruct(struct HashTable* table) {

    if(!table)
        ERROR("Null pointer encountered", int);

    if(!table -> lists) {
        free(table);
        ERROR("Damaged hashtable found, destroyed", int);
    }

    for(int i = 0; i < table -> table_size; i++) {
        if(table -> lists[i])
            node_destroy_recursive(table -> lists[i]);
    }

    return 0;
}

int add_item(struct HashTable* table, void* item) {

    int hash = 0;
    int tmp = 0;
    struct Node* new_item = 0;
    struct Node* search_res = 0;

    if(!table || !item)
        ERROR("Null pointer encountered", int);

    hash = table -> hash_func(table -> get_signature(item));

    tmp = hash % table -> table_size;

    if(!table -> lists[tmp]) {
        new_item = node_init(item);
        if(new_item == (void*)-1)
            return -1;

        table -> lists[tmp] = new_item;
        return 0;
    }

    search_res = search_node(table -> lists[tmp], item);
    if(search_res -> item != item) {
        new_item = node_init(item);
        if(new_item == (void*)-1)
            return -1;

        node_add(search_res, new_item);
        return 0;
    } else
        return ALREADY_EXIST;
}
