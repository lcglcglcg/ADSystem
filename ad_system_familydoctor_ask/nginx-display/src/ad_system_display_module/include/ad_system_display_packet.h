
#ifndef	__AD_SYSTEM_DISPLAY_PACKET_H__
#define	__AD_SYSTEM_DISPLAY_PACKET_H__

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "lg_ngx_network.h"
#include "lg_ngx_network.h"

typedef struct {

    ngx_queue_t next;
    char *key;
    char *value;
    int type;

    int index;
    void *m_data;

} ad_system_ngx_list_t;

extern int ad_system_display_packet_parse(ngx_http_request_t *r, char *buffer, size_t size);
extern ad_system_ngx_list_t *ad_system_kv_find(ngx_queue_t *head, char *key);
extern void ad_system_kv_print(ngx_queue_t *head);

// 竞价
extern int ad_system_db_select(ngx_http_request_t *r, lg_ngx_network_t *network,
	int ele_type,
	int doctors_id,
	char *js_back_call,
	char *region_code,
	char *disease_code,
	ngx_queue_t *place_head);

#endif


