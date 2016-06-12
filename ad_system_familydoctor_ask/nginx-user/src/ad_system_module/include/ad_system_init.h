
#ifndef	__AD_SYSTEM_GLOBAL_VAR_H__
#define	__AD_SYSTEM_GLOBAL_VAR_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" { 
#endif

    struct ad_system_global_region_code_t {

	char *buffer;
	size_t size;

    } ad_system_global_region_code;

    extern int ad_system_init();

#ifdef __cplusplus
}
#endif

#endif




