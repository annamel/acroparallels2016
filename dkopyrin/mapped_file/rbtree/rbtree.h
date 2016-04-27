#ifndef __RBTREE_H
#define __RBTREE_H

#include <stdint.h>

/* red-black tree description */
typedef enum { Black, Red } NodeColor;
typedef enum { PreOrder, InOrder, PostOrder } RBTREE_ORDER;

typedef struct rbnode_t {
	struct rbnode_t	*Left;		/* left child */
	struct rbnode_t	*Right;		/* right child */
	struct rbnode_t	*Parent;	/* parent */
	NodeColor	Color;		/* node color (black, red) */
	void	*Data;		/* data stored in node */
} rbnode_t;


typedef struct rbtree_t {
#ifndef NDEBUG
	uint32_t magic;
#endif
	rbnode_t *Root;
	int	num_elements;
	int (*Compare)(const void *, const void *);
	int replace_flag;
	void (*freeNode)(void *);
} rbtree_t;

void rbtree_free(rbtree_t *tree);
rbtree_t *rbtree_create(int (*Compare)(const void *, const void *),
		void (*freeNode)(void *),
		int replace_flag);

int rbtree_insert(rbtree_t *tree, void *Data);
void rbtree_delete(rbtree_t *tree, rbnode_t *Z);
int rbtree_deletebydata(rbtree_t *tree, const void *data);
rbnode_t *rbtree_find(rbtree_t *tree, const void *Data);
void *rbtree_finddata(rbtree_t *tree, const void *Data);
int rbtree_walk(rbtree_t *tree, RBTREE_ORDER order,
	int (*callback)(void *, void *), void *context);
int rbtree_num_elements(rbtree_t *tree);
void *rbtree_node2data(rbtree_t *tree, rbnode_t *node);

#endif
