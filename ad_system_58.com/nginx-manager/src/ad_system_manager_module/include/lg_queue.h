#ifndef __LG_QUEUE_H__
#define __LG_QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	char *buffer;
	unsigned int buffer_size;
	unsigned int start_pos;
	unsigned int end_pos;

    }lg_queue_t;

    extern lg_queue_t *lg_queue_init(lg_queue_t *queue, unsigned int size);
    extern void lg_queue_free(lg_queue_t  *queue);

    extern int lg_queue_enqueue(lg_queue_t *queue, char *buffer, unsigned int size);
    extern int lg_queue_dequeue(lg_queue_t *queue, char *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif //__LG_QUEUE_H__
