#include "../hdr/rw_lock.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

void rw_lock_init( rw_lock_t* lock, bool is_recursive )
{
    assert(lock);
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    lock->has_readers = lock->has_writer = false;
    lock->lock_depth = 0;
    lock->is_recursive = is_recursive;
    pthread_mutex_init( &lock->mut, NULL );
    pthread_cond_init( &lock->cv, NULL );
    lock->indexed_writer = 0;
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
}

rw_lock_t* rw_lock_alloc( bool is_recursive )
{
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    rw_lock_t* lock = (rw_lock_t*) calloc(1, sizeof(rw_lock_t));
    rw_lock_init( lock, is_recursive );
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
    return lock;
}

void rw_lock_rd_lock( rw_lock_t* lock )
{
    assert(lock);

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif

    pthread_mutex_lock( &lock->mut );

    // Check if there's a writer
    if( lock->has_writer )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] %ld Waiting for writer thread to exit\n", __func__, pthread_self() );
        #endif
        while( lock->has_writer )
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Waiting....\n", __func__, pthread_self() );
            #endif

            pthread_cond_wait( &lock->cv, &lock->mut );

           #if DEBUG
           printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Releasing...\n", __func__, pthread_self() );
           #endif
        }
    }   

    // increment the reader count
    lock->has_readers = true;
    lock->lock_depth++;

    pthread_mutex_unlock( &lock->mut );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif

}

void rw_lock_wr_lock( rw_lock_t* lock )
{
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld >>>Enter\n", __func__, pthread_self() );
    #endif

    pthread_mutex_lock( &lock->mut );
    if( lock->has_readers )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] %ld Waiting for readers to leave\n", __func__, pthread_self() );
        #endif

        while( lock->has_readers )
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Waiting....\n", __func__, pthread_self() );
            #endif

            pthread_cond_wait( &lock->cv, &lock->mut );

            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Releasing...\n", __func__, pthread_self() );
            #endif
        }
    }

    if( lock->lock_depth == 0 )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] %ld Caching writer\n", __func__, pthread_self() );
        #endif
    }
    else if( (lock->has_writer) && (lock->indexed_writer != pthread_self()) )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] %ld Waiting for recursive writer to exit\n", __func__, pthread_self() );
        #endif

        while( lock->has_writer )
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Waiting....\n", __func__, pthread_self() );
            #endif

            pthread_cond_wait( &lock->cv, &lock->mut );

            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] %ld Releasing...\n", __func__, pthread_self() );
            #endif
        }
    }
    lock->indexed_writer = pthread_self();          // cache writer id
    lock->has_writer = true;
    lock->lock_depth++;

    pthread_mutex_unlock( &lock->mut );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld <<<Exit\n", __func__, pthread_self() );
    #endif
}


void rw_lock_unlock( rw_lock_t* lock )
{
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld >>>Enter\n", __func__, pthread_self() );
    #endif

    pthread_mutex_lock( &lock->mut );

    assert( !(lock->has_readers && lock->has_writer) );

    if( lock->has_readers )
    {
        lock->lock_depth--;
        assert( lock->lock_depth >= 0 );
        if( lock->lock_depth == 0 )
        {
            lock->has_readers = false;
        }
    }
    else
    {
        assert( lock->indexed_writer == pthread_self() );
        lock->lock_depth--;
        assert( lock->lock_depth >= 0 );
        if( lock->lock_depth == 0 )
        {
            lock->has_writer = false;
        }
        lock->indexed_writer = 0;
    }

    pthread_cond_broadcast( &lock->cv );

    pthread_mutex_unlock( &lock->mut );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] %ld <<<Exit\n", __func__, pthread_self() );
    #endif
}