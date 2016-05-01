/*
 * rbtree.c	Red-black balanced binary trees.
 *
 * Version:	$Id: rbtree.c,v 1.10.4.2 2006/03/15 15:37:57 nbk Exp $
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *  Copyright 2004  The FreeRADIUS server project
 *
 *  Modified by Denis Kopyrin to conform mapped_file project
 */
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

rbnode_t* rbtree_insert(rbtree_t *tree, void *Data);
void rbtree_delete(rbtree_t *tree, rbnode_t *Z);
int rbtree_deletebydata(rbtree_t *tree, const void *data);
rbnode_t *rbtree_find(rbtree_t *tree, const void *Data);
void *rbtree_finddata(rbtree_t *tree, const void *Data);
int rbtree_walk(rbtree_t *tree, RBTREE_ORDER order,
	int (*callback)(void *, void *), void *context);
int rbtree_num_elements(rbtree_t *tree);
void *rbtree_node2data(rbtree_t *tree, rbnode_t *node);

#endif
