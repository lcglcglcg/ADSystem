
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lg_list.h"
#include "lg_string.h"

typedef struct {

    lg_list_t next;
    char code[128];

} lg_region_node_t;

typedef struct {

    lg_list_t next;
    lg_list_t head;
    char code[128];

} lg_region_t;

lg_list_t lg_region_code_head = {&lg_region_code_head, &lg_region_code_head};

int lg_region_code_node_add(lg_list_t *head, char *code)
{

    lg_region_node_t *node = malloc(sizeof(lg_region_node_t));
    if (!node) return -1;

    lg_list_add(head, &node->next);
    strcpy(node->code, code);
    return 0;
}

int lg_region_code_parse(char *father_code, char *code)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, &lg_region_code_head) {

	lg_region_t *node = (lg_region_t *)p;
	if (strcmp(father_code, node->code) == 0) {

	    if (strcmp(father_code, code) == 0)
		return 0;

	    // fprintf(stdout, "---->[2][%s][%s]\n", father_code, code);
	    return lg_region_code_node_add(&node->head, code);
	}
    }

    lg_region_t *node = malloc(sizeof(lg_region_t));
    if (!node) return -1;

    lg_list_add(&lg_region_code_head, &node->next);
    lg_list_init(&node->head);
    strcpy(node->code, father_code);

    return lg_region_code_parse(father_code, code);
}

int lg_region_code_load(const char *path)
{

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char buffer[4096];
    while(fgets(buffer, 4096, fp)) {

	char *saveptr = NULL;
	char *code = strtok_r(buffer, ",", &saveptr);
	char *father_code = strtok_r(NULL, ",", &saveptr);
	// char *name = strtok_r(NULL, ",", &saveptr);
	// char *f_name = strtok_r(NULL, ",", &saveptr);
	if (lg_region_code_parse(father_code, code))
	    return -1;
    }

    fclose(fp);
    return 0;
}

int lg_region_father_code_add_check(lg_region_t *node_father, const char *codes)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, &node_father->head) {

	lg_region_node_t *node = (lg_region_node_t *)p;
	if (!strstr(codes, node->code))
	    return -1;
    }

    if (strcmp(node_father->code, "1000500000") == 0
	    || strcmp(node_father->code, "1000600000") == 0
	    || strcmp(node_father->code, "1000700000") == 0
	    || strcmp(node_father->code, "1009900000") == 0
       )
	return -1;

    return 0;
}

int lg_region_father_code_add(const char *codes, char *buffer)
{

    if (!codes ) return -1;
    int size = sprintf(buffer, codes);

    lg_list_t *p_father  = NULL;
    lg_list_for_each(p_father, &lg_region_code_head) {

	lg_region_t *node_father = (lg_region_t *)p_father;
	if (lg_region_father_code_add_check(node_father, codes) == 0) {

	    size += sprintf(&buffer[size], ",%s", node_father->code);
	}
    }

    return 0;
}

int lg_region_father_code_del(const char *codes, char *buffer)
{

    if (!codes ) return -1;
    strcpy(buffer, codes);

    lg_list_t *p_father  = NULL;
    lg_list_for_each(p_father, &lg_region_code_head) {

	lg_region_t *node_father = (lg_region_t *)p_father;
	lg_string_remove_keyword(buffer, node_father->code, 10);
    }

    return 0;
}


