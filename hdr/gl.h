#ifndef __GL_THREAD_H__
#define __GL_THREAD_H__

#include <stdint.h>
#include <assert.h>

typedef struct glnode
{
    struct glnode *prev, *next;
}glnode_t;

void glnode_init( glnode_t* node );

typedef struct glthread
{
    uint32_t base_addr;
    uint32_t node_count;
    glnode_t *head;
}glthread_t;


void glthread_init( glthread_t* glt, uint32_t _offset );
void glthread_add_node( glthread_t* glt, glnode_t *gln );

#define GLTHREAD_ITERATOR_START( glt, node_ptr )    \
{   \
    glnode_t* curr = glt->head; \
    while( curr )   \
    {   \
        node_ptr = curr;    \
        curr = curr->next;  

#define GLTHREAD_ITERATOR_END( glnode_head )  \
    }   \
}

#endif    // __GL_THREAD_H__