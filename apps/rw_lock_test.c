#include "../hdr/rw_lock.h"
#include "../hdr/rec_mut.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define READER_CNT  5
rec_mut_t mut = {0};
rw_lock_t lock = {0};
pthread_t writer = 0, readers[READER_CNT] = {0};
int writer_cnt = 0, reader_cnt = 0;
static int data = 0;

void* _read( void* arg );
void* _write( void* arg );

int main()
{
    rec_mut_init( &mut );
    rw_lock_init( &lock, true );
    pthread_create( &writer, NULL, _write, NULL );
    for (int i = 0; i < READER_CNT; i++)
    {
        pthread_create( &readers[i], NULL, _read, NULL );
    }

    pthread_join( writer, NULL );
    
    for (int i = 0; i < READER_CNT; i++)
    {
        pthread_join( readers[i], NULL );
    }
    return 0;
}


void* _read( void* arg )
{
    while(1)
    {
        rw_lock_rd_lock( &lock );
        rec_mut_lock( &mut );
        reader_cnt++;
        rec_mut_unlock( &mut );
        #if DEBUG
        printf("\t(%s) | [ __DEBUG__ ] %ld Status : Readers : %d; Writers : %d\n", __func__, pthread_self(), reader_cnt, writer_cnt );
        fflush(stdout);
        #endif
        // assert( (reader_cnt > 0) && (writer_cnt == 0) );
        #if DEBUG
        printf("\t(%s) | [ __DEBUG__ ] %ld Reading value : %d\n", __func__, pthread_self(), data );
        #endif
        rec_mut_lock( &mut );
        reader_cnt--;
        rec_mut_unlock( &mut );
        rw_lock_unlock( &lock );
        // sleep(1);
    }
    pthread_exit(NULL);
}

void* _write( void* arg )
{
    while(1)
    {
        rw_lock_wr_lock( &lock );
        writer_cnt++;
        #if DEBUG
        printf("\t(%s) | [ __DEBUG__ ] %ld Status : Readers : %d; Writers : %d\n", __func__, pthread_self(), reader_cnt, writer_cnt );
        fflush(stdout);
        #endif
        assert( (reader_cnt == 0) && (writer_cnt == 1) );
        #if DEBUG
        printf("\t(%s) | [ __DEBUG__ ] %ld Writing value : %d\n", __func__, pthread_self(), data );
        #endif
        data++;
        writer_cnt--;
        rw_lock_unlock( &lock );
        // sleep(1);
    }
    pthread_exit(NULL);
}