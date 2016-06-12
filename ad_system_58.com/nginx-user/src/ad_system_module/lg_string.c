
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>
#include <uuid/uuid.h>
#include <openssl/md5.h>

char *strptime(const char *s, const char *format, struct tm *tm);

int lg_string_url_encode(char *dest, char *src, size_t size) 
{

    size_t n;
    size_t dest_size = 0;
    char hextable[] = "0123456789ABCDEF";
    unsigned char *str = (unsigned char *)src;

    if (src == NULL) return -1;

    for (n = 0; n < size; n++) {

	if (!isalnum(str[n])) {

	    *dest++ = '%';
	    *dest++ = hextable[str[n] >> 4];
	    *dest++ = hextable[str[n] & 0x0F];
	    dest_size += 3;

	} else {

	    *dest++ = str[n];
	    dest_size++;
	}
    }

    *dest = 0;
    return dest_size;
}

int lg_string_url_decode(char *dest, char *src, size_t size)
{


    char h, l;
    size_t vh, vl, n;
    size_t dest_size = 0;
    if (src == NULL) return -1;

    for (n = 0; n < size; n++) {

	if (src[n] == '%') {

	    h = toupper(src[n + 1]);
	    l = toupper(src[n + 2]);
	    vh = isalpha(h) ? (10 + (h - 'A')) : (h - '0');
	    vl = isalpha(l) ? (10 + (l - 'A')) : (l - '0');

	    *dest++ = ((vh << 4) + vl);
	    dest_size++;
	    n++;
	    n++;

	} else if (src[n] == '+') {

	    *dest++ = ' ';
	    dest_size++;

	} else {

	    *dest++ = src[n];
	    dest_size++;
	}
    }

    *dest = 0;
    return dest_size;
}

int lg_string_iconv(char *in_format, char *in_buff, size_t in_size, char *out_format, char *out_buff, size_t out_size)
{

    char **in = &in_buff;
    char **out = &out_buff;

    iconv_t cd = 0;
    if (!(cd = iconv_open(out_format, in_format)))
	return -1;

    if ((iconv(cd, in, &in_size, out, &out_size)) == (size_t)-1) {

	iconv_close(cd);
	return -1;
    }

    iconv_close(cd);
    return 0;
}

int lg_string_utf8_to_gbk(char *in_buff, size_t in_size, char *out_buff, size_t out_size)
{
    return lg_string_iconv("UTF-8", in_buff, in_size, "GBK", out_buff, out_size);
}

size_t lg_string_gbk_to_utf8(char *in_buff, size_t in_size, char *out_buff, size_t out_size)
{
    return lg_string_iconv("GBK", in_buff, in_size, "UTF-8", out_buff, out_size);
}

int lg_string_utf8_to_unicode(char *in_buff, size_t in_size, char *out_buff, size_t out_size)
{
    return lg_string_iconv("UTF-8", in_buff, in_size, "UTF-16", out_buff, out_size);
}

size_t lg_string_unicode_len(char *buffer)
{

    size_t i;
    for (i = 0; ; i++, i++) {

	if (!buffer[i] && !buffer[i + 1])
	    return i;
    }

    return 0;
}

char *lg_string_uuid(char *key)
{

    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, key);
    return key;
}

char *lg_string_md5(const char *str, char *md5_str)
{

    int i;
    MD5_CTX ctx;
    char temp[3] = {0};
    unsigned char md[16] = {0};

    MD5_Init(&ctx);
    MD5_Update(&ctx, str, strlen(str));//更新这块区域防止引用非法数据
    MD5_Final(md, &ctx); //ctx中的数据按照MD5算法生成16位的MD5码存放到md中

    for (i = 0; i < 16; i++) {

	sprintf(temp, "%02x", md[i]);
	strcat(md5_str, temp);
    }

    return md5_str;
}

char *lg_string_strtok(const char *src, char *dest, char **save_ptr, int ch)
{

    if (save_ptr && *save_ptr) 
	src = *save_ptr;

    char *p = dest;
    char *s = (char *)src;
    for (; s && *s; s++) {

	if (*s == ch) {

	    *p = 0;
	    *save_ptr = s + 1;
	    return dest;
	}

	*p++ = *s;
    }

    if (src || *save_ptr) {

	*p = 0;
	*save_ptr = NULL;
	return dest;
    }

    return NULL;
}

char *lg_string_filter(char *dest, char *src, size_t size, int ch)
{
    size_t i;
    for (i = 0; i < size; i++) {
	if (src[i] == ch) {

	    dest[i] = 0;
	    return dest;
	}

	dest[i] = src[i];
    }

    return NULL;
}

int lg_string_file_to_buffer(const char *path, char **buffer, size_t *size)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (*size == 0) {

	fclose(fp);
	return -1;
    }   

    if (!(*buffer = malloc(*size + 1))) {

	fclose(fp);
	return -1;
    }   

    if (fread(*buffer, 1, *size, fp) != *size) {

	fclose(fp);
	free(*buffer);
	return -1;
    }   

    fclose(fp);
    return 0;
}

/* time_t api */
time_t lg_string_get_this_year_time()
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    char buffer[128] = {0};
    sprintf(buffer, "%04d-01-01", tm.tm_year + 1900);

    memset(&tm, 0, sizeof(struct tm)); 
    strptime(buffer, "%Y-%m-%d", &tm);

    return mktime(&tm) + 28800;
}

time_t lg_string_get_last_year_time()
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    char buffer[128] = {0};
    sprintf(buffer, "%04d-01-01", tm.tm_year + 1899);

    memset(&tm, 0, sizeof(struct tm)); 
    strptime(buffer, "%Y-%m-%d", &tm);

    return mktime(&tm) + 28800;
}

time_t lg_string_get_this_day_time()
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    char buffer[128] = {0};
    sprintf(buffer, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    memset(&tm, 0, sizeof(struct tm)); 
    strptime(buffer, "%Y-%m-%d", &tm);

    return mktime(&tm) + 28800;
}

time_t lg_string_get_last_day_time()
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    char buffer[128] = {0};
    sprintf(buffer, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    memset(&tm, 0, sizeof(struct tm)); 
    strptime(buffer, "%Y-%m-%d", &tm);

    return mktime(&tm) + 28800 - 86400;
}

char *lg_string_get_time_string(char *buffer)
{
    time_t t = time(NULL);

    struct tm tm;
    localtime_r(&t, &tm);

    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
	    tm.tm_year + 1900,
	    tm.tm_mon + 1,
	    tm.tm_mday,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec);

    return buffer;
}

char *lg_string_get_this_year_string(char *buffer)
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    sprintf(buffer, "%04d", tm.tm_year + 1900);
    return buffer;
}

char *lg_string_get_last_year_string(char *buffer)
{

    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    sprintf(buffer, "%04d", tm.tm_year + 1899);
    return buffer;
}

char *lg_string_get_this_month_string(char *buffer)
{

    struct tm tm;
    time_t t = time(NULL);
    gmtime_r(&t, &tm);
    sprintf(buffer, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);
    return buffer;
}

char *lg_string_get_last_month_string(char *buffer)
{

    struct tm tm;
    time_t t = time(NULL);
    gmtime_r(&t, &tm);

    if (tm.tm_mon) {

	tm.tm_year += 1900;

    } else {

	tm.tm_year += 1899;
	tm.tm_mon = 12;
    }

    sprintf(buffer, "%04d-%02d", tm.tm_year, tm.tm_mon);
    return buffer;
}



