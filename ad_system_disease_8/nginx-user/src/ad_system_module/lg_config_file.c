
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lg_config_file.h"

void lg_config_file_add(lg_list_t *head, char *name_str, size_t name_size, char *value_str, size_t value_size)
{

    char *p = NULL;
    if (!(p = malloc(sizeof(lg_config_file_t) + name_size + value_size)))
	return;

    lg_config_file_t *node = (lg_config_file_t *)p;
    node->name = p + sizeof(lg_config_file_t);
    node->value = node->name + name_size;

    memcpy(node->name, name_str, name_size);
    memcpy(node->value, value_str, value_size);
    lg_list_add(head, &node->list);
}

char *lg_config_file_find(lg_list_t *head, const char *name)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, head) {

	lg_config_file_t *node = NULL;
	node = (lg_config_file_t *)p;

	if (strcmp(node->name, name) == 0)
	    return node->value;
    }

    return NULL;
}

void lg_config_file_parse(lg_list_t *head, char *buffer)
{

    if (!isalnum(*buffer))
	return;

    char *str = NULL;
    if (!(str = strstr(buffer, "=")))
	return;

    *str++ = 0;
    char *name_str = buffer;
    char *value_str = str;

    for (str = value_str; *str; str++) {

	if (isspace(*str)) {

	    *str = 0;
	    break;
	}
    }

    size_t name_size = 0;
    if (!(name_size = strlen(name_str)))
	return;

    size_t value_size = 0;
    if (!(value_size = strlen(value_str)))
	return;

    lg_config_file_add(head, name_str, name_size + 1, value_str, value_size + 1);
}

int lg_config_file_load(lg_list_t *head, const char *path)
{

    lg_list_init(head);

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    char buffer[4096] = {0};
    while(fgets(buffer, 4096, fp)) {

	if (buffer[4000])
	    return -1;

	lg_config_file_parse(head, buffer);
	memset(buffer, 0, 4096);
    }

    fclose(fp);
    return 0;
}


