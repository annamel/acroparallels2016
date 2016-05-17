#define malloc(size) memtest_alloc(size, 0)
#define realloc(ptr, size) memtest_realloc(ptr, size, 0)
