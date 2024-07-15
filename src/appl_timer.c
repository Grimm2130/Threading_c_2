#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "../hdr/appl_timer.h"
#include "../hdr/threadlib.h"
#include <memory.h>

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
    ev->on_cycle = on_cycle;
    ev->arg = arg;
    ev->event_fn = event_fn;
    glnode_init( &ev->glue );
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
    *on_cycle = cycle;
    *on_time = time;
    #if DEBUG        
    printf("\t[%s, __DEBUG__] : Curr State [ Cycle : %d ];  Total Delay : %d; Calculated [ Event Time : %d, Event Cycle : %d ]\n", 
    __func__, curr_cycle, total_delay, *on_time, *on_cycle );
    #endif
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
    // glue thread init
    appl_t->event_glue_thread = (glthread_t*)calloc(trigger_slots, sizeof(glthread_t));      // Instantiate insertion points for each
    uint32_t _off = GET_STRUCT_OFFSET( appln_timer_events_t, glue );
    for( int i =0; i < trigger_slots; i++ )
    {
        glthread_init( &(appl_t->event_glue_thread[i]), _off );
    }
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
    appln_timer_events_t*ev = appln_timer_events_alloc(is_recurring, exe_intv, cycle, event_fn, arg );
    if( !ev )
    {
        perror("Unable to instantiate application event value");
        exit(errno);
    }
    // TODO: Add compare function to allow ordering on event's cycle
    glthread_append_node( &appl_t->event_glue_thread[time], &ev->glue );

    #if DEBUG
    printf("\t[%s, __DEBUG__] : Node Count for Slot (%d) == %d\n", __func__, time, appl_t->event_glue_thread[time].node_count );
    #endif
    // Get 
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif

    return ev;
}


/// @brief Application Timer Cycling event
/// @param arg 
/// @return 
void* appln_timer_inner_fn( void* arg )
{
    appln_timer_t* appl_t = (appln_timer_t*) arg;
    while( !appl_t->event_trig_thres || (appl_t->current_cycle < appl_t->event_trig_thres) )
    {
        while( appl_t->curr_event_slot < appl_t->trigger_slots )
        {
            #if DEBUG
            printf("\t\t[%s, __DEBUG__] : Current slot -> %d \n", __func__, appl_t->curr_event_slot);
            #endif
            glthread_t* slot = &(appl_t->event_glue_thread[ appl_t->curr_event_slot ]);
            glnode_t* node = NULL;
            GLTHREAD_ITERATOR_START( slot, node )
            {
                #if DEBUG
                printf("\t\t\t[%s, __DEBUG__] : Starting Iteration \n", __func__);
                #endif
                if( node )
                {
                    appln_timer_events_t* event = (appln_timer_events_t*)( ((uint64_t)(node)) - slot->glue_ofset );
                    if( event )
                    {
                        if( event->on_cycle <= appl_t->current_cycle )
                        {
                            assert( appl_t->current_cycle == event->on_cycle );
                            #if DEBUG
                            printf("\t\t\t[%s, __DEBUG__] : Triggering Event [Cycle, Time] = [%d, %d] \n", __func__, event->on_cycle, appl_t->curr_event_slot );
                            #endif
                           
                            event->event_fn( event->arg );
                            
                            // detach this node
                            appln_remove_event( appl_t, event, appl_t->curr_event_slot );

                            // re-insert the node if recurring
                            if( event->is_recurring )
                            {
                                #if DEBUG
                                printf("\t\t\t(%s) | [ __DEBUG__ ] Re-registering event\n", __func__ );
                                #endif
                                uint32_t next_time, next_cycle;
                                get_event_trigger_and_cycle(appl_t->trigger_slots, appl_t->current_cycle, 
                                                            (appl_t->curr_event_slot+event->ex_int), 
                                                            &next_cycle, &next_time );
                                event->on_cycle = next_cycle;
                                glthread_append_node( (&appl_t->event_glue_thread[ next_time ]), &event->glue );
                            }
                            // release event completely
                            else
                            {
                                #if DEBUG
                                printf("\t\t\t(%s) | [ __DEBUG__ ] Destroying event\n", __func__ );
                                #endif
                                free(event);
                            }
                        }
                    }
                }
                usleep( appl_t->event_trig_int );   // sleep
                #if DEBUG
                printf("\t\t\t[%s, __DEBUG__] : Ending Iteration \n", __func__);
                #endif
            }
            GLTHREAD_ITERATOR_END( (&slot) )
            appl_t->curr_event_slot++;
             #if DEBUG
            printf("\t\t[%s, __DEBUG__] : Updating slot: %d\n", __func__, appl_t->curr_event_slot );
            #endif
        }
        appl_t->current_cycle++;
        #if DEBUG
        printf("\t\t[%s, __DEBUG__] : Updating Cycle: %d\n", __func__, appl_t->current_cycle );
        #endif
        appl_t->curr_event_slot = 0;
    }
}

void appln_timer_start( appln_timer_t* appl_t )
{
    assert(appl_t);
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif

    pthread_create( &appl_t->th, NULL, appln_timer_inner_fn, appl_t );
    
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void appln_remove_event( appln_timer_t* appl_t, appln_timer_events_t* event, uint32_t idx )
{
    assert(appl_t);
    assert(event);
    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] >>>Enter\n", __func__ );
    #endif

    gl_remove_node( (glthread_t*)&appl_t->event_glue_thread[idx], (glnode_t*)&event->glue );

    #if DEBUG
    printf("\t(%s) | [ __DEBUG__ ] <<<Exit\n", __func__ );
    #endif
}
