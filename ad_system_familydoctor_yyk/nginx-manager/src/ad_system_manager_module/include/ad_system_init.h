
#ifndef	__AD_SYSTEM_INIT_H__
#define	__AD_SYSTEM_INIT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" { 
#endif

    extern int ad_system_init();
    extern char *get_ad_system_region_code_buffer();
    extern size_t get_ad_system_region_code_size();
    extern char *get_delivery_center_domain();

#ifdef __cplusplus
}
#endif

#endif




