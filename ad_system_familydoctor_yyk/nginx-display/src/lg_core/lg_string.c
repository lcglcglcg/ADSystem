
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <uuid/uuid.h>
#include "lg_string.h"

char *lg_string_strtok(const char *src, char *dest, char **save_ptr, int ch)
{

    *dest = 0;
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

char *lg_string_strstr(const char *str, const char *needle)
{

    size_t len = strlen(needle);
    if(len == 0)
	return NULL;

    while(*str) {

	if(strncasecmp(str, needle, len) == 0)
	    return (char *)str;

	++str;
    }

    return NULL;
}

char *lg_string_strspn(char *buffer, char ch)
{

    char *str = NULL;
    for (str = buffer; *str; str++)
	if (*str == ch)
	    return str;

    return NULL;
}

char *lg_string_cutting(char *buffer, char ch)
{
    if (!buffer) return NULL;

    char *str = lg_string_strspn(buffer, ch);
    if (!str) return NULL;

    *str = 0;
    return (str + 1);
}

int lg_string_cmp(const char *s1, int s1_len, const char *s2, int s2_len)
{

    int len = s1_len <= s2_len ? s1_len : s2_len;
    int ret = strncmp(s1, s2, len);
    if (ret == 0) {

	if (s1_len == s2_len)
	    return 0;

	if (s1_len < s2_len)
	    return -1;
	else
	    return 1;
    }

    return ret;
}

size_t lg_string_url_encode(char *dest, char *src, size_t size) 
{

    size_t n;
    size_t dest_size = 0;
    char hextable[] = "0123456789ABCDEF";
    unsigned char *str = (unsigned char *)src;

    if (src == NULL) return 0;

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

size_t lg_string_url_decode(char *dest, char *src, size_t size)
{

    char h, l;
    size_t vh, vl, n;
    size_t dest_size = 0;
    if (src == NULL) return 0;

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

char *lg_string_uuid(char *key)
{

    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, key);
    return key;
}



