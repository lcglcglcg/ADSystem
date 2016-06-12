
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lg_list.h"
#include "ad_system_sql.h"

typedef struct {

    lg_list_t next;
    lg_list_t head;
    char code[128];

} ad_system_disease_code_t;

void ad_system_disease_code_add(lg_list_t *head, const char *code, int level)
{

    if (level == 2) {

	if (code[4] == '0' && code[5] == '0' && code[6] == '0')
	    return;

    } else if (level == 3) {

	if (code[7] == '0' && code[8] == '0' && code[9] == '0')
	    return;
    }

    lg_list_t *p = NULL;
    ad_system_disease_code_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_disease_code_t *)p;

	if (level == 1) {

	    if (strncmp(code, node->code, 4) == 0)
		return ad_system_disease_code_add(&node->head, code, 2);

	} else if (level == 2) {

	    if (strncmp(&code[4], &node->code[4], 3) == 0)
		return ad_system_disease_code_add(&node->head, code, 3);

	} else if (level == 3) {

	    if (strncmp(&code[7], &node->code[7], 3) == 0)
		return;
	}
    }

    if (!(node = malloc(sizeof(ad_system_disease_code_t))))
	return;

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);

    if (level == 1) {

	sprintf(node->code, "%c%c%c%c000000", code[0], code[1], code[2], code[3]);
	return ad_system_disease_code_add(&node->head, code, 2);

    } else if (level == 2) {

	sprintf(node->code, "%c%c%c%c%c%c%c000", code[0], code[1], code[2], code[3], code[4], code[5], code[6]);
	return ad_system_disease_code_add(&node->head, code, 3);

    } else if (level == 3) {

	strcpy(node->code, code);
    }
}

ad_system_disease_code_t *ad_system_disease_code_find(lg_list_t *head, char *code)
{

    lg_list_t *p = NULL;
    ad_system_disease_code_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_disease_code_t *)p;
	if (strcmp(node->code, code) == 0)
	    return node;

	if ((node = ad_system_disease_code_find(&node->head, code)))
	    return node;
    }

    return NULL;
}

void ad_system_disease_code_temp_list_add(lg_list_t *head, char *code)
{

    lg_list_t *p = NULL;
    ad_system_disease_code_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_disease_code_t *)p;
	if (strcmp(node->code, code) == 0)
	    return;
    }

    if (!(node = malloc(sizeof(ad_system_disease_code_t))))
	return;

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);
    strcpy(node->code, code);
}

void ad_system_disease_code_recursion(lg_list_t *head, lg_list_t *list_head)
{

    lg_list_t *p = NULL;
    ad_system_disease_code_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_disease_code_t *)p;
	ad_system_disease_code_temp_list_add(list_head, node->code);

	ad_system_disease_code_recursion(&node->head, list_head);
    }
}

void ad_system_disease_code_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_disease_code_t *node = NULL;

    while(p != head) {

	node = (ad_system_disease_code_t *)p;
	ad_system_disease_code_free(&node->head);

	p = p->next;
	free(node);
    }
}

int ad_system_disease_code_load(lg_list_t *disease_code_head)
{

    lg_list_init(disease_code_head);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_disease_code(&sql_conn))
	return -1;

    while(ad_system_sql_fetch_next(&sql_conn))
	ad_system_disease_code_add(disease_code_head,
		ad_system_sql_get_string(&sql_conn, NULL, 2), 
		1);

    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_disease_code_perse(char *out_code_buff, const char *in_code_buff)
{
    if (!in_code_buff) return -1;

    lg_list_t temp_list_head;
    lg_list_init(&temp_list_head);
    strcpy(out_code_buff, in_code_buff);
    char *buffer = out_code_buff;

    lg_list_t disease_code_head;
    if (ad_system_disease_code_load(&disease_code_head))
	return -1;

    char *code = NULL;
    char *saveptr = NULL;
    while((code = strtok_r(buffer, ",", &saveptr))) {

	ad_system_disease_code_t *node = NULL;
	if ((node = ad_system_disease_code_find(&disease_code_head, code))) {

	    ad_system_disease_code_temp_list_add(&temp_list_head, node->code);
	    ad_system_disease_code_recursion(&node->head, &temp_list_head);
	}

	buffer = NULL;
    }

    size_t size = 0;
    char comma[4] = "";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &temp_list_head) {

	ad_system_disease_code_t *node = NULL;
	node = (ad_system_disease_code_t *)p;
	fprintf(stdout, "1-->%s\n", node->code);
	fprintf(stderr, "1-->%s\n", node->code);
	size += sprintf(&out_code_buff[size], "%s%s", comma, node->code);
	comma[0] = ',';
    }

    ad_system_disease_code_free(&disease_code_head);
    ad_system_disease_code_free(&temp_list_head);
    return 0;
}

int ad_system_disease_code_perse_r(char *out_code_buff, const char *in_code_buff)
{
    if (!in_code_buff) return -1;

    lg_list_t temp_list_head;
    lg_list_init(&temp_list_head);
    strcpy(out_code_buff, in_code_buff);


    char *buffer = out_code_buff;

    lg_list_t disease_code_head;
    if (ad_system_disease_code_load(&disease_code_head))
	return -1;

    char *code = NULL;
    char *saveptr = NULL;
    while((code = strtok_r(buffer, ",", &saveptr))) {

	ad_system_disease_code_t *node = NULL;
	if ((node = ad_system_disease_code_find(&disease_code_head, code))) {

	    ad_system_disease_code_temp_list_add(&temp_list_head, node->code);
	    ad_system_disease_code_recursion(&node->head, &temp_list_head);
	}

	buffer = NULL;
    }

    size_t size = 0;
    char comma[4] = "";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &temp_list_head) {

	ad_system_disease_code_t *node = NULL;
	node = (ad_system_disease_code_t *)p;

	if (strncmp(node->code, "1020003012", 10) != 0) {
	    fprintf(stdout, "2-->%s\n", node->code);
	    fprintf(stderr, "2-->%s\n", node->code);
	    size += sprintf(&out_code_buff[size], "%s%s", comma, node->code);
	    comma[0] = ',';
	}
    }

    out_code_buff[size] = 0;
    ad_system_disease_code_free(&disease_code_head);
    ad_system_disease_code_free(&temp_list_head);
    return 0;
}



