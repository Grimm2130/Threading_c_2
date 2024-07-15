#ifndef __TR_LIGHT_H__
#define __TR_LIGHT_H__

#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "gl.h"

#define TR_LIGHT_COUNT 4
#define NAME_LEN 0X1F

// Forward Declarations
typedef struct wait_queue wait_queue_t;

typedef enum 
{
    TR_FACES_START,
    TR_FACE_NORTH,
    TR_FACE_SOUTH,
    TR_FACE_EAST,
    TR_FACE_WEST,
    TR_FACES_END
}TR_FACES;

// traffic light color enumerations
typedef enum TR_COLORS
{
    TR_COLORS_START = TR_FACES_START,
    TR_COLORS_RED,
    TR_COLORS_YELLOW,
    TR_COLORS_GREEN,
    TR_COLORS
}TR_COLORS_ENUM;


/// @brief Traffic light
typedef struct tr_light
{  
    TR_FACES dir;
    TR_COLORS_ENUM curr_color;
    pthread_mutex_t log_mut;
    char* log_name;
    FILE* fp;
    wait_queue_t* car_wq;
}tr_light_t;

void tr_light_init( tr_light_t* tr,  const TR_FACES dir );
bool tr_light_wait_on_red( void* arg, pthread_mutex_t** mut );
void tr_light_update_color( tr_light_t* tr );

#endif /* __TR_LIGHT_H__ */
