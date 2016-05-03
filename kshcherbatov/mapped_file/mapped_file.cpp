//
// Created by kir on 29.04.16.
//

#include "mapped_file.h"
#include "logger.h"
#include "hash_table.h"
#include "errno.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <execinfo.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>


const size_t Page_size = (size_t)sysconf(_SC_PAGE_SIZE);
const size_t Chunk_size = Page_size*1024;
const size_t RW_atomic_size = Chunk_size*64;
const size_t Ring_map_cache_size = 64;

struct chunk_t {
    void *mem;
    ssize_t mem_size;
    off_t mem_offset;
    ssize_t ref_cnt;
};

struct mapped_file_t {
    int fd;

    ssize_t file_size;

    struct hash_t chunk_ht;

    bool file_mapped;

    chunk_t cache_chunk;
    size_t cache_mem_av;

    chunk_t *ring_map_cache[Ring_map_cache_size];
    size_t ring_map_cache_w_idx;
};

struct complex_key_t {
    ssize_t size;
    off_t offset;
};

#define VALID_OR_NULL(condition, msg) \
    do {if (!condition) {LOG_ERROR(msg, NULL); assert(!msg); return NULL;}} while (0)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

bool mapped_file_is_ok(struct mapped_file_t *mapped_file);
bool complex_key_is_ok(struct complex_key_t *complex_key);
bool chunk_is_ok(struct chunk_t *chunk);

static chunk_t *chunk_create(off_t offset, size_t size, void *mem);
static complex_key_t *complex_key_clone(const complex_key_t *complex_key);
static size_t pa_size(size_t size);
static size_t ca_size(size_t size);

static void *_mmap(size_t length, int fd, off_t offset);
static int _munmap(void *addr, size_t length);

bool chunk_is_not_in_pool(mapped_file_t *mapped_file, chunk_t *chunk);

size_t hash_func(const size_t key);
bool cmp_func(const size_t key_a, const size_t key_b);
void destruct_internals_func(struct hash_list_t *hash_list);

mf_handle_t mf_open(const char* pathname) {
    LOG_DEBUG("Called mf_open (%s)\n", pathname);
    VALID_OR_NULL(pathname, "mf_open: Function argument pathname is a NULL pointer. Exit.\n");

    mapped_file_t *mapped_file = (mapped_file_t *)malloc(sizeof(mapped_file_t));
    assert(mapped_file);

    int result;

    int fd;
    if ((fd = open(pathname, O_RDWR)) < 0) {
        LOG_ERROR("mapped_file_construct: open: failed open file.\n", NULL);
        free((void *)mapped_file);
        return NULL;
    }
    mapped_file->fd = fd;

    struct stat sb;
    result = fstat(fd, &sb);
    assert(result != -1);
    mapped_file->file_size = (size_t) sb.st_size;

    struct sysinfo info;
    result = sysinfo(&info);
    assert(result == 0);

    mapped_file->cache_mem_av = info.freeram / 128;

    size_t hash_table_size = MAX(8 * mapped_file->file_size / Chunk_size, 10*Page_size);
    hash_init(&mapped_file->chunk_ht, hash_table_size, hash_func, cmp_func, destruct_internals_func);

    memset(mapped_file->ring_map_cache, NULL, Ring_map_cache_size*sizeof(chunk_t *));
    mapped_file->ring_map_cache_w_idx = 0;

    void *mem_ptr = NULL;
    ssize_t mem_size = -1;
    ssize_t mem_offset = -1;

#ifndef STRESS_NO_VIRTUAL_MEM_TEST
    struct rlimit rl;
    if (!getrlimit(RLIMIT_AS, &rl)) {
        mem_offset = 0;
        if (rl.rlim_cur == RLIM_INFINITY) {
            mem_size = (size_t)mapped_file->file_size;
        } else {
            mem_size = MIN((size_t)mapped_file->file_size, rl.rlim_cur);
        }
        mem_ptr = mmap(NULL, (size_t)mem_size, PROT_WRITE, MAP_SHARED, mapped_file->fd, mem_offset);
    }
#endif //STRESS_NO_VIRTUAL_MEM_TEST

    struct chunk_t *chunk_cache = &mapped_file->cache_chunk;
    chunk_cache->ref_cnt = -1; // prevent using as a single chunk: chunk_is_ok - not ok
    chunk_cache->mem = mem_ptr;
    chunk_cache->mem_size = mem_size;
    chunk_cache->mem_offset = mem_offset;
    mapped_file->file_mapped = mem_size >= mapped_file->file_size;

    assert(mapped_file_is_ok(mapped_file));
    return ((mf_handle_t)mapped_file);
}

int mf_close(mf_handle_t mf) {
    LOG_DEBUG("Called mf_close ( [ %p ] )\n", mf);
    assert(mf);

    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));

    close(mapped_file->fd);
    hash_fini(&mapped_file->chunk_ht);

    if (mapped_file->cache_chunk.mem) {
        _munmap(mapped_file->cache_chunk.mem, (size_t)mapped_file->cache_chunk.mem_size);
    }

#ifndef NDEBUG
    mapped_file->fd = -1;
    mapped_file->file_mapped = false;
    mapped_file->file_size = -1;
    mapped_file->cache_chunk.mem = NULL;
    mapped_file->cache_chunk.mem_offset = -1;
    mapped_file->cache_chunk.ref_cnt = -1;
    mapped_file->cache_chunk.mem_size = -1;
#endif //NDEBUG

    return 0;
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle) {
    LOG_DEBUG("Called mf_map (mf = [%p], offset = %u, size = %u, mapmem_handle = [%p])\n",
              mf, offset, size, mapmem_handle);

    assert(mf);
    assert(mapmem_handle);

    if (size == 0) {
        return NULL;
    }

    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));
    assert(mf && size > 0 && offset >= 0 && mapmem_handle);

    if (mapped_file->file_mapped) {
        *mapmem_handle = &mapped_file->cache_chunk;
        return (void *)&(((char *)mapped_file->cache_chunk.mem)[offset]);
    }

    size_t cpa_offset = ca_size((size_t)offset); //it is also pa_
    size_t stair = offset - cpa_offset;
    size_t cpa_size = ca_size(size + stair + Chunk_size);

    complex_key_t key;
    key.offset = cpa_offset;
    key.size = cpa_size;
    assert(complex_key_is_ok(&key));

    chunk_t *chunk = (chunk_t *)hash_find(&mapped_file->chunk_ht, (size_t)&key);
    if (chunk) {
        assert(chunk_is_ok(chunk));
        chunk->ref_cnt++;
        *mapmem_handle = (mf_mapmem_handle_t *)chunk;
        return (void *)&(((char *)chunk->mem)[stair]);
    }

    void *mapped_area = _mmap(cpa_size, mapped_file->fd, cpa_offset);
    if (mapped_area == MAP_FAILED) {
        LOG_ERROR("mf_map: Failed _mmap with offset = %u, cpa_size = %u file_size - (cpa_size - cpa_offset) = %d \n\n",
               cpa_offset, cpa_size, mapped_file->file_size - (cpa_size - cpa_offset));
        assert(!"mf_map: Failed _mmap area.\n");
        return NULL;
    }

    chunk = chunk_create(cpa_offset, cpa_size, mapped_area);
    assert(chunk);
    chunk_is_ok(chunk);
    complex_key_t *key2ht =  complex_key_clone(&key);
    complex_key_is_ok(key2ht);

    hash_add_data(&mapped_file->chunk_ht, (size_t)key2ht, chunk);

    *mapmem_handle = (mf_mapmem_handle_t *)chunk;
    return (void *)&(((char *)mapped_area)[stair]);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    LOG_DEBUG("Called mf_unmap (mf = [%p], mapmem_handle = %u)\n", mf, mapmem_handle);

    assert(mf);
    assert(mapmem_handle);

    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));

    if (mapped_file->file_mapped) {
        return 0;
    }

    struct chunk_t *chunk = (chunk_t *)mapmem_handle;
    assert(chunk_is_ok(chunk));

    if (chunk->ref_cnt > 0)
        chunk->ref_cnt--;

    if (chunk->ref_cnt > 0) {
        return 0;
    }

    size_t w_idx = mapped_file->ring_map_cache_w_idx;
    chunk_t *chunk_to_destruct = mapped_file->ring_map_cache[w_idx];
    if (chunk_is_not_in_pool(mapped_file, chunk)) {
        mapped_file->ring_map_cache[w_idx] = chunk;
    } else {
        mapped_file->ring_map_cache[w_idx] = NULL;
    }
    w_idx = (w_idx + 1) % Ring_map_cache_size;
    mapped_file->ring_map_cache_w_idx = w_idx;

    if (!chunk_to_destruct || chunk_to_destruct->ref_cnt > 0) {
        return 0;
    }

    assert(chunk_is_ok(chunk_to_destruct));

    complex_key_t key;
    key.offset = chunk_to_destruct->mem_offset;
    key.size = (size_t)chunk_to_destruct->mem_size;
    assert(complex_key_is_ok(&key));

    int result = hash_delete_data(&mapped_file->chunk_ht, (size_t)&key);
    // this include chunk unmapping and destructing as we inited hash with such elem destructor
    assert(result == 0);

    return 0;
}

static void *cache_mem_region(struct mapped_file_t *mapped_file, size_t offset, size_t size) {
    assert(mapped_file);
    assert(offset >= 0);
    assert(size >= 0);
    assert(mapped_file_is_ok(mapped_file));

    struct chunk_t *chunk_cache = &mapped_file->cache_chunk;
    // could be not ok

    if (chunk_cache->mem && chunk_cache->mem_offset <= offset
            && chunk_cache->mem_offset + chunk_cache->mem_size >= offset + size) {
        // cache fit
        return  (void *)&(((char *)chunk_cache->mem)[offset - chunk_cache->mem_offset]);
    }

    // cache miss
    if (chunk_cache->mem) {
        _munmap(chunk_cache->mem, (size_t)chunk_cache->mem_size);
        chunk_cache->mem = NULL;
    }
    
    size = MAX(size, RW_atomic_size);
    
    size_t pa_offset = pa_size(offset);
    size_t stair = offset - pa_offset;
    size = size + stair + 2*Chunk_size;

    if (pa_offset + size > mapped_file->file_size)
        size = mapped_file->file_size - pa_offset;

    void *mem_ptr = _mmap(size, mapped_file->fd, pa_offset);
    if (mem_ptr == MAP_FAILED)
        return NULL;

    chunk_cache->mem = mem_ptr;
    chunk_cache->mem_offset = pa_offset;
    chunk_cache->mem_size = size;

    return (void *)&(((char *)mem_ptr)[stair]);;
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
    LOG_DEBUG("Called mf_read (mf = [%p], buf = [%p], count = %u, offset = %u)\n", mf, buf, count, offset);
    assert(mf);
    assert(count >= 0);
    assert(offset >= 0);

    if (count == 0)
        return 0;
    
    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));
    assert(mf && buf && count > 0 && offset >= 0 && offset <= (size_t)mapped_file->file_size);

    if (offset + count > mapped_file->file_size)
        count = (size_t)(mapped_file->file_size - offset);

    void *mem_ptr = cache_mem_region(mapped_file, (size_t)offset, count);
    assert(mem_ptr);

    memcpy(buf, mem_ptr, count);

    if (!mapped_file->file_mapped && mapped_file->cache_mem_av <= count) {
        chunk_t *cache_chunk = &mapped_file->cache_chunk;

        _munmap(cache_chunk->mem, (size_t)cache_chunk->mem_size);
        cache_chunk->mem = NULL;
    }

    return count;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    LOG_DEBUG("Called mf_read (mf = [%p], buf = [%p], count = %u, offset = %u)\n", mf, buf, count, offset);
    assert(mf);
    assert(count >= 0);
    assert(offset >= 0);

    if (count == 0)
        return 0;
        
    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));
    assert(mf && buf && count > 0 && offset >= 0 && offset <= (size_t)mapped_file->file_size);

    if (offset + count > mapped_file->file_size)
        count = (size_t)(mapped_file->file_size - offset);

    void *mem_ptr = cache_mem_region(mapped_file, (size_t)offset, count);
    assert(mem_ptr);

    memcpy(mem_ptr, buf, count);

    if (!mapped_file->file_mapped && mapped_file->cache_mem_av <= count) {
        printf("Going destruct!\n");
        chunk_t *cache_chunk = &mapped_file->cache_chunk;

        _munmap(cache_chunk->mem, (size_t)cache_chunk->mem_size);
        cache_chunk->mem = NULL;
    }

    return count;
}

off_t mf_file_size(mf_handle_t mf) {
    LOG_DEBUG("Called mf_file_size ( [ %p ] )\n", mf);
    assert(mf);

    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    assert(mapped_file_is_ok(mapped_file));

    return mapped_file->file_size;
}

bool mapped_file_is_ok(struct mapped_file_t *mapped_file) {
    assert(mapped_file);

    return hash_is_ok(&(mapped_file->chunk_ht))
           && (mapped_file->fd >= 0) && (mapped_file->file_size >= 0)
           && ((!mapped_file->file_mapped) || (mapped_file->file_mapped && mapped_file->cache_chunk.mem));
}


bool complex_key_is_ok(struct complex_key_t *complex_key) {
    assert(complex_key);

    return (complex_key->offset >= 0) && (complex_key->size >= 0);
}

bool chunk_is_ok(struct chunk_t *chunk) {
    assert(chunk);

    return (chunk->ref_cnt >= 0) && (chunk->mem) && (chunk->mem_offset >= 0) && (chunk->mem_size >= 0);
}

bool chunk_is_not_in_pool(mapped_file_t *mapped_file, chunk_t *chunk) {
    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));
    assert(chunk);
    assert(chunk_is_ok(chunk));

    chunk_t **pool = mapped_file->ring_map_cache;
    for (size_t i = 0; i < Ring_map_cache_size; i++) {
        if (pool[i] && pool[i]->mem_size == chunk->mem_size && pool[i]->mem_offset == chunk->mem_offset)
            return false;
    }

    return true;
}

size_t hash_func(const size_t key) {
    complex_key_t *ck = (complex_key_t *)key;
    assert(ck);
    assert(complex_key_is_ok(ck));

    uint64_t single_key = (uint64_t)(ck->offset ^ (size_t)ck->size);

    // MurmerHash3 hash function
    single_key ^= single_key >> 33;
    single_key *= 0xff51afd7ed558ccd;
    single_key ^= single_key >> 33;
    single_key *= 0xc4ceb9fe1a85ec53;
    single_key ^= single_key >> 33;

    return single_key;
}

bool cmp_func(const size_t key_a, const size_t key_b) {
    complex_key_t *ck_a = (complex_key_t *)key_a;
    assert(ck_a);
    assert(complex_key_is_ok(ck_a));

    complex_key_t *ck_b = (complex_key_t *)key_b;
    assert(ck_b);
    assert(complex_key_is_ok(ck_b));

    return (ck_a->offset == ck_b->offset) && (ck_a->size == ck_b->size);
}

void destruct_internals_func(hash_list_t *hash_list) {
    assert(hash_list);
    assert(complex_key_is_ok((complex_key_t *)hash_list->key));

#ifndef NDEBUG
    ((complex_key_t *)hash_list->key)->offset = -1;
    ((complex_key_t *)hash_list->key)->size = -1;
#endif //NDEBUG

    free((void *)hash_list->key); //complex_key_t

    chunk_t *chunk = (chunk_t *)hash_list->data;
    assert(chunk);
    assert(chunk_is_ok(chunk));
    _munmap(chunk->mem, (size_t)chunk->mem_size);

#ifndef NDEBUG
    chunk->mem = NULL;
    chunk->mem_offset = -1;
    chunk->mem_size = -1;
    chunk->ref_cnt = -1;
#endif //NDEBUG

    free(hash_list->data); //chunk_t
}

static chunk_t *chunk_create(off_t offset, size_t size, void *mem) {
    assert(size > 0);
    assert(offset >= 0);

    chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
    assert(chunk);

    chunk->ref_cnt = 1;
    chunk->mem_offset = offset;
    chunk->mem_size = size;
    chunk->mem = mem;

    return chunk;
}

static complex_key_t *complex_key_clone(const complex_key_t *complex_key) {
    assert(complex_key);

    complex_key_t *complex_key_copy = (complex_key_t *)malloc(sizeof(complex_key_t));
    assert(complex_key_copy);

    memcpy(complex_key_copy, complex_key, sizeof(complex_key_t));

    return complex_key_copy;
}

static size_t pa_size(size_t size) {
    return (size_t)(size & ~(Page_size - 1));
}

static size_t ca_size(size_t size) {
    return (size / Chunk_size) * Chunk_size;
}

static void *_mmap(size_t length, int fd, off_t offset) {
    assert(fd >= 0);
    assert(length > 0);
    assert(offset == pa_size((size_t)offset));

#ifdef MEMDEBUG
    size_t true_length = length + 2 * Page_size;
    void *whole_mem_ptr = mmap(NULL, true_length,
                               PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(whole_mem_ptr != MAP_FAILED);
    memset(whole_mem_ptr, 0xFF, true_length);

    void *mem_ptr = mmap(whole_mem_ptr + Page_size, length,
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, offset);
    assert(mem_ptr != MAP_FAILED);

    return mem_ptr;
#else
    return mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
#endif //NDEBUG
}

static int _munmap(void *addr, size_t length) {
    assert(addr);
    assert(length > 0);

#ifdef MEMDEBUG
    size_t true_length = length + 2 * Page_size;
    void *true_addr = addr - Page_size;
    int munmap_result = munmap(true_addr, true_length);
    assert(munmap_result == 0);
    return munmap_result;
#else
    return munmap(addr, length);
#endif //NDEBUG
}