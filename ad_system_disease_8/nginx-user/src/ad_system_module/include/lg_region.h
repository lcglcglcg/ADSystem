
#ifndef	__LG_REGION_H__
#define	__LG_REGION_H__

#ifdef __cplusplus
extern "C" { 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lg_rbtree.h"

    typedef struct {

	lg_rbtree_node tree;
	unsigned int code;
	char father[128];
	char child[128];

    }lg_region_t;

    extern int lg_region_load(lg_rbtree_head *head, const char *path);
    extern lg_region_t *lg_region_find(lg_rbtree_head *head, int code);

#ifdef __cplusplus
}
#endif

#endif

