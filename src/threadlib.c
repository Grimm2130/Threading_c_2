#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include "../hdr/threadlib.h"
#include "../hdr/bitsop.h"
#include "../hdr/gl.h"

/******************************************************************************/
/**
* Helper functions
*/

/// @brief Get the thread pause status
/// @param flags 
/// @param flag_pos 
/// @return 
static inline bool thread_get_flag_status( const uint32_t flags, THREAD_FLAG_ENUM flag_pos )
{
    // TODO: Using Macros, will use in the event flags go past 64 bit range
    return -1;
}

/// @brief Sets the flag at the desired position
/// @param flags 
/// @param flag_pos 
/// @param bit 
static inline void thread_set_flag_status( uint32_t * volatile flags, THREAD_FLAG_ENUM flag_pos, bool bit )
{
    return;
}

/******************************************************************************/
                        // Thread Object
/******************************************************************************/

/// @brief Instantiate a new thread object
/// @param th Thread object
/// @param name Name of thread
void thread_create( thread_t* th, char * name )
{
    assert( th );
    assert( name );
    // Set thread name
    if( strlen(name) >= THREAD_NAME_LEN )
    {
        perror("Name length exceeds allowable name length");
        printf("Truncating....%s", name);
        memcpy( th->name, name, THREAD_NAME_LEN );
        th->name[THREAD_NAME_LEN-1] = '\0';     // terminate string
    }
    else
    {
        memcpy( th->name, name, strlen(name) );
    }
    // instantiate flags
    th->m_flags = 0;
    // init args
    th->arg = NULL;
    th->pause_arg = NULL;
    // Init function pointers
    th->pause_fn_ptr = NULL;
    th->thread_fn_ptr = NULL;
    // init pthread object and attr
    memset( &th->m_thread, 0, sizeof(pthread_t) );
    memset( &th->m_thread_attr, 0, sizeof(pthread_attr_t) );
    // init mutex and attr
    memset( &th->m_thread_mut, 0, sizeof(pthread_mutex_t) );
    memset( &th->m_thread_mut_attr, 0, sizeof(pthread_mutexattr_t) );
    pthread_mutex_init(&th->m_thread_mut, &th->m_thread_mut_attr);
    // init conditional and attr
    memset( &th->m_thread_cv, 0, sizeof(pthread_cond_t) );
    memset( &th->m_thread_cv_attr, 0, sizeof(pthread_condattr_t) );
    pthread_cond_init(&th->m_thread_cv, &th->m_thread_cv_attr);
}

/// @brief Allocate memory and instantiate thread object
/// @param name Name of thread
/// @return Allocated Memory
thread_t* thread_create_alloc( char * name )
{
    thread_t* newTh = (thread_t*) calloc(1, sizeof(thread_t));
    thread_create( newTh, name );
    return newTh;
}


/// @brief Set
/// @param th 
/// @param joinable 
void thread_set_attr_joinable_detached( thread_t* th, bool joinable )
{
    assert(th);
    pthread_attr_setdetachstate( &th->m_thread_attr, 
    joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED );
}

/// @brief Starts the threads operation
/// @param th 
/// @param arg 
/// @param fn_ptr 
void thread_run( thread_t* th, void* arg, void* (*fn_ptr)(void*) )
{
    assert(th);
    assert(arg);
    assert(fn_ptr);
    th->arg = arg;
    th->thread_fn_ptr = fn_ptr;
    if( pthread_create( &th->m_thread, &th->m_thread_attr, th->thread_fn_ptr, th->arg) == 0 )
    {
        SET_BIT( th->m_flags, THREAD_CREATED );
        SET_BIT( th->m_flags, THREAD_RUNNING) ;
    }
    else
    {
        perror("Thread creation failed");
        exit(errno);
    }
}

/// @brief Pauses the thread object
/// @param th Pointer to thread object
void thread_pause( thread_t* th )
{
    assert(th);
    // assert that thread is running and created
    assert( IS_BIT_SET(th->m_flags, THREAD_CREATED ) );
    pthread_mutex_lock( &th->m_thread_mut );
    if( IS_BIT_SET( th->m_flags, THREAD_RUNNING ) )
    {
        SET_BIT( th->m_flags, THREAD_MARKED_FOR_PAUSE );
    }
    pthread_mutex_unlock( &th->m_thread_mut );
}

/// @brief Checks that the pause flag has been set and waits for it to be unset
/// @param th Pointer to thread object
void thread_test_and_pause( thread_t* th )
{
    assert( th );
    pthread_mutex_lock( &th->m_thread_mut );
    // Check if the thread is marked for pause
    if( IS_BIT_SET( th->m_flags, THREAD_MARKED_FOR_PAUSE ) )
    {
        // set the THREAD_PAUSED bit
        SET_BIT( th->m_flags, THREAD_PAUSED );
        // unset mark for THREAD_PAUSED bit
        UNSET_BIT( th->m_flags, THREAD_MARKED_FOR_PAUSE);     
        // update the running flag
        UNSET_BIT( th->m_flags, THREAD_RUNNING);
        // Wait on THREAD_PAUSED bit to be unset
        while( IS_BIT_SET( th->m_flags, THREAD_PAUSED ) )
        {
            pthread_cond_wait( &th->m_thread_cv, &th->m_thread_mut );
        }
    }
    pthread_mutex_unlock( &th->m_thread_mut );
    SET_BIT( th->m_flags, THREAD_RUNNING );         // update thread's running flag
    th->pause_fn_ptr( th->pause_arg );      // run the un-THREAD_PAUSED function
}

/// @brief "R"
/// @param th 
void thread_resume( thread_t* th )
{
    assert( th );
    pthread_mutex_lock( &th->m_thread_mut );
    if( IS_BIT_SET( th->m_flags, THREAD_PAUSED ) )
    {
        UNSET_BIT( th->m_flags, THREAD_PAUSED );
        pthread_cond_signal( &th->m_thread_cv );
    }
    pthread_mutex_unlock( &th->m_thread_mut );
}

void thread_set_pause_fn( thread_t* th, void *pause_arg, void *(*pause_fn)(void *) )
{
    assert( th );
    assert( pause_arg );
    assert( pause_fn );
    th->pause_arg = pause_arg;
    th->pause_fn_ptr = pause_fn;
}

/******************************************************************************/
                        // Thread Pool Object
/******************************************************************************/

/******************************************************************************/
                        // Static Thread Pool Operations

// Checks if the thread is in the pool
static bool thread_in_pool( threadpool_t* t_pool, thread_t* th )
{
    glnode_t* node;
    bool res = false;
    GLTHREAD_ITERATOR_START( (&t_pool->thread_pool_lis), node )
    {
        if( node )
        {
            thread_t* temp = (thread_t*)(node - t_pool->thread_pool_lis.base_addr);
            if( strcmp(temp->name, th->name) == 0 )
            {
                res = true;
                break;
            }
        }
    }
    GLTHREAD_ITERATOR_END( (&t_pool->thread_pool_lis) )
    return res;
}

/******************************************************************************/

/// @brief Instantiate the thread pool
/// @param t_pool 
void thread_pool_init( threadpool_t* t_pool )
{
    glthread_init( &t_pool->thread_pool_lis, GET_STRUCT_OFFSET(thread_t, glue) );
    pthread_mutex_init( &t_pool->pool_mut, NULL );
}

/// @brief Insert New Thread into pool
/// @param t_pool 
/// @param thread 
void thread_pool_insert_new_thread( threadpool_t* t_pool, thread_t * thread )
{
    assert(t_pool);
    assert(thread);
    pthread_mutex_lock( &t_pool->pool_mut );
    if( !thread_in_pool(t_pool, thread) )
    {
        glthread_add_node( &t_pool->thread_pool_lis, &thread->glue );
    }
    pthread_mutex_unlock( &t_pool->pool_mut );
}

/// @brief Gets the next available thread from the pool
/// @param t_pool Thread Pool reference object
/// @return return 
thread_t* thread_pool_get_thread( threadpool_t* t_pool )
{
    thread_t* nextThread;
    pthread_mutex_lock( &t_pool->pool_mut );
    nextThread = t_pool->thread_pool_lis.head;
    if( t_pool->thread_pool_lis.head )
    {
        t_pool->thread_pool_lis.head = t_pool->thread_pool_lis.head->next;
    }
    pthread_mutex_unlock( &t_pool->pool_mut );
    return nextThread;
}

void thread_pool_dispatch_thread( threadpool_t* t_pool, void* (*thread_fn) (void*), void* arg )
{
    
}