
#ifndef __LG_STRING_H__
#define	__LG_STRING_H__

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" { 
#endif

    extern char *lg_string_uuid(char *key);
    extern char *lg_string_md5(const char *str, char *md5_str);
    extern char *lg_string_strtok(const char *src, char *dest, char **save_ptr, int ch);
    extern char *lg_string_filter(char *dest, char *src, size_t size, int ch);
    extern int lg_string_file_to_buffer(const char *path, char **buffer, size_t *size);

    extern char *strptime(const char *s, const char *format, struct tm *tm);

    extern int lg_string_url_encode(char *dest, char *src, size_t size);
    extern int lg_string_url_decode(char *dest, char *src, size_t size);
    extern int lg_string_url_encode_r(char *buffer, size_t size);
    extern int lg_string_url_decode_r(char *buffer, size_t size);

    extern int lg_string_iconv(char *in_format, char *in_buff, size_t in_size, char *out_format, char *out_buff, size_t out_size);
    extern int lg_string_utf8_to_gbk(char *in_buff, size_t in_size, char *out_buff, size_t out_size);
    extern int lg_string_gbk_to_utf8(char *in_buff, size_t in_size, char *out_buff, size_t out_size);
    extern int lg_string_utf8_to_unicode(char *in_buff, size_t in_size, char *out_buff, size_t out_size);
    extern size_t lg_string_unicode_len(char *buffer);

    extern char *lg_string_get_time_string(char *buffer);
    extern char *lg_string_get_this_year_string(char *buffer);
    extern char *lg_string_get_last_year_string(char *buffer);
    extern char *lg_string_get_this_month_string(char *buffer);
    extern char *lg_string_get_last_month_string(char *buffer);

#ifdef __cplusplus
}
#endif
#endif//__LG_STRING_H__

