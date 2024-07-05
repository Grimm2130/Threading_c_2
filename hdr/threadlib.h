#ifndef __THREADLIB_H__
#define __THREADLIB_H__

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>

#include "gl.h"

#define THREAD_NAME_LEN     32

#define GET_STRUCT_OFFSET( struct_name, struct_field )   \
    ( (uint64_t)&(((struct_name*)0)->struct_field) )

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

typedef struct thread_
{
    char name[THREAD_NAME_LEN];         // thread name
    uint32_t m_flags;
    pthread_t m_thread;                 // Thread object
    pthread_mutex_t m_thread_mut;       // Thread mutex object
    pthread_mutexattr_t m_thread_mut_attr;
    pthread_cond_t m_thread_cv;         // Thread conditional variable
    pthread_condattr_t m_thread_cv_attr;
    pthread_attr_t m_thread_attr;       // Thread attribute
    glnode_t glue;                      // glue node for gl funtionality reference
    void *pause_arg;                    // argument passed to pause fn
    void* (*pause_fn_ptr)(void*);       // Function executed after thread resumes
    void* arg;                          // Pointer to thread function arg
    void* (*thread_fn_ptr)( void* );    // Pointer to thread function
    sem_t sem;
}thread_t;

thread_t* thread_create_alloc( char * name );
void thread_create( thread_t* th, char * name );
void thread_set_attr_joinable_detached( thread_t* th, bool joinable );
void thread_run( thread_t* th, void* arg, void* (*fn_ptr)(void*) );
void thread_pause( thread_t* th );
void thread_test_and_pause( thread_t* th );
void thread_resume( thread_t* th );
void thread_set_pause_fn( thread_t* th, void *pause_arg, void *(*pause_fn)(void *) );
void thread_destroy( thread_t* th );

/*Thread Pool*/

typedef struct threadpool
{
    glthread_t threadpool_lis;
    pthread_mutex_t pool_mut;
}threadpool_t;


void threadpool_init( threadpool_t* t_pool );
void threadpool_insert_new_thread( threadpool_t* t_pool, thread_t * thread );
thread_t* threadpool_get_thread( threadpool_t* t_pool );
void threadpool_dispatch_thread( threadpool_t* t_pool, void* (*thread_fn) (void*), void* arg, bool block_caller );


/*Thread Execution Data*/

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

#endif  // __THREADLIB_H__