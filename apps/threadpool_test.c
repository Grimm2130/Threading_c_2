#include "../hdr/threadlib.h"
#include <stdio.h>
#include <stdlib.h>

#define ARR_SIZE    100

static const int arr_size = ARR_SIZE;
int arr[ ARR_SIZE ] = {0};
threadpool_t t_pool;

void *print_odds( void* arg );
void *print_evens( void* arg );
void *print_odds_reverse( void* arg );
void *print_evens_reverse( void* arg );


int main()
{
    printf("Instantiating threads...\n");
    threadpool_init_with_threads( &t_pool, 4);
    // Dispatch the threads
    printf("Dispatching threads\n");
    threadpool_dispatch_thread( &t_pool, print_odds, arr, true );
    threadpool_dispatch_thread( &t_pool, print_evens, arr, true );
    threadpool_dispatch_thread( &t_pool, print_odds_reverse, arr, true );
    threadpool_dispatch_thread( &t_pool, print_evens_reverse, arr, true );

    return 0;
}

/// @brief Print Odds
/// @param arg 
/// @return 
void *print_odds( void* arg )
{
    printf("Odds : ");
    int* arr = (int*)arg;
    for( int i = 0; i < arr_size; i++ )
    {
        if( i & 1 )
        {
            printf("%d ", i);
        }
    }
    printf("\n");
}

/// @brief Print Evens 
/// @param arg 
/// @return 
void *print_evens( void* arg )
{
    printf("Evens : ");
    int* arr = (int*)arg;
    for( int i = 0; i < arr_size; i++ )
    {
        if( !(i & 1) )
        {
            printf("%d ", i);
        }
    }
    printf("\n");
}

/// @brief Print the odds in reverse
/// @param arg 
/// @return 
void *print_odds_reverse( void* arg )
{
    printf("Odds Reverse: ");
    int* arr = (int*)arg;
    for( int i = arr_size-1; i >= 0 ; i-- )
    {
        if( i & 1 )
        {
            printf("%d ", i);
        }
    }
    printf("\n");
}

/// @brief Print Evens in reverse
/// @param arg 
/// @return 
void *print_evens_reverse( void* arg )
{
    printf("Evens Reverse: ");
    int* arr = (int*)arg;
    for( int i = arr_size-1; i >= 0 ; i-- )
    {
        if( !(i & 1) )
        {
            printf("%d ", i);
        }
    }
    printf("\n");
}