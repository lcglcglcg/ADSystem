
#include "lg_string.h"
#include "lg_ngx_uri.h"
#include "lg_ngx_network.h"
#include "lg_ngx_session.h"
#include "ad_system_display_module.h"
#include "ad_system_display_packet.h"
#include "ad_system_display_db.h"

int ad_system_display_send_empty(ngx_http_request_t *r, lg_ngx_network_t *network, char *js_back_call);

int ad_system_return_code(ngx_http_request_t *r, lg_ngx_network_t *network, char *js_back, int code)
{

    char buffer[1024];
    size_t size = sprintf(buffer, "%s({\"errCode\":\"%d\"});", js_back, code);
    return lg_ngx_network_buffer_send(r, network, buffer, size);
}

ad_system_ngx_list_t *ad_system_kv_find(ngx_queue_t *head, char *key)
{
    if (!key) return NULL;

    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (strcmp(node->key, key) == 0)
	    return node;
    }

    return NULL;
}

void ad_system_kv_print(ngx_queue_t *head)
{

    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (node && node->value)
	    fprintf(stdout, "[%s][%s]\n", node->key, node->value);
	else
	    fprintf(stdout, "[%s][null]\n", node->key);
    }

    fprintf(stdout, "============================\n");
}

void ad_system_display_packet_parse_arg(ngx_http_request_t *r, char *buffer, ngx_queue_t *head)
{

    char *saveptr = NULL;
    char *str = strtok_r(buffer, "&", &saveptr);
    while(str) {

	ad_system_ngx_list_t *node = ngx_pcalloc(r->pool, sizeof(ad_system_ngx_list_t));
	node->key = str;
	node->value = lg_string_cutting(str, '=');
	ngx_queue_insert_tail(head, &node->next);

	str = strtok_r(NULL, "&", &saveptr);
    }
}

void ad_system_display_packet_parse_url(ngx_http_request_t *r, char *buffer, ngx_queue_t *head)
{

    char *saveptr = NULL;
    char *str = strtok_r(buffer, "./:", &saveptr);
    while(str) {

	if (strcmp(str, "http") != 0
		&& strcmp(str, "www") != 0
		&& strcmp(str, "58") != 0
		&& strcmp(str, "com") != 0) {

	    ad_system_kv_t *code = ad_system_db_element_find_code(str);
	    if (code) {

		ad_system_ngx_list_t *node = ngx_pcalloc(r->pool, sizeof(ad_system_ngx_list_t));

		if (node) {
		    node->key = str;
		    node->value = code->value;
		    node->type = code->type;
		    ngx_queue_insert_tail(head, &node->next);
		}
	    }
	}

	str = strtok_r(NULL, "./:", &saveptr);
    }
}

int ad_system_display_packet_parse_place(ngx_http_request_t *r, char *buffer, ngx_queue_t *head)
{

    int index = 1;
    char *saveptr = NULL;
    char *str = strtok_r(buffer, ",", &saveptr);
    while(str) {

	ad_system_ngx_list_t *node = ngx_pcalloc(r->pool, sizeof(ad_system_ngx_list_t));
	ad_system_kv_t *code = ad_system_db_element_find_place(str);
	if(code) {
	    node->key = str;
	    node->value = code->value;
	    node->type = 0;
	    node->index = index++;

	} else {
	    node->key = str;
	    node->value = NULL;
	    node->type = 0;
	    node->index = index++;
	}

	ngx_queue_insert_tail(head, &node->next);
	str = strtok_r(NULL, ",", &saveptr);
    }

    return index;
}

char *ad_system_display_packet_find_code(ngx_queue_t *head, int type)
{

    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (node->value && node->type == type)
	    return node->value;
    }

    return NULL;
}

int ad_system_display_packet_parse(ngx_http_request_t *r, char *buffer, size_t size)
{

    ngx_queue_t arg_head;
    ngx_queue_t url_head;
    ngx_queue_t place_head;

    ngx_queue_init(&arg_head);
    ngx_queue_init(&url_head);
    ngx_queue_init(&place_head);

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));
    network.content_type.data = LG_NGX_URI_JS2_STR;
    network.content_type.len = LG_NGX_URI_JS2_SIZE;

    // arg parse
    ad_system_display_packet_parse_arg(r, buffer, &arg_head);
    // ad_system_kv_print(&arg_head);

    // url parse 
    ad_system_ngx_list_t *node = ad_system_kv_find(&arg_head, "url");
    if (!node || !node->value) return lg_ngx_network_str_send(r, &network, "{url:err}");
    ad_system_display_packet_parse_url(r, node->value, &url_head);
    ad_system_kv_print(&url_head);

    // get cb (js back call)
    char *js_back_call = NULL;
    node = ad_system_kv_find(&arg_head, "cb");
    if (node && node->value && *node->value)
	js_back_call = node->value;

    // get place 
    node = ad_system_kv_find(&arg_head, "cabin");
    if (node && node->value && *node->value)
	ad_system_display_packet_parse_place(r, node->value, &place_head);
    // ad_system_kv_print(&place_head);

    // get region_code
    // char *region_code = ad_system_display_packet_find_code(&url_head, 1);
    char *addr = NULL;
    char _addr[128];
    node = ad_system_kv_find(&arg_head, "ip_addr");
    if (node && node->value && *node->value) {
	addr = node->value;
    } else {
	addr = (lg_ngx_network_get_ipaddr(r, _addr));
    }

    char *region_code = NULL;
    if (addr) region_code = ad_system_db_element_find_ip(addr);

    // get disease_code
    char *disease_code = NULL;
    node = ad_system_kv_find(&arg_head, "disease_uri");
    if (node && node->value && *node->value) {

	ad_system_kv_t *kv = ad_system_db_element_find_disease(node->value);
	if (kv && kv->value && *kv->value)
	    disease_code = kv->value;
    }

    if (!disease_code)
	disease_code = ad_system_display_packet_find_code(&url_head, 2);

    // get ele 1:img 2:txt
    int ele = 0;
    node = ad_system_kv_find(&arg_head, "ele");
    if (node && node->value && *node->value)
	ele = atoi(node->value);

    return ad_system_db_select(r, &network,
	    ele,
	    js_back_call,
	    region_code,
	    disease_code,
	    &place_head);
}



