
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "json.h"
#include "lg_string.h"

int lg_string_to_json(struct printbuf *pb, const char *str)
{
    int pos = 0, start_offset = 0;
    unsigned char c;
    do {
	c = str[pos];
	switch(c) {
	    case '\0':
		break;
	    case '\b':
	    case '\n':
	    case '\r':
	    case '\t':
	    case '"':
	    case '\\':
	    case '/':
		if(pos - start_offset > 0)
		    printbuf_memappend(pb, str + start_offset, pos - start_offset);
		if(c == '\b') printbuf_memappend(pb, "\\b", 2);
		else if(c == '\n') printbuf_memappend(pb, "\\n", 2);
		else if(c == '\r') printbuf_memappend(pb, "\\r", 2);
		else if(c == '\t') printbuf_memappend(pb, "\\t", 2);
		else if(c == '"') printbuf_memappend(pb, "\\\"", 2);
		else if(c == '\\') printbuf_memappend(pb, "\\\\", 2);
		else if(c == '/') printbuf_memappend(pb, "\\/", 2);
		start_offset = ++pos;
		break;
	    default:
		if(c < ' ') {
		    if(pos - start_offset > 0)
			printbuf_memappend(pb, str + start_offset, pos - start_offset);
		    sprintbuf(pb, "\\u00%c%c",
			    json_hex_chars[c >> 4],
			    json_hex_chars[c & 0xf]);
		    start_offset = ++pos;
		} else pos++;
	}
    } while(c);

    if(pos - start_offset > 0)
	printbuf_memappend(pb, str + start_offset, pos - start_offset);
    return 0;
}

char *lg_string_to_json_r(char *dest, const char *str)
{

    if (str == NULL) return NULL;

    struct printbuf *json_str = NULL;
    if ((json_str = printbuf_new())) {

	lg_string_to_json(json_str, str);
	strcpy(dest, json_str->buf);
	printbuf_free(json_str);
	return dest;
    }

    return NULL;
}

int lg_string_json_value_to_number(json_object *json_value)
{

    const char *str = NULL;
    if ((str = json_object_get_string(json_value)))
	return atoi(str);

    return 0;
}

char *lg_string_json_value_to_string(json_object *json_value, char *buffer)
{

    const char *str = NULL;
    if (!(str = json_object_get_string(json_value)))
	return NULL;

    char *p = buffer;
    for (; *str; str++) {

	if (*str == '%')
	    *buffer++ = '\\';

	if (*str == '\'')
	    *buffer++ = '\'';

	*buffer++ = *str;
    }

    *buffer = 0;
    return p;
}

size_t lg_string_json_sprintf(char *buffer, char *head_str, char *end_str, int count, ...)
{

    int i, size = 0;
    char comma[4] = {0};

    va_list arg_list;
    va_start(arg_list, count);
    if (head_str) size += sprintf(&buffer[size], head_str);

    for(i = 0; i < count; i++) {

	char *format = va_arg(arg_list, char *);
	char *name = va_arg(arg_list, char *);

	if (strcmp(format, "%d") == 0 || strcmp(format, "%u") == 0) {

	    int value = va_arg(arg_list, int);
	    size += sprintf(&buffer[size], "%s\"%s\":\"%u\"", comma, name, value);
	    comma[0] = ',';

	} else if (strcmp(format, "%f") == 0) {

	    double value = va_arg(arg_list, double);
	    size += sprintf(&buffer[size], "%s\"%s\":\"%0.2f\"", comma, name, value);
	    comma[0] = ',';
	
	} else if (strcmp(format, "%e") == 0) {

	    double value = va_arg(arg_list, double);
	    size += sprintf(&buffer[size], "%s\"%s\":\"%0.2f%%\"", comma, name, value * 100);
	    comma[0] = ',';

	} else if (strcmp(format, "%s") == 0) {

	    char *value = va_arg(arg_list, char *);
	    if (value) {

		if (*value) {

		    struct printbuf *json_value = NULL;
		    if ((json_value = printbuf_new())) {

			lg_string_to_json(json_value, value);
			size += sprintf(&buffer[size], "%s\"%s\":\"%s\"", comma, name, json_value->buf);
			printbuf_free(json_value);
		    }

		} else {

		    size += sprintf(&buffer[size], "%s\"%s\":\"\"", comma, name);
		}

	    } else {

		size += sprintf(&buffer[size], "%s\"%s\":\"\"", comma, name);
	    }

	    comma[0] = ',';

	} else {

	    va_end(arg_list);
	    return 0;
	}
    }

    va_end(arg_list);
    if (end_str)size += sprintf(&buffer[size], end_str);

    return size;
}

void lg_string_json_to_sql_array(const char *str, char *array)
{

    size_t size = 0;
    char temp[1024];
    char comma[2] = {0};
    char *save_ptr = NULL;
    char *p = lg_string_strtok(str, temp, &save_ptr, ',');
    while(p){

	size += sprintf(&array[size], "%s'%s'", comma, p);
	p = lg_string_strtok(NULL, temp, &save_ptr, ',');
	comma[0] = ',';
    }
}


