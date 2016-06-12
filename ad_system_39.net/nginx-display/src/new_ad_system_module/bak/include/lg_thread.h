
#ifndef	__LG_THREAD_POOL_H__
#define	__LG_THREAD_POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "lg_list.h"
#include "lg_queue.h"

#define	THREAD_POLL_QUEUE_MAX 40960

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct lg_thread_node_t lg_thread_node_t;
    typedef struct lg_thread_pool_t lg_thread_pool_t;
    typedef void *(lg_thread_function)(lg_thread_node_t *self);
    typedef void *(lg_thread_function_)(void *arg);

    struct lg_thread_node_t {

	lg_list_t node;

	pthread_t tid;
	pthread_attr_t *attr;
	lg_thread_function *function;
	void *arg;

	lg_thread_pool_t *pool;
	pthread_cond_t *cond;
	lg_queue_t *queue;
	pthread_mutex_t *mutex;
	sem_t *sem;
    };

    struct lg_thread_pool_t {

	lg_list_t head;
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
	lg_queue_t *queue;
	sem_t *sem;
    };

    extern lg_thread_pool_t *lg_thread_pool_init(
	    lg_thread_pool_t *pool,
	    lg_queue_t *queue,
	    pthread_mutex_t *mutex,
	    sem_t *sem);
    extern lg_thread_node_t *lg_thread_node_add(
	    lg_thread_pool_t *pool,
	    lg_thread_function *function,
	    void *arg);
    extern int lg_thread_pool_add(int max_pool,
	    lg_thread_pool_t *pool,
	    lg_thread_function *function,
	    void *arg);

    extern void lg_thread_pool_start(lg_thread_pool_t *pool);
    extern void lg_thread_pool_stop(lg_thread_pool_t *pool);
    extern void lg_thread_pool_join(lg_thread_pool_t *pool);
    extern void lg_thread_pool_free(lg_thread_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif// __LG_THREAD_POOL_H__

