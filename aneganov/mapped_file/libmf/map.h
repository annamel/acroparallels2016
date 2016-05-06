#ifndef __MF_MAP_H__
#define __MF_MAP_H__

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

typedef union hkey {
	struct {
		off_t idx;
		off_t len;
	};
	uint32_t raw[sizeof(off_t)/2];
} hkey_t;

typedef void* val_t;

typedef struct map map_t;

int map_construct(int (*cmp)(val_t, val_t), off_t (*skey_from_val)(val_t), map_t **map);
int map_destruct(map_t *map);
int map_add(map_t * map, hkey_t *key, val_t val, val_t *oldval_ptr);
int map_lookup_le(const map_t * map, hkey_t *key, val_t *val);
int map_del(map_t *map, hkey_t *key, bool is_indexed);

#endif