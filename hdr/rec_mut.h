#ifndef __REC_MUT_H__
#define __REC_MUT_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct rec_mut
{
    pthread_t indexed;
    int lock_depth;
    pthread_mutex_t mut;
    pthread_cond_t cv;
}rec_mut_t;

void rec_mut_init( rec_mut_t* r_mut );
void rec_mut_lock( rec_mut_t* r_mut );
void rec_mut_unlock( rec_mut_t* r_mut );
void rec_mut_destroy( rec_mut_t* r_mut );

#endif /* __REC_MUT_H__ */
