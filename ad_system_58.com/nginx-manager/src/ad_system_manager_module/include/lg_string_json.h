
#ifndef __LG_STRING_JSON_H__
#define	__LG_STRING_JSON_H__

#ifdef __cplusplus
extern "C" { 
#endif

#include "json.h"
#define	GET_JSON_VALUE_STR(o,s) json_object*o=json_object_object_get(json_root, s);if(o)json->o=json_object_get_string(o)
#define	GET_JSON_VALUE_STR2(o,s) json_object*o=json_object_object_get(json_root, s);if(o)lg_string_json_value_to_string(o, json->o)
#define	GET_JSON_VALUE_NUM(o,s) json_object*o=json_object_object_get(json_root, s);if(o)json->o=lg_string_json_value_to_number(o)

    extern int lg_string_to_json(struct printbuf *pb, const char *str);
    extern char *lg_string_to_json_r(char *dest, const char *str);
    extern int lg_string_json_value_to_number(json_object *json_value);
    extern char *lg_string_json_value_to_string(json_object *json_value, char *buffer);
    extern size_t lg_string_json_sprintf(char *buffer, char *head_str, char *end_str, int count, ...);

#ifdef __cplusplus
}
#endif
#endif//__LG_STRING_JSON_H__


