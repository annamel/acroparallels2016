# Mapped file

This is implementation of API provided at __include/mapped_file.h__ by Denis Kopyrin

# Hashtable

Aside most common implementation using hashtable that maps offset into chunk this
very library use rbtree. If hashtable is used to map offset, then making new chunk
costs O(chunk length) that might be very expensive (see __test_ht.c__). More optimized
way to lookup for offset is using self-balancing tree or using hashtable in another way.

# Main lookup function in library
This library provides two main functions:

__chunk_manager_get_av_chunk_from_pool__ - using FIFO algorithm allows to give
available chunks in chunk pool at nearly O(1) in best case and O(n) in worst case.

__chunk_manager_gen_chunk__ - do the actual lookup in rbtree and use __chunk_manager_get_av_chunk_from_pool__
to generate chunk if lookup failed. The main structure that is used is rbtree that
maps chunk startpoints to chunks (see next abstract)

# Lookup in rbtrees

The problem of mapping offset to chunk might be treated as mapping point to interval
that contains this point. The most trivial way of doing this is using rbtree to sort
start endpoint of each interval and finding the closest startpoint to given point.
However such simple approach might give wrong results. Let's head to __test_pyramid.c__

In this test "pyramid" construction is built of chunks:

               CCCCC 
            CCCCCCCCCCCCC           |
          CCCCCCCCCCCCCCCCCCCCCCCC  V
        CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

Clearly rbtree gives the smallest chunk on the top rather then the one that is needed.
That is why rbtree might fail in this case but as map is lazy we still might use this
approach if chunk beginning and chunk endings are aligned by chunk size.

More finicky way to solve this problem is to use interval trees that are able to give
interval without errors at logarithmic time as well as the simple way but does not have the problem
stated above. This library use simple approach as it is significantly faster
(constant of time usage and memory usage is nearly 3 times bigger).

# Improve insert speed on eviction

As this library use simple approach it is easy to improve insert speed if we know
offset of chunk in rbtree but its size is too small. Both of this criteria are
correct for __chunk_manager_gen_chunk__ function so we just save found chunk and its node
in rbtree and update its value with new bigger chunk (see __test_stairs.c__).
