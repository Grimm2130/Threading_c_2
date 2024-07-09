#ifndef __TR_LIGHT_H__
#define __TR_LIGHT_H__

#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Forward Declarations
typedef struct glnode glnode_t;
typedef struct glthread glthread_t;
typedef struct wait_queue wait_queue_t;

// traffic light color enumerations
typedef enum TR_COLORS
{
    RED,
    YELLOW,
    GREEN
}TR_COLORS_ENUM;

//

typedef void (*appln_timer_events_cb) (void*);
/// @brief Application timer event
typedef struct appln_timer_events
{
    bool is_recurring;
    uint32_t ex_int;            // defined in us
    uint32_t on_cyle;
    void* arg;
    appln_timer_events_cb event_fn;
    glnode_t* glue;
}appln_timer_events_t;


appln_timer_events_t* appln_timer_events_alloc( bool is_recurring, uint32_t exe_intv, appln_timer_events_cb event_fn, void* arg );
void appln_timer_events_init( appln_timer_events_t* ev, bool is_recurring, uint32_t exe_intv, appln_timer_events_cb event_fn, void* arg );


/// @brief Application level timer
typedef struct appln_timer
{
    // config fields
    uint32_t cycles;
    uint32_t event_trig_int;
    uint32_t event_trig_thres;      // max number of trigger cycles
    // Auxiliary fields
    pthread_t th;
    pthread_mutex_t mut;
    // variable fields
    uint32_t current_cycle;
    uint32_t curr_event_slot;
    glthread_t *glue_thread;
}appln_timer_t;

appln_timer_t* appln_timer_alloc( const uint32_t cycles, const uint32_t timer_intv, const uint32_t thres );
void appln_timer_alloc( appln_timer_t* appl_t, const uint32_t cycles, const uint32_t timer_intv, const uint32_t thres );
appln_timer_events_t* appln_timer_reg_event( appln_timer_t* appl_t, bool is_recurring, uint32_t exe_intv, appln_timer_events_cb event_fn, void* arg );

/// @brief Traffic light
typedef struct tr_light
{  
    TR_COLORS_ENUM curr_color;
    wait_queue_t* car_wq;
    char* log_name;
    pthread_mutex_t log_mut;
}tr_light_t;

void tr_light_init( tr_light_t* tr );
void tr_light_update_color( tr_light_t* tr );

#endif /* __TR_LIGHT_H__ */
