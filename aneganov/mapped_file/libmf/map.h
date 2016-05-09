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

typedef struct chunk* hval_t;

typedef struct map map_t;

int map_construct(map_t **map);
int map_destruct(map_t *map);
int map_add(map_t * map, hkey_t *key, hval_t val);
int map_lookup_le(const map_t * map, hkey_t *key, hval_t *val);
int map_del(map_t *map, hkey_t *key, bool is_indexed);

#endif