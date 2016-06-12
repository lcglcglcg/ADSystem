
#ifndef __LG_JSON_H__
#define	__LG_JSON_H__

#include <stdio.h>
#define lg_json_t void

#ifdef __cplusplus
extern "C" { 
#endif

    extern lg_json_t *lg_json_parse(char *buffer);
    extern void lg_json_free(lg_json_t *json_obj);
    
    extern lg_json_t *lg_json_get_object(lg_json_t *json_obj, const char *str);

    extern int lg_json_value_to_number(lg_json_t *json_obj);
    extern double lg_json_value_to_double(lg_json_t *json_obj);
    extern char *lg_json_value_to_string(lg_json_t *json_obj);

    extern lg_json_t *lg_json_get_array(lg_json_t *json_obj, int idx);
    extern int json_object_get_array_length(lg_json_t *json_obj);

    extern size_t lg_json_sprintf(char *buffer, char *head_str, char *end_str, int count, ...);

#ifdef __cplusplus
}
#endif
#endif//__LG_JSON_H__


