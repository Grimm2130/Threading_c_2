#ifndef __THREADLIB_H__
#define __THREADLIB_H__

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include "gl.h"

#define THREAD_NAME_LEN     0xFF

#define GET_STRUCT_OFFSET( struct_name, struct_field )   \
    ( (uint64_t)&(((struct_name*)0)->struct_field) )

/* -----------------------------------------------------------------------*/
/* --------------------------- Enumerations ------------------------------*/
/* -----------------------------------------------------------------------*/

typedef enum THREAD_FLAG
{
    THREAD_CREATED = 0,         // thread has been created
    THREAD_RUNNING = 1,         // thread is running
    THREAD_MARKED_FOR_PAUSE = 2,
    THREAD_PAUSED = 3,
    THREAD_BLOCKED = 4,
    
    THREAD_CALLER_BLOCKED = 30, 
    THREAD_DETACHED = 31
}THREAD_FLAG_ENUM;

/* ----------------------------------------------------------------------------------------------------*/
/* --------------------------------------- Data Strutures -----------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

/* --------------------- Thread Encasulation ----------------------- */

typedef struct thread
{
    char name[THREAD_NAME_LEN];         // thread name
    uint32_t m_flags;                   // Thread flags
    uint8_t m_priority;                 // Thread priority
    glnode_t glue;                      // glue node for gl funtionality reference
    pthread_t m_thread;                 // Thread object
    pthread_attr_t m_thread_attr;       // Thread attribute
    pthread_mutex_t m_thread_mut;       // Thread mutex object
    pthread_cond_t m_thread_cv;         // Thread conditional variable
    void *pause_arg;                    // argument passed to pause fn
    void* (*pause_fn_ptr)(void*);       // Function executed after thread resumes
    void* arg;                          // Pointer to thread function arg
    void* (*thread_fn_ptr)( void* );    // Pointer to thread function
    sem_t sem;
}thread_t;

thread_t* thread_create_alloc( const char * name );
void thread_create( thread_t* th, const char * name );
void thread_set_attr_joinable_detached( thread_t* th, bool joinable );
void thread_run( thread_t* th, void* arg, void* (*fn_ptr)(void*) );
void thread_pause( thread_t* th );
void thread_test_and_pause( thread_t* th );
void thread_resume( thread_t* th );
void thread_set_pause_fn( thread_t* th, void *pause_arg, void *(*pause_fn)(void *) );
void thread_destroy( thread_t* th );

/* --------------------- Thread Pool ----------------------- */

typedef struct threadpool
{
    glthread_t threadpool_lis;
    pthread_mutex_t pool_mut;
    int (*cmp_fn)( thread_t*, thread_t* );
}threadpool_t;

void threadpool_init( threadpool_t* t_pool );
void threadpool_init_with_threads( threadpool_t* t_pool, uint8_t num_threads );
void threadpool_insert_new_thread( threadpool_t* t_pool, thread_t * thread );
void threadpool_priority_insert_thread( threadpool_t* t_pool, thread_t * thread );
thread_t* threadpool_get_thread( threadpool_t* t_pool );
void threadpool_set_cmp_fn( threadpool_t* t_pool, int (*cmp_fn)(thread_t*, thread_t*) );
void threadpool_dispatch_thread( threadpool_t* t_pool, void* (*thread_fn) (void*), void* arg, bool block_caller );

/* ----------------- Thread Execution Data ------------------- */

typedef struct thread_execution_data
{
    // Stage 2 (execute)
    void* (*thread_ex_fn)(void *);
    void* arg;
    // Stage 3  (recycle)
    void* (*thread_recyle)(threadpool_t*, thread_t *);
    threadpool_t* t_pool;
    thread_t* th;
} thread_execution_data_t;

/* ----------------- Wait Queue ------------------- */

typedef bool (*wait_queue_cond_fn)( void* arg, pthread_mutex_t** res_mut);

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

#endif  // __THREADLIB_H__