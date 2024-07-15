#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "gl.h"

typedef void (*appln_timer_events_cb) (void*);
/// @brief Application timer event
typedef struct appln_timer_events
{
    bool is_recurring;
    uint32_t ex_int;            // defined in us
    uint32_t on_cycle;
    appln_timer_events_cb event_fn;
    glnode_t glue;
    void* arg;
}appln_timer_events_t;


appln_timer_events_t* appln_timer_events_alloc( bool is_recurring, uint32_t exe_intv, uint32_t on_cycle, appln_timer_events_cb event_fn, void* arg );
void appln_timer_events_init( appln_timer_events_t* ev, bool is_recurring, uint32_t exe_intv, uint32_t on_cycle, appln_timer_events_cb event_fn, void* arg );
void appln_timer_events_destroy( appln_timer_events_t* ev );


/// @brief Application level timer
typedef struct appln_timer
{
    // config fields
    uint32_t trigger_slots;
    uint32_t event_trig_int;
    uint32_t event_trig_thres;      // max number of trigger trigger_slots
    // Auxiliary fields
    pthread_t th;
    // pthread_mutex_t mut;
    // variable fields
    uint32_t current_cycle;
    uint32_t curr_event_slot;
    glthread_t *event_glue_thread;
}appln_timer_t;

appln_timer_t* appln_timer_alloc( const uint32_t trigger_slots, const uint32_t timer_intv, const uint32_t thres );
void appln_timer_init( appln_timer_t* appl_t, const uint32_t trigger_slots, const uint32_t timer_intv, const uint32_t thres );
appln_timer_events_t* appln_timer_reg_event( appln_timer_t* appl_t, bool is_recurring, uint32_t exe_intv, appln_timer_events_cb event_fn, void* arg );
void appln_timer_start( appln_timer_t* appl_t );
void appln_remove_event( appln_timer_t* appl_t, appln_timer_events_t* event, uint32_t idx );