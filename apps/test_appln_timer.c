#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../hdr/tr_light.h"

#define S_AS_US(s)  ((uint32_t)(s * 1000000))

static char msg1[10];
static char msg2[10];   
static char msg3[10];
const uint32_t timer_slots = 10;
const uint32_t timer_trigger_period = S_AS_US(1); // 5 s
const uint32_t timer_cycle_thres = 0;
static appln_timer_t timer;

void print( void * arg );

int main()
{
    appln_timer_init( &timer, timer_slots, timer_trigger_period, timer_cycle_thres );

    appln_timer_events_t* ev1, *ev2, *ev3;

    memset( (void*)&msg1[0], '\0', 10 );   
    memset( (void*)&msg2[0], '\0', 10 );   
    memset( (void*)&msg3[0], '\0', 10 );   
   
    memcpy( (void*)&msg1[0], "Event 1", strlen("Event x") );   
    memcpy( (void*)&msg2[0], "Event 2", strlen("Event x") );   
    memcpy( (void*)&msg3[0], "Event 3", strlen("Event x") );   

    printf("Messages : %s %s %s\n", msg1, msg2, msg3 );

    ev1 = appln_timer_reg_event( &timer, true, 3, print, (void*)&msg1[0] );
    ev2 = appln_timer_reg_event( &timer, true, 5, print, (void*)&msg2[0] );
    ev3 = appln_timer_reg_event( &timer, true, 7, print, (void*)&msg3[0] );

    appln_timer_start( &timer );

    while (1);
    return 0;
}

void print( void * arg )
{
    char* msg = (char*)arg;

    printf("Msg: %s\n", msg);
}