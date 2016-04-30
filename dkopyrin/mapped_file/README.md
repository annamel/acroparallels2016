# Mapped file

This is implementation of API provided at __include/mapped_file.h__ by Denis Kopyrin

# Hashtable

Aside most common implementation using hashtable that maps offset into chunk this
very library use rbtree. If hashtable is used for offset mapping, then making new chunk
costs O(chunk length) that might be very expensive (see __test_ht.c__). More optimized
way to lookup for offset is using self-balancing tree or using hashtable in another way.

# Main lookup function in library
Internal workflow of library is based on two main functions:

__chunk_manager_get_av_chunk_from_pool__ - using FIFO algorithm allows to give
chunks with ref_counter=0 or unused chunks in chunk pool at nearly O(1) in best case and O(n) in worst case.

__chunk_manager_gen_chunk__ - do the actual lookup in rbtree and use __chunk_manager_get_av_chunk_from_pool__
to generate chunk if lookup failed.

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
approach if chunk beginning and chunk endings are aligned by fixed minimum chunk size.

More finicky way to solve this problem is to use __interval trees__ that are able to give
interval without errors at logarithmic time as well as the simple way but does not have the problem
stated above. This library use simple approach as it is significantly faster
(constant of time usage and memory usage of interval tree is nearly 3 times higher).

# Improve insert speed on eviction

As this library use simple approach it is easy to improve insert speed if we know that 
offset of evicted chunk in rbtree is the same as offset of new one and size of new chunk is bigger. 
Both of this criteria are correct for __chunk_manager_gen_chunk__ function so we just save found chunk and its node
in rbtree and update its value with new bigger chunk (see __test_stairs.c__) that takes O(1)
instead of O(log n)

# Improve lookup speed by caching

If one use any profiling tool on __test_ladder.c__(generating continuous intersecting maps) one will 
see that hotspot is rbtree lookup. As scenario of continuous read is common storing last
used chunk might save from rbtree lookup that takes O(1) unlike lookup O(log n).
