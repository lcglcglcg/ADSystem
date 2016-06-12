
#ifndef	__AD_SYSTEM_REGION_CODE_H__
#define	__AD_SYSTEM_REGION_CODE_H__

#include "lg_rbtree.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	lg_rbtree_node tree;
	unsigned int code;
	char father[128];
	char child[128];

    } ad_system_region_code_t;

    extern int ad_system_region_code_load(const char *path);
    extern ad_system_region_code_t *ad_system_region_code_find(unsigned int code);

#ifdef __cplusplus
}
#endif

#endif


