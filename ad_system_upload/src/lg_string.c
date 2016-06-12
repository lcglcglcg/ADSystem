
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>
#include <uuid/uuid.h>
#include <openssl/md5.h>

char *strptime(const char *s, const char *format, struct tm *tm);

int lg_int_cmp(int dest, int src)
{

    if (dest < src)
	return -1;

    else if (dest > src)
	return 1;

    return 0;
}

int lg_double_cmp(double dest, double src)
{

    if (dest < src)
	return -1;

    else if (dest > src)
	return 1;

    return 0;
}

int lg_string_url_encode(char *dest, char *src, size_t size) 
{

    int n;
    int dest_size = 0;
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

int lg_string_url_encode_r(char *buffer, size_t size) 
{

    char temp[size];
    memset(temp, 0, size);
    int temp_size = lg_string_url_encode(temp, buffer, size);
    memcpy(buffer, temp, temp_size);
    buffer[temp_size] = 0;
    return temp_size;
}

int lg_string_url_decode(char *dest, char *src, size_t size) 
{


    char h, l;
    int vh, vl, n;
    int dest_size = 0;

    if (src == NULL || size < 0) return -1;

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

int lg_string_url_decode_r(char *buffer, size_t size) 
{

    char temp[size];
    memset(temp, 0, size);
    int temp_size = lg_string_url_decode(temp, buffer, size);
    memcpy(buffer, temp, temp_size);
    buffer[temp_size] = 0;
    return temp_size;
}

size_t lg_string_utf8_to_gbk(char *in_buff, size_t in_size, char *out_buff, size_t out_size)
{

    char *in = in_buff;
    char *out = out_buff;
    iconv_t cd = iconv_open("GBK", "UTF-8");
    iconv(cd, &in, &in_size, &out, &out_size);
    iconv_close(cd);
    return strnlen(out_buff, out_size);
}

size_t lg_string_gbk_to_utf8(char *in_buff, size_t in_size, char *out_buff, size_t out_size)
{

    char *in = in_buff;
    char *out = out_buff;
    iconv_t cd = iconv_open("UTF-8", "GBK");
    iconv(cd, &in, &in_size, &out, &out_size);
    iconv_close(cd);
    return strnlen(out_buff, out_size);
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

#if 0
    int i;
    MD5_CTX ctx = {0};
    char temp[3] = {0};
    unsigned char md[16] = {0};

    MD5_Init(&ctx);
    MD5_Update(&ctx, str, strlen(str));//更新这块区域防止引用非法数据
//    MD5_Final(md, &ctx); //ctx中的数据按照MD5算法生成16位的MD5码存放到md中

    for (i = 0; i < 16; i++) {

	sprintf(temp, "%02x", md[i]);
	strcat(md5_str, temp);
    }
#endif

    return md5_str;
}


/* time_t api */
time_t lg_string_get_this_year_time()
{

    struct tm tm = {0};
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

    struct tm tm = {0};
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

    struct tm tm = {0};
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

    struct tm tm = {0};
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

    struct tm tm = {0};
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

    struct tm tm = {0};
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    sprintf(buffer, "%04d", tm.tm_year + 1900);
    return buffer;
}

char *lg_string_get_last_year_string(char *buffer)
{

    struct tm tm = {0};
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    sprintf(buffer, "%04d", tm.tm_year + 1899);
    return buffer;
}

char *lg_string_get_this_month_string(char *buffer)
{

    struct tm tm = {0};
    time_t t = time(NULL);
    gmtime_r(&t, &tm);
    sprintf(buffer, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);
    return buffer;
}

char *lg_string_get_last_month_string(char *buffer)
{

    struct tm tm = {0};
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

