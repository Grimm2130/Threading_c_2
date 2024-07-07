#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "../hdr/gl.h"

void glnode_init( glnode_t* node )
{
    assert(node);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    node->prev = node->next = NULL;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void glnode_detach( glnode_t* node )
{
    glnode_t *p, *n;
    p = node->prev;
    n = node->next;
    if(p) p->next = n;
    if(n) n->prev = p;
    node->prev = node->next = NULL;
}

/// @brief Gl
/// @param glt 
/// @param _offset 
void glthread_init( glthread_t* glt, uint32_t _offset )
{
    assert(glt);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    glt->node_count = 0;
    glt->glue_ofset = _offset;
    glt->head = NULL;
    glt->tail = NULL;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}


/// @brief Add the
/// @param glt 
/// @param gln 
void glthread_prepend_node( glthread_t* glt, glnode_t *gln )
{
    assert(glt);
    assert(gln);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    if( !glt->head )
    {
        glt->head = gln;
        glt->tail = glt->head;
    }
    else
    {
        gln->next = glt->head;
        glt->head->prev = gln;
        glt->head = gln;
    }
    glt->node_count++;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}

void glthread_append_node( glthread_t* glt, glnode_t *gln )
{
    assert(glt);
    assert(gln);
    #if DEBUG
    printf("\t[%s, __DEBUG__] : >> Enter \n", __func__);
    #endif
    
    if( !glt->tail )
    {
        glt->tail = gln;
        glt->head = glt->tail;
    }
    else
    {
        glt->tail->next = gln;
        gln->prev = glt->tail;
        glt->tail = gln;
    }
    glt->node_count++;
    #if DEBUG
    printf("\t[%s, __DEBUG__] : << Exit \n", __func__);
    #endif
}