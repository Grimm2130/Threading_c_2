#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "../hdr/tr_light.h"
#include "../hdr/gl.h"

/*------------------------------------------------------*/
/*----------------Application Timer Event----------------*/
/*------------------------------------------------------*/
appln_timer_events_t* appln_timer_events_alloc( bool is_recurring, uint32_t exe_intv, uint32_t on_cycle,  appln_timer_events_cb event_fn, void* arg )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    appln_timer_events_t* ev = (appln_timer_events_t*) calloc(1, sizeof(appln_timer_events_t));
    appln_timer_events_init( ev, is_recurring, exe_intv, on_cycle, event_fn, arg );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    return ev;
}

void appln_timer_events_init( appln_timer_events_t* ev, bool is_recurring, uint32_t exe_intv, uint32_t on_cycle,  appln_timer_events_cb event_fn, void* arg )
{
    assert(ev);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    ev->is_recurring = true;
    ev->ex_int = exe_intv;
    ev->on_cyle = on_cycle;
    ev->arg = arg;
    ev->event_fn = event_fn;
    ev->glue = (glnode_t*) calloc(1, sizeof(glnode_t));
    glnode_init( ev->glue );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}


/*------------------------------------------------------*/
/*----------------Application Timer ----------------*/
/*------------------------------------------------------*/

/* Helpers */
static inline void get_event_trigger_and_cycle( const uint32_t trig_slots, const uint32_t curr_cycle, const uint32_t total_delay, uint32_t* volatile on_cycle, uint32_t* volatile on_time )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    int time, cycle; 
    time = total_delay%trig_slots;
    cycle = curr_cycle + (total_delay/trig_slots);
    #if DEBUG        
    printf("\t[%s, __DEBUG__] : Curr State [ Cycle : %d ];  Total Delay : %d; Calculated [ Event Time : %d, Event Cycle : %d ]\n", 
    __func__, curr_cycle, total_delay, *on_time, *on_cycle );
    #endif
    *on_cycle = cycle;
    *on_time = time;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

appln_timer_t* appln_timer_alloc( const uint32_t trigger_slots, const uint32_t timer_intv, const uint32_t thres )
{
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    appln_timer_t* timer = (appln_timer_t*) calloc(1, sizeof(appln_timer_t));
    appln_timer_init( timer, trigger_slots, timer_intv, thres );
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
    return timer;
}

void appln_timer_init( appln_timer_t* appl_t, const uint32_t trigger_slots, const uint32_t timer_intv, const uint32_t thres )
{
    assert(appl_t);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    // configs
    appl_t->trigger_slots = trigger_slots;
    appl_t->event_trig_int = timer_intv;
    appl_t->event_trig_thres = thres;
    // variables
    appl_t->curr_event_slot = appl_t->current_cycle = 0;
    // glue thread
    appl_t->event_glue_thread = (glthread_t*)calloc(trigger_slots, sizeof(glthread_t));      // Instantiate insertion points for each
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

appln_timer_events_t* appln_timer_reg_event( appln_timer_t* appl_t, bool is_recurring, uint32_t exe_intv, appln_timer_events_cb event_fn, void* arg )
{
    assert(appl_t);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    // get event trigger time and cycle
    uint32_t cycle, time;
    get_event_trigger_and_cycle( appl_t->trigger_slots, appl_t->current_cycle, (appl_t->curr_event_slot + exe_intv), &cycle, &time );

    appln_timer_events_t*ev = appln_timer_events_alloc(is_recurring, time, cycle, event_fn, arg );
    if( !ev )
    {
        perror("Unable to instantiate application event value");
        exit(errno);
    }

    // TODO: Add compare function to allow ordering on event's cycle
    glthread_append_node( &appl_t->event_glue_thread[time], ev->glue )

    // Get 
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}