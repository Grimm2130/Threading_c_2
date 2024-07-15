#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "../hdr/tr_light.h"
#include "../hdr/threadlib.h"
#include <memory.h>


static inline char* get_tr_light_name_from_dir( const TR_FACES dir )
{
    char* name = (char*)calloc(NAME_LEN, sizeof(char));
    char* temp;
    switch (dir)
    {
    case TR_FACE_NORTH:
        temp = "North";
        break;
    case TR_FACE_EAST:
        temp = "East";
        break;
    case TR_FACE_SOUTH:
        temp = "South";
        break;
    case TR_FACE_WEST:
        temp = "West";
        break;
    default:
        break;
    }
    memcpy( name, temp, strlen(temp));
    return name;
}

/// @brief Traffic Light Instantiation
/// @param tr 
/// @param dir 
void tr_light_init( tr_light_t* tr, const TR_FACES dir )
{
    assert(tr);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    tr->dir = dir;
    tr->log_name = get_tr_light_name_from_dir(dir);
    pthread_mutex_init( &tr->log_mut, NULL );
    tr->car_wq = wait_queue_alloc();
    char fName[100];
    memset( fName, '\0', 100);
    sprintf( fName, "%s.%s", tr->log_name, "txt" );
    tr->fp = fopen(fName , "w+");
    if( !tr->fp )
    {
        perror("Failed to open file");
        exit( errno );
    }
    fprintf( tr->fp, "Hello from %s\n", tr->log_name);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : Hello from %s", __func__, tr->log_name);
    #endif
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

/// @brief Reads the state and returns it's value
/// @param arg 
/// @param mut 
/// @return 
bool tr_light_wait_on_red( void* arg, pthread_mutex_t** mut )
{
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif
    tr_light_t* tr = (tr_light_t*)arg;
    bool res;

    pthread_mutex_lock( &(tr->log_mut) );
    res = tr->curr_color == TR_COLORS_RED;
    pthread_mutex_unlock( &(tr->log_mut) );

    // If result is true
    if( res )
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] Light is red, block\n", __func__ );
        #endif
        pthread_mutex_lock( &(tr->log_mut) );
        if( mut )
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] Caching Mutex\n", __func__ );
            #endif
            (*mut) = &(tr->log_mut);
        }
        else
        {
            #if DEBUG
            printf("\t\t\t(%s) | [ __DEBUG__ ] Unlocking mutex\n", __func__ );
            #endif
            pthread_mutex_unlock( &(tr->log_mut) );
        }
    }
    else
    {
        #if DEBUG
        printf("\t\t(%s) | [ __DEBUG__ ] Light is Green\n", __func__ );
        #endif
    }

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
    return res;
}

/// @brief Traffic Light Update Count
/// @param tr 
void tr_light_update_color( tr_light_t* tr )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    assert(tr);
   
    tr->curr_color = (tr->curr_color == TR_COLORS_RED) ? TR_COLORS_GREEN : TR_COLORS_RED;
   
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}