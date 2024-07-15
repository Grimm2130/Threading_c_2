#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../hdr/wait_queue.h"
#include "../hdr/tr_light.h"
#include "../hdr/gl.h"

wait_queue_t* wait_queue_alloc()
{
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    wait_queue_t* new = (wait_queue_t*) calloc(1, sizeof(wait_queue_t));
    wait_queue_init(new);
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
    return new;
}

void wait_queue_init( wait_queue_t* wq )
{
    assert( wq );
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    wq->blocked_threads = 0;
    wq->resource_mut_cache = NULL;
    pthread_cond_init( &wq->wq_cv, NULL );
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
}

void wait_queue_test_and_wait( wait_queue_t* wq, wait_queue_cond_fn resource_check_fn, void* resource )
{
    assert(wq);
    assert( resource_check_fn );
    assert( resource );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    bool block;
    pthread_mutex_t* temp_ref = NULL;
    
    do
    {
        block = resource_check_fn( resource, &temp_ref );       // Get value for resource availability
        if( block )                                             // Check value for availability
        {
            #if DEBUG
            printf("\t\t(%s) | [ __DEBUG__ ] Enter Wait\n", __func__ );
            #endif
            wq->resource_mut_cache = temp_ref;                      // cache resource mutex
            wq->blocked_threads++;                              // update blocked threads count
            pthread_cond_wait( &wq->wq_cv, wq->resource_mut_cache );    // wait on resource
            // resource is automatically unlocked here
            wq->blocked_threads--;                              // decrement count
            block = resource_check_fn( resource, NULL );       // do a non blocking check, if false, we lock and wait again
            #if DEBUG
            printf("\t\t(%s) | [ __DEBUG__ ] Release Wait\n", __func__ );
            #endif
        }
    }
    while ( block );

    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void wait_queue_signal( wait_queue_t* wq )
{
    assert(wq);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif

    pthread_cond_signal( &wq->wq_cv );
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void wait_queue_broadcast( wait_queue_t* wq )
{
    assert(wq);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    pthread_cond_broadcast( &wq->wq_cv );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void wait_queue_destroy( wait_queue_t* wq )
{
    assert(wq);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    wq->blocked_threads = 0;
    wq->resource_mut_cache = NULL;
    pthread_cond_destroy( &wq->wq_cv );

    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}