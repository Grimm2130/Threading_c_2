#include "../hdr/rec_mut.h"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TH_CNT  10

static int val = 0;
rec_mut_t mut;
pthread_t ths[TH_CNT] = {0};

void *fnc( void*arg );

int main()
{
    rec_mut_init( &mut );
    for( int i = 0; i < TH_CNT; i++ )
    {
        pthread_create( &ths[i], NULL, fnc, NULL );
    }

    for( int i = 0; i < TH_CNT; i++ )
    {
        pthread_join( ths[i], NULL );
    }

    printf("passed");
    assert( val == 10 );


    rec_mut_destroy( &mut );
    return 0;
}

void *fnc( void*arg )
{
    int depth = 5;
    printf("Enter function\n");
    for( int i = 0; i < depth; i++)
        rec_mut_lock( &mut );
    
    printf("Perform Operation\n");
    val++;

    for( int i = 0; i < depth; i++)
        rec_mut_unlock( &mut );
    printf("Exit function\n");
}