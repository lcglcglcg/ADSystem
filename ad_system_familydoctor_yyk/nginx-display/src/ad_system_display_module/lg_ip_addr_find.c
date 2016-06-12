
#include "lg_ip_addr_find.h"

int _lg_ip_addr_add(lg_rbtree_head *head, lg_ip_addr_t *ip_node)
{

    lg_ip_addr_t *parent = NULL;
    lg_rbtree_node **node = &head->rb_node;

    while (*node) {

	parent = (lg_ip_addr_t *)*node;

	if (ip_node->ip_s < parent->ip_s)
	    node = &(*node)->rb_left;
	else if (ip_node->ip_e > parent->ip_s)
	    node = &(*node)->rb_right;
	else
	    return -1;
    }

    lg_rbtree_link_node(&ip_node->node, &parent->node, node);
    lg_rbtree_insert_color(&ip_node->node, head);
    return 0;
}

int lg_ip_addr_add(lg_rbtree_head *head,
	char *ip_s,
	char *ip_e, 
	char *addr_s, 
	char *addr_e,
	char *name,
	char *code)
{

    lg_ip_addr_t *node = malloc(sizeof(lg_ip_addr_t));
    node->ip_s = atoi(ip_s);
    node->ip_e = atoi(ip_e);
    strcpy(node->addr_s,	addr_s);
    strcpy(node->addr_e,	addr_e);
    strcpy(node->name,		name);
    strcpy(node->code,		code);

    return _lg_ip_addr_add(head, node);
}

int lg_ip_addr_load(lg_rbtree_head *head, char *path)
{


    FILE *fp = fopen(path, "r");
    if (!fp) {

	fprintf(stdout, "open ip.csv: err\n");
	return -1;
    }

    char buffer[1024];
    while(fgets(buffer, 1024, fp)) {
	
	char *code = strtok(buffer, "\t");
	char *name = strtok(NULL, "\t");
	char *addr_s = strtok(NULL, "\t");
	char *addr_e = strtok(NULL, "\t");
	char *ip_s = strtok(NULL, "\t");
	char *ip_e = strtok(NULL, "\t");

	lg_ip_addr_add(head,
		ip_s,
		ip_e, 
		addr_s, 
		addr_e,
		name,
		code);
    }

    return 0;
}

lg_ip_addr_t *_lg_ip_addr_find(lg_rbtree_head *head, uint32_t ip_addr)
{

    lg_rbtree_node *node = head->rb_node;

    while (node) {

	lg_ip_addr_t *ip_node = (lg_ip_addr_t *)node;

	if (ip_addr >= ip_node->ip_s && ip_addr <= ip_node->ip_e)
	    return ip_node;
	else if (ip_addr < ip_node->ip_s)
	    node = node->rb_left;
	else if (ip_addr > ip_node->ip_s)
	    node = node->rb_right;
	else
	    return NULL;
    }

    return NULL;
}

uint32_t lg_ip_to_long(char *ip)
{

    uint8_t i, n;
    uint32_t ip_sec = 0;
    int8_t ip_level = 3;
    uint32_t ip_long = 0;
    uint8_t ip_len = strlen(ip);

    for (i = 0;i <= ip_len; i++) {

	if (i != ip_len && ip[i] != '.' && (ip[i] < 48 || ip[i] > 57))
	    continue;

	if (ip[i]=='.' || i == ip_len) {

	    if (ip_level == -1)
		return 0;

	    for (n = 0; n < ip_level; n++)
		ip_sec *= 256;

	    ip_long += ip_sec;
	    if (i == ip_len)
		break;

	    ip_level--;
	    ip_sec = 0;

	} else {

	    ip_sec = ip_sec * 10 + (ip[i] - 48);
	}
    }

    return ip_long;
}

lg_ip_addr_t *lg_ip_addr_find(lg_rbtree_head *head, char *addr)
{
    return _lg_ip_addr_find(head, lg_ip_to_long(addr));
}




