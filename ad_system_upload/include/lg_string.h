
#ifndef __LG_STRING_H__
#define	__LG_STRING_H__

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" { 
#endif

    extern int lg_int_cmp(int dest, int src);
    extern int lg_double_cmp(double dest, double src);

    extern int lg_string_url_encode(char *dest, char *src, size_t size);
    extern int lg_string_url_decode(char *dest, char *src, size_t size);
    extern int lg_string_url_encode_r(char *buffer, size_t size);
    extern int lg_string_url_decode_r(char *buffer, size_t size);
    extern size_t lg_string_utf8_to_gbk(char *in_buff, size_t in_size, char *out_buff, size_t out_size);
    extern size_t lg_string_gbk_to_utf8(char *in_buff, size_t in_size, char *out_buff, size_t out_size);
    extern char *lg_string_uuid(char *key);
    extern char *lg_string_md5(const char *str, char *md5_str);

    extern time_t lg_string_get_this_year_time();
    extern time_t lg_string_get_last_year_time();
    extern time_t lg_string_get_this_day_time();
    extern time_t lg_string_get_last_day_time();

    extern char *lg_string_get_time_string(char *buffer);
    extern char *lg_string_get_this_year_string(char *buffer);
    extern char *lg_string_get_last_year_string(char *buffer);
    extern char *lg_string_get_this_month_string(char *buffer);
    extern char *lg_string_get_last_month_string(char *buffer);


#ifdef __cplusplus
}
#endif
#endif//__LG_STRING_H__

