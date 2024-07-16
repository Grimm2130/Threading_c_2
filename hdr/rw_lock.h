#ifndef __RW_LOCK_H__
#define __RW_LOCK_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_WRITERS 1

/// @brief Reader Writer Lock
typedef struct rw_lock
{
    bool has_readers;                // number of readers
    uint8_t has_writer;                // Number of writers
    int lock_depth;                     // lock depth
    bool is_recursive;                  // lock is recursive
    pthread_mutex_t mut;                // Mutex
    pthread_cond_t cv;                  // Conditionals
    pthread_t indexed_writer;       // thread cache
}rw_lock_t;


void rw_lock_init( rw_lock_t* lock, bool is_recursive );
rw_lock_t* rw_lock_alloc( bool is_recursive );
void rw_lock_rd_lock( rw_lock_t* lock );
void rw_lock_wr_lock( rw_lock_t* lock );
void rw_lock_unlock( rw_lock_t* lock );


#endif // __RW_LOCK_H__
