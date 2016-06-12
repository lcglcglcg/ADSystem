
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ad_system_domain_parse.h"

char *ad_system_domain_parse_r(char *buffer, char *domain)
{

    char *str = NULL;
    char *domain_cut = NULL;
    char *domain_end = NULL;
    for (str = buffer; *str; str++) {

	if (*str == '.') {

	    if (!domain_cut) {

		domain_cut = str;

	    } else if (!domain_end) {
		domain_end = str;
	    }
	}
    }

    if (domain_end) {

	strcpy(domain, &domain_cut[1]);
	return domain;
    }

    if (domain_cut) {

	strcpy(domain, buffer);
	return domain;
    }

    return NULL;
}

char *ad_system_domain_parse(const char *url, char *domain)
{
    if (strncmp(url, "http://", 7) == 0)
	url += 7;

    char *str = NULL;
    char buffer[1024] = {0};
    char *p_buffer = buffer;
    for (str = (char *)url;*str && *str != '/'; str++)
	*p_buffer++ = *str;

    return ad_system_domain_parse_r(buffer, domain);
}

char *ad_system_domain_memory_add(lg_list_t *head, const char *key, const char *domain_str)
{

    ad_system_domain_memory_t *node = NULL;
    if (!(node = malloc(sizeof(ad_system_domain_memory_t))))
	return NULL;

    strcpy(node->key, key);
    strcpy(node->domain_str, domain_str);
    lg_list_add(head, &node->list);
    return node->key;
}

char *ad_system_domain_memory_find(lg_list_t *head, char *key, char *domain_str)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, head) {

	ad_system_domain_memory_t *node = NULL;
	node = (ad_system_domain_memory_t *)p;

	if (key) {

	    if (strcmp(node->key, key) == 0)
		return node->domain_str;

	} else if (domain_str) {

	    if (strcmp(node->domain_str, domain_str) == 0)
		return node->key;
	}
    }

    return NULL;
}

void ad_system_domain_memory_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    lg_list_t *node = NULL;

    while(p != head) {

	node = p;
	p = p->next;
	free(node);
    }

    lg_list_init(head);
}

