#ifndef __WAIT_QUEUE_H__
#define __WAIT_QUEUE_H__


#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

typedef bool (*wait_queue_cond_fn)( void* arg, pthread_mutex_t* res_mut);

typedef struct wait_queue
{
    uint32_t blocked_threads;
    pthread_cond_t wq_cv;
    pthread_mutex_t* resource_mut_cache;
} wait_queue_t;

wait_queue_t* wait_queue_alloc();
void wait_queue_init( wait_queue_t* wq );
void wait_queue_test_and_wait( wait_queue_t* wq, wait_queue_cond_fn resource_check_fn, void* resource);
void wait_queue_signal( wait_queue_t* wq );
void wait_queue_broadcast( wait_queue_t* wq );
void wait_queue_destroy( wait_queue_t* wq );


#endif  // __WAIT_QUEUE_H__