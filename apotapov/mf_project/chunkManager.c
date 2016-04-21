#include <chunkManager.h>

#define DEFAULT_CH_POOL_SIZE (0x40000)
#define DEFAULT_HASH_TABLE_SIZE 256

static size_t get_ch_size (off_t multiplier) {
	return multiplier * sysconf(_SC_PAGESIZE);
}

static int ch_init(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length) {
	chunk->index = index;
	chunk->length = length;
	chunk->prev = NULL;
	chunk->next = NULL;
	chunk->ch_pool = ch_pool;
	chunk->refer_counter = 1;
	chunk->pld = mmap(NULL, get_ch_size(length), ch_pool->prot, MAP_SHARED, ch_pool->file_d, get_ch_size(1));
	if(chunk->pld == MAP_FAILED)
		return errno;
	return 0;
}

static int ch_destr(chunk_t *chunk) {
	if(munmap(chunk->pld, chunk->length*get_ch_size(1)) == -1 )
		return errno;
	free(chunk);
	chunk = NULL;
	return 0;
}

static int ch_create(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length) {
	chunk_t* res = (chunk_t*)malloc(sizeof(chunk_t));
	if(res == NULL) {
		return ENOMEM;
	}
    chunk = res;
	int res_code = ch_init(ch_pool, chunk, index, length);
	if(res_code) return res_code;
    res_code = ch_pool_add(chunk);
	if(res_code) return res_code;
	return 0;
}

static int ch_get(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length) {
    int res_code = find_value(ch_pool->hashtable, (const hash_key_t)index, (value_t*)chunk);
    if(res_code) {
        return res_code;
    }
	if(chunk->length < length) {
		return ENOKEY;
	}
	return 0;
}

int ch_acquire(ch_pool_t *ch_pool, chunk_t *chunk, off_t offset, size_t size) {
	if(!ch_pool || !chunk || !ch_pool->hashtable || ch_pool->file_d < 0 || ch_pool->num_of_pages > ch_pool->size)
		return EINVAL;
	if(size == 0) {
		chunk = NULL;
		return 0;
	}
	off_t index = offset / get_ch_size(1);
	off_t length = size/get_ch_size(1) + 1;
	int res_code = ch_get(ch_pool, index, length, chunk);
	if(res_code == ENOKEY) {
		res_code = ch_pool_del(chunk);
		if(res_code && res_code != EBUSY) {
			return res_code;
		}
		ch_create(ch_pool, chunk, index, length);
		return 0;
	} else {
        return res_code;
    }
}

int ch_find(ch_pool_t* ch_pool, chunk_t *chunk, size_t size, off_t offset) {
	if(!ch_pool->hashtable || !chunk || !ch_pool || ch_pool->file_d < 0 || ch_pool->num_of_pages > ch_pool->size) {
		return EINVAL;
	}
	if(size == 0) {
		chunk = NULL;
		return 0;
	}
	off_t index = offset / get_ch_size(1);
	off_t length = size / get_ch_size(1) + 1;
	return ch_get(ch_pool, chunk, index, length);
}

static int ch_pool_del(chunk_t *chunk) {
	ch_pool_t *ch_pool = chunk->ch_pool;
	if(chunk == NULL || chunk->refer_counter != 0)
		return EBUSY;
	if(chunk->prev) chunk->prev->next = chunk->next;
	if(chunk->next) chunk->next->prev = chunk->prev;
    int i = 0;
    for(i = 0; i < chunk->length; i++) {
		int res_code = remove_element(ch_pool->hashtable, chunk->index+i, (value_t*)chunk);
		if(res_code && res_code != ENOKEY) {
			return res_code;
		}
	}
	ch_pool->num_of_pages -= chunk->length;
	int res_code = ch_destr(chunk);
	if(res_code) {
		return res_code;
	}
	return 0;
}

static int ch_pool_add(chunk_t *chunk) {
	ch_pool_t *ch_pool = chunk->ch_pool;
	int res_code = 0;
	while(chunk->length + ch_pool->num_of_pages > ch_pool->size && !(res_code = ch_pool_del(ch_pool->ch_list)));
	if(res_code == EBUSY) {
		return ENOBUFS;
	}
	if(res_code) {
		return res_code;
	}
	chunk->prev = NULL;
	chunk->next = ch_pool->ch_list;
	ch_pool->ch_list = chunk;
    int i = 0;
    for(i = 0; i < chunk->length; i++) {
		res_code = add_element(ch_pool->hashtable, chunk->index + i, (value_t)chunk);
		if(res_code) {
			return res_code;
		}
	}
	ch_pool->num_of_pages += chunk->length;
	return 0;
}

int ch_release(chunk_t *chunk) {
	if(!chunk) {
		return EINVAL;
	}
	if(chunk->refer_counter == 0) {
		return EAGAIN;
	}
	if(chunk->refer_counter == 1) {
		if(chunk->prev) chunk->prev->next = chunk->next;
		if(chunk->next) chunk->next->prev = chunk->prev;
		chunk->next = chunk->ch_pool->ch_list;
		chunk->prev = NULL;
	}	
	return 0;
}

int ch_pool_destruct(ch_pool_t *ch_pool) {
    if(ch_pool == NULL) {
        return EINVAL;
    }
    while(ch_pool->ch_list) {
        chunk_t *k = ch_pool->ch_list;
        ch_pool->ch_list = k->next;
        int res_code = ch_destr(k->prev);
        if(res_code) {
            return res_code;
        }
    }
    ch_pool->num_of_pages = 0;

    int res_code = deinit(ch_pool->hashtable);
    if(res_code) {
        return res_code;
    }
    res_code = deinit(ch_pool->mem_hash_table);
    if(res_code) {
        return res_code;
    }
    res_code = close(ch_pool->file_d);
    if(res_code == -1) {
        return errno;
    }
    free(ch_pool);
    ch_pool = NULL;
    return 0;
}

static int ch_pool_init(ch_pool_t *ch_pool, unsigned size, int fd, int prot) {
	ch_pool->ch_list = NULL;
	ch_pool->size = size;
	ch_pool->prot = prot;
	ch_pool->num_of_pages = 0;
    ch_pool->file_d = fd;
	ch_pool->hashtable = hash_table_init(DEFAULT_HASH_TABLE_SIZE); //???
	if(ch_pool->hashtable == NULL) {
		return ENOMEM;
	}
	ch_pool->mem_hash_table = hash_table_init(DEFAULT_HASH_TABLE_SIZE);//????
	if(ch_pool->mem_hash_table == NULL) {
		return ENOMEM;
	}
	return 0;
}

int ch_pool_construct(ch_pool_t *ch_pool, size_t max_m, int file_d, int prot) {
    size_t size = max_m ? max_m / get_ch_size(1) : DEFAULT_CH_POOL_SIZE;
	ch_pool = (ch_pool_t*)malloc(sizeof(ch_pool_t));
    return ch_pool_init(ch_pool, size, file_d, prot);
}


int ch_pool_mem_add(void *pointer, chunk_t *chunk) {
    return add_element(chunk->ch_pool->mem_hash_table, (hash_key_t)pointer, (value_t*)chunk);
}

int ch_pool_mem_get(ch_pool_t *ch_pool, void *ptr, chunk_t *chunk) {
    return find_value(ch_pool->mem_hash_table, (hash_key_t)ptr, (value_t*)chunk);
}

int ch_get_mem(chunk_t *chunk, off_t offset, void *buf) {
	if(chunk == NULL || buf == NULL) {
		return EINVAL;
	}
	off_t l = get_ch_size(chunk->index);
	off_t r = l + get_ch_size(chunk->length);

	if(offset < l || offset > r) {
		return EINVAL;
	}
	
    buf = chunk -> pld + (offset - l);
	return 0;
}

int ch_pool_fd(ch_pool_t *ch_pool) {
	if(!ch_pool || ch_pool->file_d < 0)
		return -1;
	return ch_pool->file_d;
}



