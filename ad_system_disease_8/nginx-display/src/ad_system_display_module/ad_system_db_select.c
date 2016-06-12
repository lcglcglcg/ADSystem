
#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "lg_json.h"
#include "lg_string.h"
#include "lg_ngx_uri.h"
#include "lg_ngx_network.h"
#include "ad_system_display_db.h"
#include "ad_system_display_packet.h"

#define CLICK_HOSTS "http://disease8.haomeit.com"

int ad_system_display_send_empty(ngx_http_request_t *r, lg_ngx_network_t *network, char *js_back_call)
{

    char buffer[128];
    size_t size = sprintf(buffer, "%s({\"data\":[", js_back_call);
    lg_ngx_network_buffer_add(r, network, buffer, size);

    size = sprintf(buffer, "],\"targetUrl\":\"%s\"});", CLICK_HOSTS);
    lg_ngx_network_buffer_add(r, network, buffer, size);
    return lg_ngx_network_send(r, network);
}

int ad_system_display_send_data(ngx_http_request_t *r, lg_ngx_network_t *network,
	char *js_back_call,
	char *region_code,
	ngx_queue_t *head)
{

    int index = 0;
    char buffer[40960];
    size_t size = sprintf(buffer, "%s({\"data\":[", js_back_call);
    lg_ngx_network_buffer_add(r, network, buffer, size);

    char comma[4] = "{";
    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	index++;
	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (node->m_data) {

	    size = sprintf(buffer, "%s\"num\":\"%u\", \"placeId\":\"%s\",", comma, index, node->key);
	    lg_ngx_network_buffer_add(r, network, buffer, size);

	    ad_system_info_t *info = (ad_system_info_t *)node->m_data;
	    lg_ngx_network_buffer_add(r, network, info->buffer, info->size);

	    size = sprintf(buffer, "}");
	    lg_ngx_network_buffer_add(r, network, buffer, size);

	    strcpy(comma, ",{");
	}
    }

    size = sprintf(buffer, "],\"targetUrl\":\"%s\"});", "http://39.haomeit.com");
    lg_ngx_network_buffer_add(r, network, buffer, size);

    return lg_ngx_network_send(r, network);
}

int ad_system_db_select_new_list_cmp(void *priv, lg_list_t *a, lg_list_t *b) 
{

    ad_system_info_t *info_a = lg_list_entry(a, ad_system_info_t, select_next);
    ad_system_info_t *info_b = lg_list_entry(b, ad_system_info_t, select_next);

    if (info_b->cpc_price > info_a->cpc_price)
	return 1;

    return -1;
}

void ad_system_db_select_new_list(ngx_http_request_t *r, lg_list_t *head, lg_list_t *new_head)
{

    if (!new_head) return;

    lg_list_t *p = NULL;
    lg_list_for_each(p, new_head) {

	ad_system_list_info_t *node = (ad_system_list_info_t *)p;
	ad_system_info_t *info = (ad_system_info_t *)node->data;
	lg_list_add(head, &info->select_next);
    }
}

int ad_system_db_select_cmp_img(ngx_queue_t *head, ad_system_info_t *info)
{

    if (!info) return -1;

    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (node->value && strcmp(node->value, info->img_size) == 0) { //比较尺寸
	    if (!node->m_data) {

		node->m_data = info;
		return 0;
	    }
	}
    }

    return -1;
}

int ad_system_db_select_cmp_txt(ngx_queue_t *head, ad_system_info_t *info)
{

    if (!info) return -1;

    ngx_queue_t *p = NULL;
    for (p = ngx_queue_head(head);
	    p != ngx_queue_sentinel(head);
	    p = ngx_queue_next(p)) {

	ad_system_ngx_list_t *node = (ad_system_ngx_list_t *)p;
	if (!node->m_data) {

	    node->m_data = info;
	    return 0;
	}
    }

    return -1;
}

int ad_system_db_select_img(ngx_http_request_t *r, lg_ngx_network_t *network,
	char *region_code, 
	char *disease_code, 
	char *js_back_call,
	ngx_queue_t *place)
{

    lg_list_t head;
    lg_list_init(&head);

    // 查询指定地域
    lg_list_t *info_head = ad_system_db_element_find_img(region_code, disease_code);
    ad_system_db_select_new_list(r, &head, info_head);

    // fprintf(stderr, "[1]%p\n", info_head);

    // 查询全国
    info_head = ad_system_db_element_find_img_all_region(disease_code);
    ad_system_db_select_new_list(r, &head, info_head);
    
    // fprintf(stderr, "[2]%p\n", info_head);

    lg_list_sort(NULL, &head, 40960, ad_system_db_select_new_list_cmp);

    // 匹配广告
    lg_list_t *p = NULL;
    lg_list_for_each(p, &head) {

	ad_system_info_t *info = lg_list_entry(p, ad_system_info_t, select_next);
	ad_system_db_select_cmp_img(place, info);
    }

    return ad_system_display_send_data(r, network,
	    js_back_call,
	    region_code,
	    place);
}

int ad_system_db_select_txt(ngx_http_request_t *r, lg_ngx_network_t *network,
	char *region_code, 
	char *disease_code, 
	char *js_back_call,
	ngx_queue_t *place)
{

    lg_list_t head;
    lg_list_init(&head);

    // 查询指定地域
    lg_list_t *info_head = ad_system_db_element_find_txt(region_code, disease_code);
    ad_system_db_select_new_list(r, &head, info_head);

    // 查询全国
    info_head = ad_system_db_element_find_txt_all_region(disease_code);
    ad_system_db_select_new_list(r, &head, info_head);

    lg_list_sort(NULL, &head, 40960, ad_system_db_select_new_list_cmp);

    // 匹配广告
    lg_list_t *p = NULL;
    lg_list_for_each(p, &head) {

	ad_system_info_t *info = lg_list_entry(p, ad_system_info_t, select_next);
	ad_system_db_select_cmp_txt(place, info);
    }

    return ad_system_display_send_data(r, network,
	    js_back_call,
	    region_code,
	    place);
}

int ad_system_db_select(ngx_http_request_t *r, lg_ngx_network_t *network,
	int ele_type,
	char *js_back_call,
	char *region_code,
	char *disease_code,
	ngx_queue_t *place)
{

#if 1
    fprintf(stderr, "[ele_type][%d]\n", ele_type);
    fprintf(stderr, "[region_code][%s]\n", region_code);
    fprintf(stderr, "[disease_code][%s]\n", disease_code);
    fprintf(stderr, "[js_back_call][%s]\n", js_back_call);
    if (!ele_type || !js_back_call || !disease_code)
	return ad_system_display_send_empty(r, network, js_back_call);
#endif

    return ad_system_db_select_img(r, network,
	    region_code, 
	    disease_code, 
	    js_back_call,
	    place);

    return ad_system_display_send_empty(r, network, js_back_call);
}




