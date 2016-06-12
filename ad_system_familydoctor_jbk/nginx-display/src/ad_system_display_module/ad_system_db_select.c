
#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "lg_json.h"
#include "lg_string.h"
#include "lg_ngx_uri.h"
#include "lg_ngx_network.h"
#include "ad_system_display_db.h"
#include "ad_system_display_packet.h"

#define CLICK_HOSTS "http://39.haomeit.com"

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

int ad_system_db_select_cmp(ngx_queue_t *head, ad_system_info_t *info)
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

int ad_system_db_select_cmp_r(ngx_queue_t *head, ad_system_info_t *info)
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
	char *db_type, 
	char *js_back_call,
	ngx_queue_t *place)
{

    // 查询地域科室对应的全部创意
    lg_list_t *p = NULL;
    lg_list_t *info_head = ad_system_db_element_find_img(region_code, disease_code, db_type);

    // 匹配广告
    if (info_head) {
	lg_list_for_each(p, info_head) {

	    lg_db_list_node_t *node = (lg_db_list_node_t *)p;
	    ad_system_info_t *info = (ad_system_info_t *)node->m_data;
	    ad_system_db_select_cmp(place, info);
	}
    }

    // 剩余流量
    info_head = ad_system_db_element_default_img();
    lg_list_for_each(p, info_head) {

	lg_db_list_node_t *node = (lg_db_list_node_t *)p;
	ad_system_info_t *info = (ad_system_info_t *)node->m_data;
	ad_system_db_select_cmp(place, info);
    }

    return ad_system_display_send_data(r, network,
	    js_back_call,
	    region_code,
	    place);
}

int ad_system_db_select_txt(ngx_http_request_t *r, lg_ngx_network_t *network,
	char *region_code, 
	char *disease_code, 
	char *db_type, 
	char *js_back_call,
	ngx_queue_t *place)
{

    // 查询地域科室对应的全部创意
    lg_list_t *info_head = ad_system_db_element_find_txt(region_code, disease_code, db_type);
    if (!info_head) return ad_system_display_send_empty(r, network, js_back_call);

    // 匹配广告
    lg_list_t *p = NULL;
    lg_list_for_each(p, info_head) {

	lg_db_list_node_t *node = (lg_db_list_node_t *)p;
	ad_system_info_t *info = (ad_system_info_t *)node->m_data;
	ad_system_db_select_cmp_r(place, info);
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
    char *db_type = NULL;
    fprintf(stderr, "[ele_type][%d]\n", ele_type);
    fprintf(stderr, "[region_code][%s]\n", region_code);
    fprintf(stderr, "[disease_code][%s]\n", disease_code);
    fprintf(stderr, "[js_back_call][%s]\n", js_back_call);
    // ad_system_kv_print(place);
#endif

    if (ele_type == 1) {

	return ad_system_db_select_txt(r, network,
		region_code, 
		disease_code, 
		db_type, 
		js_back_call,
		place); 

    } else if (ele_type == 2) {

	return ad_system_db_select_img(r, network,
		region_code, 
		disease_code, 
		db_type, 
		js_back_call,
		place);
    }

    return ad_system_display_send_empty(r, network, js_back_call);
}





