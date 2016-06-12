
#ifndef	__AD_SYSTEM_DOMAIN_PARSE__
#define	__AD_SYSTEM_DOMAIN_PARSE__

#include "lg_list.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	lg_list_t list;
	char domain_str[256];
	char key[128];

    } ad_system_domain_memory_t;

    extern char *ad_system_domain_parse(const char *buffer, char *domain);
    extern char *ad_system_domain_memory_add(lg_list_t *head, const char *key, const char *domain_str);
    extern char *ad_system_domain_memory_find(lg_list_t *head, char *key, char *domain_str);
    extern void ad_system_domain_memory_free(lg_list_t *head);
    extern void ad_system_domain_memory_print(lg_list_t *head);

#ifdef __cplusplus
}
#endif

#endif


