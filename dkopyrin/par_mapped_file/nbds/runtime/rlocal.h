#ifndef RLOCAL_H
#define RLOCAL_H

#include "runtime.h"
#include "tls.h"

extern DECLARE_THREAD_LOCAL(ThreadId, int);

/*assert(ThreadId != 0)*/
//TODO: Very dirty hack!!!
#define GET_THREAD_INDEX() ({ LOCALIZE_THREAD_LOCAL(ThreadId, int); if (ThreadId == 0) {nbd_thread_init(); LOCALIZE_THREAD_LOCAL(ThreadId, int); }; ThreadId - 1; })

void mem_init (void);
void rnd_init (void);

void rnd_thread_init (void);
void rcu_thread_init (void);
void lwt_thread_init (void);

#endif//RLOCAL_H
