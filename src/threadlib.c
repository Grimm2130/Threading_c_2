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
                        // * Thread
/******************************************************************************/

/// @brief Instantiate a new thread object
/// @param th Thread object
/// @param name Name of thread
void thread_create( thread_t* th, const char * name )
{
    assert( th );
    assert( name );

    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    memset( th->name, '\0', THREAD_NAME_LEN );
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
        memcpy( th->name, name, strlen(name) );     // copy the name into the thread
    }
    // instantiate flags
    th->m_flags = 0;
    th->m_priority = 0;
    // init args
    th->arg = NULL;
    th->pause_arg = NULL;
    // Init function pointers
    th->pause_fn_ptr = NULL;
    th->thread_fn_ptr = NULL;
    // init pthread object and attr
    memset( &th->m_thread, 0, sizeof(pthread_t) );
    memset( &th->m_thread_attr, 0, sizeof(pthread_attr_t) );
    // init Semaphore
    sem_init( &th->sem, 0, 0 );        // base value is zero
    // init mutex and attr
    memset( &th->m_thread_mut, 0, sizeof(pthread_mutex_t) );
    pthread_mutex_init(&th->m_thread_mut, NULL );
    // init conditional and attr
    memset( &th->m_thread_cv, 0, sizeof(pthread_cond_t) );
    pthread_cond_init(&th->m_thread_cv, NULL );

    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Allocate memory and instantiate thread object
/// @param name Name of thread
/// @return Allocated Memory
thread_t* thread_create_alloc( const char * name )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    thread_t* newTh = (thread_t*) calloc(1, sizeof(thread_t));
    thread_create( newTh, name );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    return newTh;
}


/// @brief Set the thread as joinable
/// @param th 
/// @param joinable 
void thread_set_attr_joinable_detached( thread_t* th, bool joinable )
{
    assert(th);
    assert( !IS_BIT_SET(th->m_flags, THREAD_CREATED ) );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    pthread_attr_init( &th->m_thread_attr );                // instantiate the thread attribute
    pthread_attr_setdetachstate( &th->m_thread_attr, joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED );
    SET_BIT(th->m_flags, THREAD_DETACHED);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
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
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    th->arg = arg;
    th->thread_fn_ptr = fn_ptr;
    pthread_attr_init( &th->m_thread_attr );                // instantiate the thread attribute
    // mark created and running flags
    SET_BIT( th->m_flags, THREAD_CREATED );                 
    SET_BIT( th->m_flags, THREAD_RUNNING) ;
    if( pthread_create( &th->m_thread, &th->m_thread_attr, th->thread_fn_ptr, th->arg) != 0 )
    {
        perror("Thread creation failed");
        exit(errno);
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Pauses the thread object
/// @param th Pointer to thread object
void thread_pause( thread_t* th )
{
    assert(th);
    assert( IS_BIT_SET( th->m_flags, THREAD_CREATED ) );    // assert that thread is running and created
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    pthread_mutex_lock( &th->m_thread_mut );
    if( IS_BIT_SET( th->m_flags, THREAD_RUNNING ) )
    {
        SET_BIT( th->m_flags, THREAD_MARKED_FOR_PAUSE );
    }
    pthread_mutex_unlock( &th->m_thread_mut );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Checks that the pause flag has been set and waits for it to be unset
/// @param th Pointer to thread object
void thread_test_and_pause( thread_t* th )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    assert( th );
    pthread_mutex_lock( &th->m_thread_mut );
    // Check if the thread is marked for pause
    if( IS_BIT_SET( th->m_flags, THREAD_MARKED_FOR_PAUSE ) )
    {
        // set the THREAD_PAUSED bit
        SET_BIT( th->m_flags, THREAD_PAUSED );
        // update the running flag
        UNSET_BIT( th->m_flags, THREAD_RUNNING);
        // unset mark for THREAD_PAUSED bit
        UNSET_BIT( th->m_flags, THREAD_MARKED_FOR_PAUSE);     
        // Wait on THREAD_PAUSED bit to be unset
        while( IS_BIT_SET( th->m_flags, THREAD_PAUSED ) )
        {
            pthread_cond_wait( &th->m_thread_cv, &th->m_thread_mut );
        }
    }
    pthread_mutex_unlock( &th->m_thread_mut );
    SET_BIT( th->m_flags, THREAD_RUNNING );         // update thread's running flag
    th->pause_fn_ptr( th->pause_arg );      // run the un-THREAD_PAUSED function
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief "R"
/// @param th 
void thread_resume( thread_t* th )
{
    assert( th );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    pthread_mutex_lock( &th->m_thread_mut );
    if( IS_BIT_SET( th->m_flags, THREAD_PAUSED ) )
    {
        UNSET_BIT( th->m_flags, THREAD_PAUSED );
        pthread_cond_signal( &th->m_thread_cv );
    }
    pthread_mutex_unlock( &th->m_thread_mut );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Thread Set Pause Function
/// @param th 
/// @param pause_arg 
/// @param pause_fn 
void thread_set_pause_fn( thread_t* th, void *pause_arg, void *(*pause_fn)(void *) )
{
    assert( th );
    assert( pause_arg );
    assert( pause_fn );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    th->pause_arg = pause_arg;
    th->pause_fn_ptr = pause_fn;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Thread Destroy Functino
/// @param th 
void thread_destroy( thread_t* th )
{
    assert( !IS_BIT_SET(th->m_flags, THREAD_RUNNING) );     // thread not running
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    th->pause_arg = th->arg = NULL;
    th->pause_fn_ptr = th->thread_fn_ptr = NULL;
    pthread_attr_destroy( &th->m_thread_attr );
    pthread_mutex_destroy(&th->m_thread_mut);
    pthread_cond_destroy(&th->m_thread_cv);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/******************************************************************************/
                        // * Thread Pool Object
/******************************************************************************/

/******************************************************************************/
                        // * Static Thread Pool Operations

/// @brief Checks if the thread is in the pool
/// @param t_pool 
/// @param th 
/// @return 
static bool thread_in_pool( threadpool_t* t_pool, thread_t* th )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    glnode_t* node;
    bool res = false;
    GLTHREAD_ITERATOR_START( (&t_pool->threadpool_lis), node )
    {
        if( node )
        {
            thread_t* temp = (thread_t*)(node - t_pool->threadpool_lis.glue_ofset);
            if( temp )
            {
                if( strcmp(temp->name, th->name) == 0 )
                {
                    res = true;
                    break;
                }
            }
        }
    }
    GLTHREAD_ITERATOR_END( (&t_pool->threadpool_lis) )
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    return res;
}

/// @brief Thread Pool Operation Stage 3
/// @param t_pool 
/// @param thread 
static void threadpool_op_stage3(threadpool_t* t_pool, thread_t * thread)
{
    assert( t_pool );
    assert( thread );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    if( IS_BIT_SET( thread->m_flags, THREAD_RUNNING ) )
    {
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Reycling thread : %s\n", __func__, thread->name);
        #endif
        pthread_mutex_lock( &t_pool->pool_mut );                                    
        
        threadpool_priority_insert_thread( t_pool, thread );        // Add thread back to pool
        
        // Check if caller needs be unblocked
        if( IS_BIT_SET(thread->m_flags, THREAD_CALLER_BLOCKED ) )
        {
            UNSET_BIT(thread->m_flags, THREAD_CALLER_BLOCKED );     // unset
            sem_post( &thread->sem );
             #if DEBUG
            printf("\t\t[%s, __DEBUG__] : Unblocking Caller\n", __func__);
            #endif
        }
        UNSET_BIT( thread->m_flags, THREAD_RUNNING );
        SET_BIT( thread->m_flags, THREAD_BLOCKED );
        pthread_cond_wait( &thread->m_thread_cv, &t_pool->pool_mut );   // Hold thread on the conditional
        pthread_mutex_unlock( &t_pool->pool_mut );          
    }
    else
    {
        // ? Does this section get called by a thread not running?
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Start the thread in the pool
/// @param arg 
static void threadpool_run_thread ( thread_t* th )
{
    assert(th);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    SET_BIT( th->m_flags, THREAD_RUNNING );
    // Check if the thread has been created
    if( IS_BIT_SET(th->m_flags, THREAD_CREATED) )
    {
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Thread already created, signalling to start execution\n", __func__);
        #endif
        // signal the thread to start operation
        UNSET_BIT( th->m_flags, THREAD_BLOCKED );
        pthread_cond_signal( &th->m_thread_cv );
    }
    else
    {
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Thread not created, calling pthread_create to start execution\n", __func__);
        #endif
        thread_run( th, th->arg, th->thread_fn_ptr );
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Call the execute and recycle 
/// @param arg 
static void thread_execute_and_recycle( void* arg )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    thread_execution_data_t* thread_ex_data = (thread_execution_data_t*)arg;
    // Run the execution stage
    while (1)
    {
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Executing Application Function\n", __func__);
        #endif
        // execute the application's function
        thread_ex_data->thread_ex_fn( thread_ex_data->arg);
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Executing recycling step\n", __func__);
        #endif
        // recycle the thread
        thread_ex_data->thread_recyle( thread_ex_data->t_pool, thread_ex_data->th );
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

static int thread_priority_cmp( thread_t* a, thread_t *b )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif

    int p_a, p_b, res;
    pthread_mutex_lock( &a->m_thread_mut ); 
    p_a = a->m_priority;
    pthread_mutex_unlock( &a->m_thread_mut ); 
    pthread_mutex_lock( &b->m_thread_mut ); 
    p_b = b->m_priority;
    pthread_mutex_unlock( &b->m_thread_mut ); 

    if( p_a < p_b ) res = -1;
    else if( p_a == p_b ) res = 0;
    else res = 1; 
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    
    return res;
}

/******************************************************************************/

/// @brief Instantiate the thread pool
/// @param t_pool 
void threadpool_init( threadpool_t* t_pool )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    #if DEBUG
    printf("\t[%s, __DEBUG__] : Glue's Offset from struct base address [%d]\n", __func__, GET_STRUCT_OFFSET(thread_t, glue));
    #endif
    glthread_init( &t_pool->threadpool_lis, GET_STRUCT_OFFSET(thread_t, glue) );
    pthread_mutex_init( &t_pool->pool_mut, NULL );
    // Set default comparator
    t_pool->cmp_fn = thread_priority_cmp;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Instantiate the thread pool with threads
/// @param t_pool 
void threadpool_init_with_threads( threadpool_t* t_pool, uint8_t num_threads )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    threadpool_init( t_pool );
    char* name = (char*)calloc( THREAD_NAME_LEN, sizeof(char));
    // Instantiate and insert threads
    for( int i = 0; i < num_threads; i++ )
    {
        thread_t* t = (thread_t*) calloc(1, sizeof(thread_t));
        memset( name, '\0', THREAD_NAME_LEN );          // reset
        sprintf( name, "Thread %d", i);

        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Setting thread name as %s \n", __func__, name);
        #endif
        
        thread_create( t, name );
        threadpool_insert_new_thread( t_pool, t );
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : ** \n", __func__);
        #endif
    }
    free( name );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Insert New Thread into pool
/// @param t_pool 
/// @param thread 
void threadpool_insert_new_thread( threadpool_t* t_pool, thread_t * thread )
{
    assert(t_pool);
    assert(thread);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    // mutex lock
    pthread_mutex_lock( &t_pool->pool_mut );
    if( !thread_in_pool(t_pool, thread) )       // ensure that the thread is not in the pool
    {
        glthread_append_node( &t_pool->threadpool_lis, &thread->glue );
    }
    pthread_mutex_unlock( &t_pool->pool_mut );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Thread Priority insert
/// @param t_pool 
/// @param thread 
void threadpool_priority_insert_thread( threadpool_t* t_pool, thread_t * thread )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif

    if( t_pool->threadpool_lis.node_count == 0 )
    {
        threadpool_insert_new_thread( t_pool, thread );
    }
    else
    {
        bool inserted = false;
        glnode_t *node = NULL;
        GLTHREAD_ITERATOR_START( (&t_pool->threadpool_lis), node )
        {
            if( node )
            {
                thread_t* currTh = (thread_t*)( (uint64_t)(node) - t_pool->threadpool_lis.glue_ofset );
                if( t_pool->cmp_fn( thread, currTh ) > 0 )  //
                {
                    glnode_t* prev = node->prev;
                    if( prev )
                    {
                        thread->glue.prev = prev;
                        thread->glue.next = node;
                        prev->next = &(thread->glue);
                        node->prev = &(thread->glue);
                    }
                    else
                    {
                       glthread_prepend_node( &t_pool->threadpool_lis, &thread->glue );
                    }
                    inserted = true;
                    break;
                }
            }
        }
        GLTHREAD_ITERATOR_END( (&t_pool->threadpool_lis) )

        if( !inserted )
        {
            glthread_append_node( &t_pool->threadpool_lis, &thread->glue );
        }
    }
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Gets the next available thread from the pool
/// @param t_pool Thread Pool reference object
/// @return return 
thread_t* threadpool_get_thread( threadpool_t* t_pool )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    thread_t* nextThread = NULL;
    pthread_mutex_lock( &t_pool->pool_mut );
    glnode_t* node = t_pool->threadpool_lis.head;
    if( node )
    {
        nextThread = (thread_t*)( (uint64_t)node - (uint64_t)t_pool->threadpool_lis.glue_ofset);
        t_pool->threadpool_lis.head = t_pool->threadpool_lis.head->next;        // Update head
        glnode_detach( node );
    }
    pthread_mutex_unlock( &t_pool->pool_mut );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    return nextThread;
}


void threadpool_set_cmp_fn( threadpool_t* t_pool, int (*cmp_fn)(thread_t*, thread_t*) )
{
    assert( t_pool);
    assert( cmp_fn );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    t_pool->cmp_fn = cmp_fn;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void threadpool_dispatch_thread( threadpool_t* t_pool, void* (*thread_fn) (void*), void* arg, bool block_caller )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    //  Get the thread from the queue
    thread_t* nextTh = threadpool_get_thread( t_pool );
    // Validate thread is not null
    if( nextTh )
    {
        // Update the thread data arguments
        thread_execution_data_t* thread_ex_data = (thread_execution_data_t*)nextTh->arg;
        if( !thread_ex_data )
        {
            // newly added thread Update data
            thread_ex_data = (thread_execution_data_t*)calloc(1, sizeof(thread_execution_data_t));
        }
        // Set execution data values
        thread_ex_data->arg = arg;
        thread_ex_data->thread_ex_fn = thread_fn;
        thread_ex_data->thread_recyle = (void*)(&threadpool_op_stage3);
        thread_ex_data->t_pool = t_pool;
        thread_ex_data->th = nextTh;

        nextTh->arg = thread_ex_data;       // Ensure argument address is referenced
        nextTh->thread_fn_ptr = (void*)&thread_execute_and_recycle; // Set the thread's entery funtion

        assert( !thread_in_pool( t_pool, nextTh) );         // Ensure thread has been removed from pool

        threadpool_run_thread( nextTh );

        if( block_caller )
        {
            SET_BIT( nextTh->m_flags, THREAD_CALLER_BLOCKED );
            #if DEBUG
            printf("\t\t[%s, __DEBUG__] : Blocking Caller\n", __func__);
            #endif
            sem_wait( &nextTh->sem );       // Block caller
        }

    }
    else
    {
        perror("Unable to retrieve thread");
        errno = -1;
        exit(errno);;
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}