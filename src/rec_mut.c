#include "../hdr/rec_mut.h"
#include <assert.h>
#include <pthread.h>
#include <errno.h>

void rec_mut_init( rec_mut_t* r_mut )
{
    assert(r_mut);
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    
    pthread_cond_init( &r_mut->cv, NULL );
    pthread_mutex_init( &r_mut->mut, NULL );
    r_mut->indexed = 0;
    r_mut->lock_depth = 0;
    
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
}

void rec_mut_lock( rec_mut_t* r_mut )
{
    assert( r_mut );
    #if DEBUG
   printf("\t(%s) | [ __DEBUG__ ] >>> %ld Enter\n", __func__, pthread_self() );
    #endif

    pthread_mutex_lock(&r_mut->mut);

    assert( r_mut->lock_depth >= 0 );

    // initial lock
    if(r_mut->lock_depth == 0 )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] Intial lock Caching thread [%ld]\n", __func__, pthread_self() );
        #endif
        assert(r_mut->indexed == 0 );
        r_mut->indexed = pthread_self();            // cache self
    }
    // Ensure only one thread
    else if(  r_mut->lock_depth > 0 && (r_mut->indexed != pthread_self()) )
    {
        #if DEBUG
        printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Wait for valid state\n", __func__, pthread_self() );
        #endif

        while( r_mut->lock_depth > 0 )
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Waiting...\n", __func__, pthread_self() );
            #endif
            pthread_cond_wait( &r_mut->cv, &r_mut->mut );
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Signalled!!\n", __func__, pthread_self() );
            #endif
        }
        r_mut->indexed = pthread_self();            // cache new self
    }
    
    r_mut->lock_depth++;                // increment lock depth

    pthread_mutex_unlock(&r_mut->mut);

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
}

void rec_mut_unlock( rec_mut_t* r_mut )
{
    assert( r_mut );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>> %ld Enter\n", __func__, pthread_self() );
    #endif

    pthread_mutex_lock(&r_mut->mut);

    assert( r_mut->lock_depth > 0 );
    assert( r_mut->indexed == pthread_self() );
    r_mut->lock_depth--;                // increment lock depth
    
    if( r_mut->lock_depth == 0 )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] %ld Signalling blocked threads\n", __func__, pthread_self() );
        #endif
        pthread_cond_signal( &r_mut->cv );
        r_mut->indexed = 0;
    }

    pthread_mutex_unlock(&r_mut->mut);

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld <<<Exit\n", __func__, pthread_self() );
    #endif
}

void rec_mut_destroy( rec_mut_t* r_mut )
{
    assert(r_mut);
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif

    assert( r_mut->lock_depth == 0 );
    pthread_mutex_destroy( &r_mut->mut );
    pthread_cond_destroy( &r_mut->cv );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld <<<Exit\n", __func__, pthread_self() );
    #endif
}