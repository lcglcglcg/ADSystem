
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ad_system_region_code.h"

static lg_rbtree_head region_code_head = RB_ROOT;
int ad_system_region_code_add(char *father, char *child, unsigned int code)
{

    lg_rbtree_node *parent = NULL;
    lg_rbtree_node **node = &region_code_head.rb_node;
    ad_system_region_code_t *temp = NULL;

    while (*node) {

	parent = *node;
	temp = (ad_system_region_code_t *)parent;

	if (code < temp->code)
	    node = &(*node)->rb_left;
	else if (code > temp->code)
	    node = &(*node)->rb_right;
	else
	    return -1;
    }

    if (!(temp = malloc(sizeof(ad_system_region_code_t))))
	return -1;

    temp->code = code;
    strcpy(temp->father, father);
    strcpy(temp->child, child);

    lg_rbtree_link_node(&temp->tree, parent, node);
    lg_rbtree_insert_color(&temp->tree, &region_code_head);
    return 0;
}

ad_system_region_code_t *ad_system_region_code_find(unsigned int code)
{

    lg_rbtree_node *node = region_code_head.rb_node;
    ad_system_region_code_t *temp = NULL;

    while (node) {

	temp = (ad_system_region_code_t *)node;

	if (code < temp->code)
	    node = node->rb_left;
	else if (code > temp->code)
	    node = node->rb_right;
	else
	    return temp;
    }

    return NULL;
}

int ad_system_region_code_load(const char *path)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    char buffer[1024];
    while(fgets(buffer, 1024, fp)) {

	char *father = strtok(buffer, ",");
	char *child = strtok(NULL, ",");
	char *code_str = strtok(NULL, "\n");
	unsigned int code = atoi(code_str);
	ad_system_region_code_add(father, child, code);
    }

    fclose(fp);
    return 0;
}



