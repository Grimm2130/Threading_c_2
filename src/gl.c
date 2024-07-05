#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "../hdr/gl.h"

void glnode_init( glnode_t* node )
{
    assert(node);
    node->prev = node->next = NULL;
}

/// @brief Gl
/// @param glt 
/// @param _offset 
void glthread_init( glthread_t* glt, uint32_t _offset )
{
    assert(glt);
    glt->node_count = 0;
    glt->base_addr = _offset;
    glt->head = NULL;
}


/// @brief Add the
/// @param glt 
/// @param gln 
void glthread_add_node( glthread_t* glt, glnode_t *gln )
{
    assert(glt);
    assert(gln);

    if( !glt->head )
    {
        glt->head = gln;
    }
    else
    {
        gln->next = glt->head;
        glt->head->prev = gln;
        glt->head = gln;
    }
    glt->node_count++;
}