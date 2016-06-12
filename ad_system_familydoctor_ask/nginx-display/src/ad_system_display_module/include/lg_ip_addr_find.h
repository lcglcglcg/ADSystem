
#ifndef	__LG_IP_ADDR_FIND_H__
#define	__LG_IP_ADDR_FIND_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lg_rbtree.h"

typedef struct {

    lg_rbtree_node node;

    unsigned int ip_s;
    unsigned int ip_e;

    char addr_s[128];
    char addr_e[128];

    char name[128];
    char code[128];

} lg_ip_addr_t;

extern int lg_ip_addr_load(lg_rbtree_head *head, char *path);
extern lg_ip_addr_t *lg_ip_addr_find(lg_rbtree_head *head, char *addr);

#endif



