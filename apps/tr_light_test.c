#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../hdr/threadlib.h"
#include "../hdr/tr_light.h"

#define S_AS_US(s)  ((uint32_t)(s * 1000000))

const uint32_t timer_slots = 10;
const uint32_t timer_trigger_period = S_AS_US(1); // 5 s
const uint32_t timer_cycle_thres = 0;
static appln_timer_t timer;
static tr_light_t tr_post[TR_LIGHT_COUNT] = {0};
static volatile int car_counts[TR_LIGHT_COUNT];

struct tp_fn_arg
{
    tr_light_t* post;
    uint8_t lights;
};

static void update_traffic_lights( void * );
static void launch_car_thread( void*);
static void* car( void* );

int main()
{
    memset( (void*)&car_counts, 0, sizeof(int) * TR_LIGHT_COUNT);
    // Instantiate lights
    tr_light_init(&tr_post[0], TR_FACE_NORTH );
    tr_light_init(&tr_post[1], TR_FACE_EAST );
    tr_light_init(&tr_post[2], TR_FACE_WEST );
    tr_light_init(&tr_post[3], TR_FACE_SOUTH );
    
    tr_post[0].curr_color = tr_post[2].curr_color = TR_COLORS_RED;
    tr_post[1].curr_color = tr_post[3].curr_color = TR_COLORS_GREEN;

    // Instantiate the timer
    appln_timer_init( &timer, timer_slots, timer_trigger_period, timer_cycle_thres );
    struct tp_fn_arg arg = {
        .post = tr_post, 
        .lights = 4
    };
    appln_timer_events_t* updt_trl = appln_timer_reg_event( &timer, true, 3, update_traffic_lights, &arg );
    
    appln_timer_events_t* north = appln_timer_reg_event( &timer, true, 5, launch_car_thread, &tr_post[0] );
    appln_timer_events_t* south = appln_timer_reg_event( &timer, true, 5, launch_car_thread, &tr_post[1] );
    appln_timer_events_t* east = appln_timer_reg_event( &timer, true, 5, launch_car_thread, &tr_post[2] );
    appln_timer_events_t* west = appln_timer_reg_event( &timer, true, 5, launch_car_thread, &tr_post[3] );

    appln_timer_start( &timer );

    // pause
    while (1);
    pthread_exit(NULL);
}

static void update_traffic_lights( void* arg )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    struct tp_fn_arg* tp_args = (struct tp_fn_arg*)arg;
    tr_light_t* post = tp_args->post;
    uint8_t lights = tp_args->lights;
    for( uint8_t i = 0; i < lights; i++ )
    {
        tr_light_update_color( (tr_light_t*)&post[i] );
    }
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

static void launch_car_thread( void* arg)
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    pthread_attr_t mut_attr = {0};
    pthread_t t = {0};

    pthread_attr_init( &mut_attr );
    pthread_attr_setdetachstate( &mut_attr, PTHREAD_CREATE_DETACHED );

    pthread_create( &t, &mut_attr, car, arg);

    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}


static void* car( void* arg)
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    // init light objec
    tr_light_t* light = (tr_light_t*)arg;
    
    timer_t t = {0};
    time(&t);
    // car arrives at light
    fprintf(light->fp, "(%s) Thread [%ld] : Car %d Arrives At Light; %d Face %s\n", 
            ctime(&t), pthread_self(), ++car_counts[light->dir-TR_FACE_NORTH], light->log_name );
    printf("(%s) Thread [%ld] : Car %d Arrives At Light; %d Face %s\n", 
            ctime(&t), pthread_self(), ++car_counts[light->dir-TR_FACE_NORTH], light->log_name );
    
    // checks if can pass
    wait_queue_test_and_wait( light->car_wq, tr_light_wait_on_red, light );
    wait_queue_broadcast( light->car_wq );
    
    fprintf(light->fp, "(%s) Thread [%ld] : Car %d passes Light on Green; %d Face %s\n", 
            ctime(&t), pthread_self(), ++car_counts[light->dir-TR_FACE_NORTH], light->log_name );
    printf("(%s) Thread [%ld] : Car %d passes Light on Green; %d Face %s\n", 
            ctime(&t), pthread_self(), ++car_counts[light->dir-TR_FACE_NORTH], light->log_name );
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    pthread_exit(NULL);
}