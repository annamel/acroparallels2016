#include "chunk_queue.h"

int chunk_queue_init(struct chunk_queue * cq){
	int i;
	for (i = 0; i < POOL_SIZE; i++){
		struct chunk* cur_ch = cq -> chunk_pool + i;
		chunk_init_unused(cur_ch);
		cur_ch -> next = cur_ch + 1;
	}

	cq -> head = cq -> chunk_pool;
	cq -> tail = cq -> chunk_pool + POOL_SIZE - 1;
	return 0;
}

void chunk_queue_finalize(struct chunk_queue * cq){
	int i;
	for (i = 0; i < POOL_SIZE; i++)
		if (cq -> chunk_pool[i].ref_cnt != -1)
			chunk_finalize (cq -> chunk_pool + i);
#ifdef MEMORY_DEBUG
	cq -> head = 0xDEADBEEF;
	cq -> tail = 0xDEADBEEF;
#endif
}

/*
ENQUEUE(x)
    q ← new record
    q^.value ← x
    q^.next ← NULL
    repeat
        p ← tail
        succ ← COMPARE&SWAP(p^.next, NULL, q)
        if succ ≠ TRUE
            COMPARE&SWAP(tail, p, p^.next)
    until succ = TRUE
    COMPARE&SWAP(tail,p,q)
end*/
void chunk_queue_enqueue(struct chunk_queue* cq, struct chunk *ch){
	ch -> next = NULL;
	int succ = 0;
	do{
		succ = __sync_bool_compare_and_swap(cq -> tail -> next, NULL, ch);
		if (succ != 0)
			__sync_bool_compare_and_swap(cq -> tail -> next, NULL, ch);
	}while(!succ);
}

/*
DEQUEUE()
    repeat
        p ← head
        if p^.next = NULL
            error queue empty
    until COMPARE&SWAP(head, p, p^.next)
    return p^.next^.value
end
*/
