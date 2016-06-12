
#ifndef __LG_CONFIG_FILE_H__
#define	__LG_CONFIG_FILE_H__

#include "lg_list.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	lg_list_t list;
	char *name;
	char *value;

    } lg_config_file_t;

    extern int lg_config_file_load(lg_list_t *head, const char *path);
    extern char *lg_config_file_find(lg_list_t *head, const char *name);

#ifdef __cplusplus
}
#endif

#endif


