
#include <ngx_core.h>
#include <ngx_http.h>
#include <pthread.h>

#include "gd.h"
#include "gdfontl.h"

#include "lg_string.h"
#include "lg_string_json.h"
#include "ad_system_json_packet.h"

#include "ad_system_init.h"
#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_redis.h"

#include "ad_system_packet.h"
#include "ad_system_report.h"
#include "ad_system_region_code.h"

//返回CODE
int ad_system_packet_send_code(ngx_http_request_t *r, int code)
{

    char *p_buffer = ngx_pcalloc(r->pool, 1024);
    size_t size = sprintf(p_buffer, "{\"CODE\":\"%u\"}", code);
    ngx_chain_t *buffer = ad_system_buffer_to_ngx_chain(r, p_buffer, size);
    return ad_system_http_send(r, buffer, size);
}

int ad_system_packet_lcglcg(ngx_http_request_t *r)
{

    return ad_system_packet_send_code(r, 0);
}

//检测是否登录
int ad_system_packet_login_check(ngx_http_request_t *r)
{

    if (ngx_ad_system_get_session(r))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_redis_get_login_session(r->ad_system_session_id, NULL))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_OK);
}

//发送验证码
int ad_system_packet_send_image(ngx_http_request_t *r)
{

    unsigned char result[128];
    sprintf((char *)result, "%u", rand() % 8888 + 1000);

    gdImagePtr im = gdImageCreate(32, 16);
    gdImageColorAllocate(im, 255, 255, 255);
    gdImageString(im, gdFontGetLarge(), 0, 0, result, 1);

    int size = 0;
    char *img_buffer = gdImagePngPtr(im, &size);
    gdImageDestroy(im);

    char session_id[128];
    lg_string_uuid(session_id);
    ad_system_redis_add_var_code(session_id, (const char *)result);
    ngx_ad_system_set_session(r, session_id);

    ngx_chain_t *buffer = NULL;
    if (!(buffer = ad_system_buffer_to_ngx_chain(r, img_buffer, size)))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    free(img_buffer);
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = size;
    int rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	return rc;

    return ngx_http_output_filter(r, buffer);
}

//登录
int ad_system_packet_login(ngx_http_request_t *r, const char *account_name, const char *password_md5, const char *var_code)
{

    //获取session
    if (ngx_ad_system_get_session(r))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char img_result[128] = {0};
    ad_system_redis_get_var_code(r->ad_system_session_id, img_result);

    if (strcmp(var_code, img_result))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_IMG_ERR);

    char account_id[128];
    if (ad_system_sql_login(account_name, password_md5, account_id))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    //更新session
    char session_id[128];
    lg_string_uuid(session_id);
    ad_system_redis_set_login_session(session_id, account_id);
    ngx_ad_system_set_session(r, session_id);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//登出
int ad_system_packet_logout(ngx_http_request_t *r)
{
    ad_system_redis_del_login_session(r->ad_system_session_id);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_get_title_info(ngx_http_request_t *r)
{

    char account_name[1024];
    if (ad_system_sql_get_get_title_info(r->ad_system_account_id, account_name))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    char buffer[2048];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"ACCOUNT_NAME\":\"%s\"}", account_name);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_get_text_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char count[128] = "0";
    if (ad_system_sql_get_text_creative_count(json->FIND_NAME_COL, json->FIND_NAME, json->FIND_STATUS, json->BALANCE_TYPE, count) != 0)
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_text_creative(&sql_conn,
		json->FIND_NAME_COL,
		json->FIND_NAME, 
		json->FIND_STATUS, 
		json->BALANCE_TYPE,
		json->SORT_TYPE, 
		json->SORT_COLUMN, 
		json->page_index_s, 
		json->page_index_e))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"CREATIVE_COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	int CREATIVE_APPEND_ID_TYPE = 1;
	const char *CREATIVE_APPEND_ID = ad_system_sql_get_string(&sql_conn, NULL, 10);
	if (CREATIVE_APPEND_ID) CREATIVE_APPEND_ID_TYPE = 2;

	max += size = lg_string_json_sprintf(buffer, comma, "}", 14,

		"%d", "ACCOUNT_ID",			ad_system_sql_get_number(&sql_conn, NULL, 1),  
		"%s", "ACCOUNT_NAME",                   ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "CREATIVE_ID",                    ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "CREATIVE_NAME",                  ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%d", "CREATIVE_STATE",                 ad_system_sql_get_number(&sql_conn, NULL, 5),
		"%s", "CREATIVE_REFUSE_REASON",         ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "CREATIVE_DESCRIPTION",           ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "CREATIVE_DESTINATION_URL",       ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "CREATIVE_DISPLAY_URL",           ad_system_sql_get_string(&sql_conn, NULL, 9),  
		"%d", "CREATIVE_APPEND_TYPE",           CREATIVE_APPEND_ID_TYPE,
		"%s", "CREATIVE_APPEND_ID",             CREATIVE_APPEND_ID,
		"%s", "LATE_LOGIN_TIME",                ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "AUDITOR_NAME",                   ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "AUDITOR_UPDATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 13));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_append_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    char count[128] = "0";
    int ret = ad_system_sql_get_append_creative_count(json->FIND_NAME_COL, json->FIND_NAME, json->FIND_STATUS, json->BALANCE_TYPE, count);
    if (ret == 100 || count[0] == '0') {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_append_creative(&sql_conn,
		json->FIND_NAME_COL,
		json->FIND_NAME, 
		json->FIND_STATUS, 
		json->BALANCE_TYPE,
		json->SORT_TYPE, 
		json->SORT_COLUMN, 
		json->page_index_s, 
		json->page_index_e))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"CREATIVE_COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 9,
		"%d", "ACCOUNT_ID",			ad_system_sql_get_number(&sql_conn, NULL, 1),
		"%s", "ACCOUNT_NAME",             	ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "CREATIVE_ID",              	ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%d", "CREATIVE_STATE",           	ad_system_sql_get_number(&sql_conn, NULL, 4),
		"%s", "CREATIVE_REFUSE_REASON",    	ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "CREATIVE_CONTENT",         	ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "LATE_LOGIN_TIME",          	ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "AUDITOR_NAME",             	ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "AUDITOR_UPDATE_TIME",      	ad_system_sql_get_string(&sql_conn, NULL, 9));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_img_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    size_t size, max = 0;
    char buffer[40960] = {0};

    char count[128] = "0";
    if (ad_system_sql_get_img_creative_count(json->FIND_NAME_COL, json->FIND_NAME, json->FIND_STATUS, json->BALANCE_TYPE, count))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_get_img_creative(&sql_conn,
	    json->FIND_NAME_COL,
	    json->FIND_NAME, 
	    json->FIND_STATUS, 
	    json->DSP_STATUS, 
	    json->BALANCE_TYPE,
	    json->SORT_TYPE, 
	    json->SORT_COLUMN, 
	    json->page_index_s, 
	    json->page_index_e);
    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"CREATIVE_COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 16,

		"%s", "ACCOUNT_ID",			ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "ACCOUNT_NAME",                   ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "CREATIVE_ID",                    ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "CREATIVE_NAME",                  ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%u", "CREATIVE_STATE",                 ad_system_sql_get_number(&sql_conn, NULL, 6),
		"%s", "CREATIVE_REFUSE_REASON",         ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "CREATIVE_IMG_SIZE",              ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "CREATIVE_IMG_SIZE_IMGLENGTH",    ad_system_sql_get_string(&sql_conn, NULL, 9),
		"%s", "CREATIVE_FILE_PATH",             ad_system_sql_get_string(&sql_conn, NULL, 10),
		"%s", "CREATIVE_DESTINATION_URL",       ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "CREATIVE_DISPLAY_URL",           ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "LATE_LOGIN_TIME",                ad_system_sql_get_string(&sql_conn, NULL, 13),
		"%s", "AUDITOR_NAME",                   ad_system_sql_get_string(&sql_conn, NULL, 14),
		"%s", "AUDITOR_UPDATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 15),
		"%u", "CREATIVE_DSP_STATE",		ad_system_sql_get_number(&sql_conn, NULL, 16),
		"%s", "CREATIVE_DSP_REFUSE_REASON",     ad_system_sql_get_string(&sql_conn, NULL, 17));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}"); 
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_set_creative_batch(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int creative_status = 0;
    if (json->CREATIVE_STATUS == 1)
	creative_status = 31;
    else if (json->CREATIVE_STATUS == 2)
	creative_status = 35;
    else
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[40960];
    char *save_ptr = NULL;
    char *creative_id = lg_string_strtok(json->CREATIVE_ID_ARRAY, buffer, &save_ptr, ',');

    while(creative_id) {

	if (json->CREATIVE_TYPE == 1) {

	    if (ad_system_sql_set_text_creative(r->ad_system_account_id, creative_id, creative_status, json->CREATIVE_REFUSE_REASON) != 0)
		return ad_system_packet_send_code(r, PACKET_CODE_ERR);

	} else if (json->CREATIVE_TYPE == 2) {

	    if (ad_system_sql_set_append_creative(r->ad_system_account_id, creative_id, creative_status, json->CREATIVE_REFUSE_REASON) != 0)
		return ad_system_packet_send_code(r, PACKET_CODE_ERR);

	} else if (json->CREATIVE_TYPE == 3) {

	    if (ad_system_sql_set_img_creative(r->ad_system_account_id, creative_id, creative_status, json->CREATIVE_REFUSE_REASON) != 0)
		return ad_system_packet_send_code(r, PACKET_CODE_ERR);
	}

	creative_id = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_get_media_domain_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_report_media(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char domain_str[1024] = {0};
    if (json->MEDIA_TYPE == 1) {
	if (ad_system_mongo_get_media_domain_unique(json->MEDIA_ID, domain_str))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    } else if (json->MEDIA_TYPE != 2) {
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    ad_system_report_thread_t *p = NULL;
    if (!(p = malloc(sizeof(ad_system_report_thread_t))))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (ad_system_report_path_create(p->report_id, p->report_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    p->media_type = json->MEDIA_TYPE;
    strcpy(p->domain_str, domain_str);
    strcpy(p->time_s, json->TIME_START);
    strcpy(p->time_e, json->TIME_END);

    char buffer[1024];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"REPORT_ID\":\"%s\"}", p->report_id);

    pthread_t tid;
    pthread_create(&tid, NULL, ad_system_report_media_create, p);
    //ad_system_report_media_create(p);

    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_report_account(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char domain_str[1024] = {0};
    if (json->MEDIA_TYPE == 1) {
	if (ad_system_mongo_get_media_domain_unique(json->MEDIA_ID, domain_str))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    } else if (json->MEDIA_TYPE != 2) {
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    ad_system_report_thread_t *p = NULL;
    if (!(p = malloc(sizeof(ad_system_report_thread_t))))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (ad_system_report_path_create(p->report_id, p->report_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    p->date_type = json->DATE_TYPE;
    p->media_type = json->MEDIA_TYPE;
    strcpy(p->domain_str, domain_str);
    strcpy(p->time_s, json->TIME_START);
    strcpy(p->time_e, json->TIME_END);

    pthread_t tid;
    pthread_create(&tid, NULL, ad_system_report_account_create, p);
    //ad_system_report_account_create(p);

    char buffer[1024];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"REPORT_ID\":\"%s\"}", p->report_id);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_report_status(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    size_t size = 0;
    char buffer[2048] = {0};

    char report_path[1024];
    if (ad_system_redis_get_report(json->REPORT_ID, report_path)) {

	size = sprintf(buffer, "{\"CODE\":\"0\",\"STATUS\":\"1\",\"PATH\":\"null\"}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    if (strcmp(report_path, "null") == 0) {

	size = sprintf(buffer, "{\"CODE\":\"0\",\"STATUS\":\"3\",\"PATH\":\"null\"}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    size = sprintf(buffer, "{\"CODE\":\"0\",\"STATUS\":\"2\",\"PATH\":\"%s\"}", report_path);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_manager_get_account_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    char count[128] = {0};
    if (ad_system_sql_get_manager_account_list_count(count,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->STATUS,
		json->ACCOUNT_ID)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{},\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_manager_account_list(&sql_conn,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->STATUS,
		json->ACCOUNT_ID,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{},\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 30,
		"%s", "ACCOUNT_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "API_TOKEN",              ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "LOGIN_NAME",             ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "COMPANY_NAME",           ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "COMPANY_SITE",           ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "DOMAIN",                 ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "INDUSTRY",               ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "PROVINCE_ID",            ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "CITY_ID",                ad_system_sql_get_string(&sql_conn, NULL, 9),
		"%s", "ADDRESS",                ad_system_sql_get_string(&sql_conn, NULL, 10),
		"%s", "POSTCODE",               ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "CONTACT",                ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "TELEPHONE",              ad_system_sql_get_string(&sql_conn, NULL, 13),
		"%s", "FAX",                    ad_system_sql_get_string(&sql_conn, NULL, 14),
		"%s", "MOBILE",                 ad_system_sql_get_string(&sql_conn, NULL, 15),
		"%s", "EMAIL",                  ad_system_sql_get_string(&sql_conn, NULL, 16),
		"%s", "WEBIM",                  ad_system_sql_get_string(&sql_conn, NULL, 17),
		"%s", "GUAHAO_TOKEN",           ad_system_sql_get_string(&sql_conn, NULL, 18),
		"%s", "PROTECT_EMAIL",          ad_system_sql_get_string(&sql_conn, NULL, 26),
		"%s", "PROTECT_MOBILE",         ad_system_sql_get_string(&sql_conn, NULL, 27),
		"%s", "HOSPITAL_IMG",           ad_system_sql_get_string(&sql_conn, NULL, 28),
		"%s", "REGION_NAME_CN",         ad_system_sql_get_string(&sql_conn, NULL, 29),
		"%s", "CONSUMPTION",            ad_system_sql_get_string(&sql_conn, NULL, 30),
		"%s", "CREATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 31),
		"%s", "STATUS",                 ad_system_sql_get_string(&sql_conn, NULL, 32),
		"%s", "SALES",                  ad_system_sql_get_string(&sql_conn, NULL, 33),
		"%s", "CUSTOMER_SERVICE",       ad_system_sql_get_string(&sql_conn, NULL, 34),
		"%s", "PLACE_REGION",       	ad_system_sql_get_string(&sql_conn, NULL, 35),
		"%s", "JOIN_DSP",       	ad_system_sql_get_string(&sql_conn, NULL, 36),
		"%s", "DOCTORS_ID",       	ad_system_sql_get_string(&sql_conn, NULL, 37));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_manager_set_account(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{
    int ret = 0;
    if (json->UPDATE_TYPE & 131072) {

	if (!(json->STATUS == 1 || json->STATUS == 2))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    if (json->UPDATE_TYPE & 1048576 ) {

	if (!(json->PLACE_REGION == 1 || json->PLACE_REGION == 2))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

	if (json->PLACE_REGION == 2) {

	    if ((ret = campaign_region_check(json->ACCOUNT_ID)))
		return ad_system_packet_send_code(r, PACKET_CODE_REGION_CODE_ERR);
	}
    }

    fprintf(stdout, "ret=%d\n", ret);
    ret = ad_system_sql_set_manager_account_info(
	    json->UPDATE_TYPE,
	    json->ACCOUNT_ID,
	    json->COMPANY_NAME,
	    json->COMPANY_SITE,
	    json->INDUSTRY,
	    json->PROVINCE_ID,
	    json->CITY_ID,
	    json->ADDRESS,
	    json->POSTCODE,
	    json->CONTACT,
	    json->TELEPHONE,
	    json->FAX,
	    json->MOBILE,
	    json->EMAIL,
	    json->WEBIM,
	    json->GUAHAO_TOKEN,
	    json->HOSPITAL_IMG,
	    json->LOGIN_PASSWORD,
	    json->REGION_NAME_CN,
	    json->STATUS,
	    json->SALES,
	    json->CUSTOMER_SERVICE,
	    json->PLACE_REGION,
	    json->JOIN_DSP,
	    json->DOCTORS_ID);
    if (ret != 0)
	ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (json->UPDATE_TYPE & 16384)
	ad_system_hospital_image_copy(json->ACCOUNT_ID, json->HOSPITAL_IMG);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_manager_add_account(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int ret = ad_system_sql_add_manager_account_name_check(json->LOGIN_NAME);
    if (ret == -1)
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    else if (ret != 100)
	return ad_system_packet_send_code(r, PACKET_CODE_NAME_REPEAT);

    char account_id[128] = {0};
    if (ad_system_sql_manager_new_account_id(account_id))
	return ad_system_packet_send_code(r, PACKET_CODE_NAME_REPEAT);

    if (!(json->PLACE_REGION == 1 || json->PLACE_REGION == 2))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    if (ad_system_sql_add_manager_account(r->ad_system_account_id, json->INSERT_TYPE,
		account_id,
		json->LOGIN_NAME,			
		json->LOGIN_PASSWORD,
		json->API_TOKEN,
		NULL,//DOMAIN
		json->COMPANY_NAME,
		json->COMPANY_SITE,
		json->INDUSTRY,
		json->PROVINCE_ID,
		json->CITY_ID,
		json->ADDRESS,
		json->POSTCODE,
		json->CONTACT,
		json->TELEPHONE,
		json->FAX,
		json->MOBILE,
		json->EMAIL,
		json->WEBIM,
		json->GUAHAO_TOKEN,
		json->REGION_NAME_CN,
		json->HOSPITAL_IMG,
		json->SALES,
		json->CUSTOMER_SERVICE,
		json->PLACE_REGION,
		json->JOIN_DSP,
		json->DOCTORS_ID))
		    return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (json->INSERT_TYPE & 16)
	ad_system_hospital_image_copy(account_id, json->HOSPITAL_IMG);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_manager_del_account(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_manager_account(json->ACCOUNT_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_add_manager_announcement(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char title[4096] = {0};
    char content[4096] = {0};
    ad_system_sql_string_escaped(title, json->ANNOUNCEMENT_TITLE, 0);
    ad_system_sql_string_escaped(content, json->CONTENT, 0);

    if (ad_system_sql_add_manager_announcement(r->ad_system_account_id, json->SEND_TIME, title, content))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_set_manager_announcement(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char title[4096] = {0};
    char content[4096] = {0};
    ad_system_sql_string_escaped(title, json->ANNOUNCEMENT_TITLE, 0);
    ad_system_sql_string_escaped(content, json->CONTENT, 0);

    if (ad_system_sql_set_manager_announcement(json->UPDATE_TYPE, r->ad_system_account_id, json->ANNOUNCEMENT_ID, json->SEND_TIME, title, content))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_del_manager_announcement(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_manager_announcement(json->ANNOUNCEMENT_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_get_manager_announcement(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    if (json->FIND_TYPE & 2)
	if (!(json->TIME_TYPE == 1 || json->TIME_TYPE == 2))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    if (json->FIND_TYPE & 4) {

	int status = json->STATUS;
	if (!(status == 1 || status == 2 || status == 3 || status == 4 || status == 100))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

	if (status == 3 || status == 4) {

	    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	    return ad_system_http_send_buffer(r, buffer, size);
	}
    }

    char count[128] = {0};
    if (ad_system_sql_get_manager_announcement_list_count(count,
		json->FIND_TYPE,
		json->FIND_NAME,
		json->TIME_TYPE,
		json->TIME_START,
		json->TIME_END,
		json->STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_get_manager_announcement_list(&sql_conn,
	    json->FIND_TYPE,
	    json->FIND_NAME,
	    json->TIME_TYPE,
	    json->TIME_START,
	    json->TIME_END,
	    json->STATUS,
	    json->SORT_TYPE,
	    json->SORT_COLUMN,
	    json->page_index_s,
	    json->page_index_e);
    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 9,
		"%s", "ANNOUNCEMENT_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "ANNOUNCEMENT_TITLE",             ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "SEND_TIME",                      ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "SEND_STATE",                     ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "CREATE_TIME",                    ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "CONTENT",                        ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "CREATE_NAME",                    ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "UPDATE_NAME",                    ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "UPDATE_TIME",                    ad_system_sql_get_string(&sql_conn, NULL, 9));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_brand_unique(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    size_t size = 0;
    char buffer[40960];

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_brand_unique(&sql_conn, json->BRAND_ID)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    size = lg_string_json_sprintf(buffer, "{", "}", 15,
	    "%s", "CODE", "0",
	    "%s", "BRAND_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
	    "%s", "AD_ORDER_NO",        	ad_system_sql_get_string(&sql_conn, NULL, 2),
	    "%s", "ACCOUNT_ID",         	ad_system_sql_get_string(&sql_conn, NULL, 3),
	    "%s", "ACCOUNT_NAME",       	ad_system_sql_get_string(&sql_conn, NULL, 4),
	    "%s", "DEPT_SERIALIZE",         ad_system_sql_get_string(&sql_conn, NULL, 5),
	    "%s", "REGION_TYPE",        	ad_system_sql_get_string(&sql_conn, NULL, 6),
	    "%s", "REGION_SERIALIZE",   	ad_system_sql_get_string(&sql_conn, NULL, 7),
	    "%s", "DAYS",               	ad_system_sql_get_string(&sql_conn, NULL, 8),
	    "%s", "SHOW_TIME_START",    	ad_system_sql_get_string(&sql_conn, NULL, 9),
	    "%s", "SHOW_TIME_END",		ad_system_sql_get_string(&sql_conn, NULL, 10),
	    "%s", "PRICE",                  ad_system_sql_get_string(&sql_conn, NULL, 11),
	    "%s", "CREATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 12),
	    "%s", "STATUS",                 ad_system_sql_get_string(&sql_conn, NULL, 13),
	    "%s", "REFUSE_REASON",          ad_system_sql_get_string(&sql_conn, NULL, 14));

    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_get_brand(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char DEPT_ARRAY[2048] = {0};
    if (json->DEPT_TYPE == 2)
	lg_string_json_to_sql_array(json->DEPT_SERIALIZE, DEPT_ARRAY);

    char REGION_ARRAY[2048] = {0};
    if (json->REGION_TYPE == 2)
	lg_string_json_to_sql_array(json->REGION_SERIALIZE, REGION_ARRAY);

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_get_brand(&sql_conn,
	    json->FIND_TYPE,
	    json->FIND_NAME,
	    json->STATUS,
	    json->DEPT_TYPE, 
	    DEPT_ARRAY, 
	    json->REGION_TYPE, 
	    REGION_ARRAY,
	    json->SHOW_TIME_TYPE,
	    json->SHOW_TIME_START,
	    json->SHOW_TIME_END,
	    json->CREATE_TIME_TYPE,
	    json->CREATE_TIME_START,
	    json->CREATE_TIME_END,
	    json->SORT_TYPE,
	    json->SORT_COLUMN,
	    json->BALANCE_TYPE,
	    json->page_index_s,
	    json->page_index_e);
    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    char count[128] = {0};
    if (ad_system_sql_get_brand_count(count,
		json->FIND_TYPE,
		json->FIND_NAME,
		json->STATUS,
		json->DEPT_TYPE, 
		DEPT_ARRAY, 
		json->REGION_TYPE, 
		REGION_ARRAY,
		json->SHOW_TIME_TYPE,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->CREATE_TIME_TYPE,
		json->CREATE_TIME_START,
		json->CREATE_TIME_END,
		json->BALANCE_TYPE)) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 16,
		"%s", "BRAND_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "AD_ORDER_NO",        	ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "ACCOUNT_ID",         	ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "ACCOUNT_NAME",       	ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "DEPT_SERIALIZE",         ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "REGION_TYPE",        	ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "REGION_SERIALIZE",   	ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "DAYS",               	ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "SHOW_TIME_START",    	ad_system_sql_get_string(&sql_conn, NULL, 9),
		"%s", "SHOW_TIME_END",		ad_system_sql_get_string(&sql_conn, NULL, 10),
		"%s", "PRICE",                  ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "CREATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "STATUS",                 ad_system_sql_get_string(&sql_conn, NULL, 13),
		"%s", "REFUSE_REASON",          ad_system_sql_get_string(&sql_conn, NULL, 14),
		"%s", "AUDITOR_UPDATE_TIME",    ad_system_sql_get_string(&sql_conn, NULL, 15),
		"%s", "AUDITOR_NAME",           ad_system_sql_get_string(&sql_conn, NULL, 16)
		);

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_find_brand_schedule(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->REGION_SERIALIZE && strlen(json->REGION_SERIALIZE) >= 4000)
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_REGION_LEN_ERR);

    if (json->DEPT_SERIALIZE && strlen(json->DEPT_SERIALIZE) >= 2000)
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_REGION_LEN_ERR);


    char DEPT_ARRAY[40960] = {0};
    if (json->DEPT_TYPE == 2)
	lg_string_json_to_sql_array(json->DEPT_SERIALIZE, DEPT_ARRAY);

    char REGION_ARRAY[409600] = {0};
    if (json->REGION_TYPE == 2) {

	char region_serialize[409600] = {0};
	lg_region_father_code_del(json->REGION_SERIALIZE, region_serialize);
	lg_string_json_to_sql_array(region_serialize, REGION_ARRAY);
    }

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    char count[128] = {0};
    if ( ad_system_sql_find_brand_schedule_count(count,
		json->DEPT_TYPE,
		DEPT_ARRAY,
		json->REGION_TYPE,
		REGION_ARRAY)) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_find_brand_schedule(&sql_conn,
	    json->DEPT_TYPE,
	    DEPT_ARRAY,
	    json->REGION_TYPE,
	    REGION_ARRAY,
	    json->SHOW_TIME_START,
	    json->SHOW_TIME_END,
	    json->page_index_s,
	    json->page_index_e,
	    json->PAGE_TYPE);

    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 3,
		"%s", "DEPT_SERIALIZE",		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "REGION_SERIALIZE",        	ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "STATUS",         	ad_system_sql_get_string(&sql_conn, NULL, 3)
		);

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_add_brand(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->PRICE && !(atoi(json->PRICE) > 0))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (ad_system_sql_brand_pric_check(&sql_conn, json->ACCOUNT_ID, json->PRICE)) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_PRICE_ERR);
    }

    char REGION_SERIALIZE[4096000] = {0};
    lg_region_father_code_add(json->REGION_SERIALIZE, REGION_SERIALIZE);
    fprintf(stderr, ">>>>>>>>>>>>>>>>>>%s::%s\n", json->REGION_SERIALIZE, REGION_SERIALIZE);

    if (ad_system_sql_brand_inspection(&sql_conn,
		r->ad_system_account_id,
		NULL, 
		1,
		NULL,
		json->ACCOUNT_ID,
		json->REGION_TYPE,
		REGION_SERIALIZE,
		json->DEPT_SERIALIZE,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->PRICE,
		r->ad_system_account_id)) {
	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_ERR);
    }

    ad_system_sql_free(&sql_conn);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_brand_inspection(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (json->STATUS == 1) {

	if (ad_system_sql_brand_pric_check(&sql_conn, json->ACCOUNT_ID, json->PRICE)) {

	    ad_system_sql_free(&sql_conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_PRICE_ERR);
	}

    } else if (json->STATUS != 2) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    char REGION_SERIALIZE[4096000] = {0};
    lg_region_father_code_add(json->REGION_SERIALIZE, REGION_SERIALIZE);
    fprintf(stderr, ">>>>>>>>>>>>>>>>>>%s::%s\n", json->REGION_SERIALIZE, REGION_SERIALIZE);

    if (ad_system_sql_brand_inspection(&sql_conn, 
		r->ad_system_account_id,
		json->BRAND_ID, 
		json->STATUS,
		json->REFUSE_REASON,
		json->ACCOUNT_ID,
		json->REGION_TYPE,
		REGION_SERIALIZE,
		json->DEPT_SERIALIZE,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->PRICE,
		r->ad_system_account_id)) {
	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_ERR);
    }

    ad_system_sql_free(&sql_conn);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_get_brand_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    if (!(json->STATUS == 1 || json->STATUS == 2 || json->STATUS == 3 || json->STATUS == 100))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    char count[128] = {0};
    if (ad_system_sql_get_brand_creative_count(count,
		json->FIND_TYPE,
		json->FIND_NAME,
		json->BRAND_ID_TYPE,
		json->BRAND_ID,
		json->STATUS,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_get_brand_creative(
	    &sql_conn,
	    json->FIND_TYPE,
	    json->FIND_NAME,
	    json->BRAND_ID_TYPE,
	    json->BRAND_ID,
	    json->STATUS,
	    json->SORT_TYPE,
	    json->SORT_COLUMN,
	    json->page_index_s,
	    json->page_index_e);
    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    char BRAND_CREATIVE_ID[128] = {0};
    while(ad_system_sql_fetch_next(&sql_conn)) {


	char BRAND_CREATIVE_ID_temp[128];
	char BRAND_CREATIVE_APPEND[4096] = {0};
	strcpy(BRAND_CREATIVE_ID_temp, ad_system_sql_get_string(&sql_conn, NULL, 2));
	if (strcmp(BRAND_CREATIVE_ID_temp, BRAND_CREATIVE_ID) == 0) {

	    max += size = lg_string_json_sprintf(buffer, ",{", "}", 3,
		    "%s", "SEQUENCE", ad_system_sql_get_string(&sql_conn, NULL, 24),
		    "%s", "IMG_PATH", ad_system_sql_get_string(&sql_conn, NULL, 21),
		    "%s", "IMG_URL", ad_system_sql_get_string(&sql_conn, NULL, 23));

	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

	} else {

	    if (*BRAND_CREATIVE_ID) {

		max += size = sprintf(buffer, "]}");
		ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    }

	    strcpy(BRAND_CREATIVE_ID, BRAND_CREATIVE_ID_temp);
	    lg_string_json_sprintf(BRAND_CREATIVE_APPEND, "[{", "}", 3,
		    "%s", "SEQUENCE", ad_system_sql_get_string(&sql_conn, NULL, 24),
		    "%s", "IMG_PATH", ad_system_sql_get_string(&sql_conn, NULL, 21),
		    "%s", "IMG_URL", ad_system_sql_get_string(&sql_conn, NULL, 23));

	    max += size = lg_string_json_sprintf(buffer, comma, ",", 17,
		    "%s", "BRAND_ID",		    ad_system_sql_get_string(&sql_conn, NULL, 1),
		    "%s", "BRAND_CREATIVE_ID",      BRAND_CREATIVE_ID,
		    "%s", "TITLE",                  ad_system_sql_get_string(&sql_conn, NULL, 3),
		    "%s", "DESCRIPTION",            ad_system_sql_get_string(&sql_conn, NULL, 4),
		    "%s", "DESTINATION_URL",        ad_system_sql_get_string(&sql_conn, NULL, 5),
		    "%s", "DISPLAY_URL",            ad_system_sql_get_string(&sql_conn, NULL, 6),
		    "%s", "TELEPHONE",              ad_system_sql_get_string(&sql_conn, NULL, 7),
		    "%s", "WEB_SITE",               ad_system_sql_get_string(&sql_conn, NULL, 8),
		    "%s", "WEB_IM",                 ad_system_sql_get_string(&sql_conn, NULL, 9),
		    "%s", "GUAHAO_URL",             ad_system_sql_get_string(&sql_conn, NULL, 10),
		    "%s", "CREATIVE_STATE",         ad_system_sql_get_string(&sql_conn, NULL, 11),
		    "%s", "REFUSE_REASON",          ad_system_sql_get_string(&sql_conn, NULL, 12),
		    "%s", "CREATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 13),
		    "%s", "LAST_UPDATE_TIME",       ad_system_sql_get_string(&sql_conn, NULL, 14),
		    "%s", "ACCOUNT_ID",       	    ad_system_sql_get_string(&sql_conn, NULL, 15),
		    "%s", "ACCOUNT_NAME",           ad_system_sql_get_string(&sql_conn, NULL, 16),
		    "%s", "AUDITOR_NAME",           ad_system_sql_get_string(&sql_conn, NULL, 17));
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

	    max += size = sprintf(buffer, "\"BRAND_CREATIVE\":%s", BRAND_CREATIVE_APPEND);
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

	    strcpy(comma, ",{");
	}
    }

    if (strcmp(comma, "{")) {

	max += size = sprintf(buffer, "]}");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_set_brand_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!(json->STATUS == 2 || json->STATUS == 3))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (ad_system_sql_set_brand_creative(&sql_conn, json->BRAND_ID, r->ad_system_account_id, json->STATUS, json->REFUSE_REASON)) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    ad_system_sql_free(&sql_conn);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_get_money_operation(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    if (json->FIND_TYPE & 1) {

	if (!(json->FIND_NAME_TYPE == 1 
		    || json->FIND_NAME_TYPE == 2 
		    || json->FIND_NAME_TYPE == 3 
		    || json->FIND_NAME_TYPE == 4 
		    || json->FIND_NAME_TYPE == 5 
		    || json->FIND_NAME_TYPE == 6 
		    || json->FIND_NAME_TYPE == 7))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    if (json->FIND_TYPE == 100)
	json->FIND_TYPE = 0;

    if (json->FIND_TYPE != 100 && json->FIND_TYPE & 4) {

	if (!(json->STATUS == 1 || json->STATUS == 2 || json->STATUS == 100))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    char count[128] = {0};
    char INVEST[128] = {0};
    char BALANCE[128] = {0};
    char CONSUMPTION[128] = {0};
    if (ad_system_sql_get_money_operation_count(count,
		INVEST,
		BALANCE,
		CONSUMPTION,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->STATUS)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_money_operation(&sql_conn,
		NULL,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->STATUS,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{"
	    "\"PAGE_INDEX\":\"%u\","
	    "\"COUNT\":\"%s\","
	    "\"INVEST\":\"%s\","
	    "\"BALANCE\":\"%s\","
	    "\"CONSUMPTION\":\"%s\""
	    "},\"LIST\":[", 
	    json->PAGE_INDEX, 
	    count,
	    INVEST,
	    BALANCE,
	    CONSUMPTION);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 8,

		"%s", "ACCOUNT_ID", 	ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "LOGIN_NAME",     ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "COMPANY_NAME",   ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "COMPANY_SITE",   ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "BALANCE",        ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "INVEST",         ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "CREATE_TIME",    ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "ROW_STATE",      ad_system_sql_get_string(&sql_conn, NULL, 8));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_account_name(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    fprintf(stderr, "---------->find_type=%d\n", json->FIND_TYPE);

    if (!(json->FIND_TYPE == 1 || json->FIND_TYPE == 2 || json->FIND_TYPE == 3 || json->FIND_TYPE == 100))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_account_name(&sql_conn, json->FIND_TYPE, json->FIND_NAME)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":["); 
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 3,

		"%s", "ACCOUNT_ID", 	ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "ACCOUNT_NAME",     ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "COMPANY_NAME",     ad_system_sql_get_string(&sql_conn, NULL, 3));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_money_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    if (json->FIND_TYPE & 1) {

	if (!(json->FIND_NAME_TYPE == 1 
		    || json->FIND_NAME_TYPE == 2 
		    || json->FIND_NAME_TYPE == 3 
		    || json->FIND_NAME_TYPE == 4 
		    || json->FIND_NAME_TYPE == 5 
		    || json->FIND_NAME_TYPE == 6 
		    || json->FIND_NAME_TYPE == 7))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    char count[128] = {0};
    if (ad_system_sql_get_money_list_count(count,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->ACCOUNT_ID)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_money_list(&sql_conn,
		json->FIND_TYPE,
		json->MONEY_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->ACCOUNT_ID,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", 
	    json->PAGE_INDEX, 
	    count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 13,
		"%s", "ACCOUNT_ID", 				ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "ACCOUNT_NAME",                           ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "COMPANY_NAME",                           ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "COMPANY_SITE",                           ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "SWIFT_NUMBER",                           ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "MONEY",                                  ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "CURRENT_MONEY_BALANCE",                  ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "INVEST",                                 ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "PAYMENT_TYPE",                           ad_system_sql_get_string(&sql_conn, NULL, 9),
		"%s", "ACCOUNTING_DOCUMENT_NUMBER",             ad_system_sql_get_string(&sql_conn, NULL, 10),
		"%s", "ACCOUNTING_DOCUMENT_FILE",               ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "ACCOUNTING_DOCUMENT_REMARK",             ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "CREATE_TIME",                            ad_system_sql_get_string(&sql_conn, NULL, 13));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_add_money_operation(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!(json->PAYMENT_TYPE == 1 || json->PAYMENT_TYPE == 2 || json->PAYMENT_TYPE == 3))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char key[128] = {0};
    lg_string_uuid(key);
    ad_system_redis_add_money_info(key,
	    json->ACCOUNT_ID,
	    json->MONEY,
	    json->PAYMENT_TYPE,
	    json->ACCOUNTING_DOCUMENT_NUMBER,
	    json->ACCOUNTING_DOCUMENT_FILE,
	    json->ACCOUNTING_DOCUMENT_REMARK,
	    json->BRAND_TYPE,
	    json->DISCOUNT);

    char buffer[1024] = {0};
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"KEY\":\"%s\",\"MONEY\":\"%s\"}", key, json->MONEY);
    fprintf(stdout, "%s\n", buffer);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_add_money_operation_confirm(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char account_id[1024] = {0};
    char money[1024] = {0};
    char payment_type[1024] = {0};
    char accounting_document_number[1024] = {0};
    char accounting_document_file[1024] = {0};
    char accounting_document_remark[1024] = {0};
    char BRAND_TYPE[128] = {0};
    char DISCOUNT[128] = {0};
    ad_system_redis_get_money_info(json->KEY,
	    account_id,
	    money,
	    payment_type,
	    accounting_document_number,
	    accounting_document_file,
	    accounting_document_remark,
	    BRAND_TYPE,
	    DISCOUNT);

    if (ad_system_sql_add_money_operation(
		account_id,
		money,
		payment_type,
		accounting_document_number,
		accounting_document_file,
		accounting_document_remark,
		r->ad_system_account_id,
		r->ad_system_account_id,
		BRAND_TYPE,
		DISCOUNT))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_batch_operation(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->BATCH_TYPE == 1)
	if (!(json->STATUS == 1 || json->STATUS == 2))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    char buffer[40960];
    char *save_ptr = NULL;
    char *id = lg_string_strtok(json->ID, buffer, &save_ptr, ',');

    while(id) {

	if (json->BATCH_TYPE == 1) {

	    if (json->ID_TYPE == 1) {

		ad_system_sql_set_manager_account_status(id, json->STATUS);
	    }

	} else if (json->BATCH_TYPE == 2) {

	    if (json->ID_TYPE == 1) {
		ad_system_sql_del_manager_account(id);

	    } else if (json->ID_TYPE == 2) {
		ad_system_sql_del_manager_announcement(id);
	    }
	}

	id = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_delivery_center(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char key[40] = {0};
    lg_string_uuid(key);
    if (ad_system_redis_add_delivery_center(key, json->ACCOUNT_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    char buffer[4048] = {0};
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"KEY\":\"%s\",\"DOMAIN\":\"%s\"}", key, get_delivery_center_domain());
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_report_money_operation(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->FIND_TYPE & 1) {

	if (!(json->FIND_NAME_TYPE == 1 
		    || json->FIND_NAME_TYPE == 2 
		    || json->FIND_NAME_TYPE == 3 
		    || json->FIND_NAME_TYPE == 4 
		    || json->FIND_NAME_TYPE == 5 
		    || json->FIND_NAME_TYPE == 6 
		    || json->FIND_NAME_TYPE == 7))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    if (json->FIND_TYPE & 4) {

	if (!(json->STATUS == 1 || json->STATUS == 2 || json->STATUS == 100))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    char file_path[1024] = {0};
    char down_path[1024] = {0};
    if (ad_system_report_path_create_r(file_path, down_path))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_money_operation(&sql_conn,
		json->ACCOUNT_ID,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->STATUS,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		0,
		0)) {

	char buffer[1024];
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    if (ad_system_report_csv_money_operation(&sql_conn, file_path)) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    ad_system_sql_free(&sql_conn);

    char buffer[2048];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=/getPic%s\"></html>", down_path);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_report_money_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->FIND_TYPE & 1) {

	if (!(json->FIND_NAME_TYPE == 1 
		    || json->FIND_NAME_TYPE == 2 
		    || json->FIND_NAME_TYPE == 3 
		    || json->FIND_NAME_TYPE == 4 
		    || json->FIND_NAME_TYPE == 5 
		    || json->FIND_NAME_TYPE == 6 
		    || json->FIND_NAME_TYPE == 7))
	    return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }

    char file_path[1024] = {0};
    char down_path[1024] = {0};
    if (ad_system_report_path_create_r(file_path, down_path))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_money_list(&sql_conn,
		json->FIND_TYPE,
		json->MONEY_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->ACCOUNT_ID,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    if (ad_system_report_csv_money_list(&sql_conn, file_path)) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);
    }

    ad_system_sql_free(&sql_conn);

    char buffer[2048];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=/getPic%s\"></html>", down_path);
    return ad_system_http_send_buffer(r, buffer, size);
}

//推广->获取科室病种CODE
int ad_system_packet_get_disease_code(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    // ngx_chain_t *send_head = NULL;
    // ngx_chain_t *send_next = NULL;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_disease_code(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    char comma[4] = "{";
    char buffer[4096000];
    size_t size = 0;//, max = 0;

    while(ad_system_sql_fetch_next(&sql_conn)) {

	size += sprintf(&buffer[size], "%s\"%s\":\"%s\"", 
		comma, 
		ad_system_sql_get_string(&sql_conn, NULL, 2),
		ad_system_sql_get_string(&sql_conn, NULL, 1));

	// ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	comma[0] = ',';
    }

    // ad_system_chain_add_r(r, &send_head, &send_next, "}", 1);
    // max++;

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
    // return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_manager_dsp_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    char count[128] = {0};
    if (ad_system_sql_get_manager_dsp_list_count(count,
		json->CPC_PRICE,
		json->BUDGET_MORE,
		json->BUDGET_LESS,
		json->ACCOUNT_NAME,
		json->REGION_SERIALIZE,
		json->DEPT_SERIALIZE,
		json->STATUS,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_manager_dsp_list(&sql_conn,
		json->CPC_PRICE,
		json->BUDGET_MORE,
		json->BUDGET_LESS,
		json->ACCOUNT_NAME,
		json->REGION_SERIALIZE,
		json->DEPT_SERIALIZE,
		json->STATUS,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 6,
		"%s", "CREATIVE_ID",	ad_system_sql_get_string(&sql_conn, NULL, 1),  
		"%s", "ACCOUNT_NAME",   ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "BUDGET",         ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "CPC_PRICE",      ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "TITLE",          ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "STATUS",         ad_system_sql_get_string(&sql_conn, NULL, 6));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_manager_dsp_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_manager_dsp(&sql_conn, json->CREATIVE_ID)) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{},\"LIST\":[");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 5,
		"%s", "IMG_ID",			ad_system_sql_get_string(&sql_conn, NULL, 1),  
		"%s", "IMG_NAME",   		ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "IMG_SIZE",   		ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "FILE_SIZE",         	ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "FILE_PATH",      	ad_system_sql_get_string(&sql_conn, NULL, 5));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_add_manager_dsp_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{


    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_ERR);

    char img_name[1024] = {0};
    char img_size[128] = {0};
    char img_buffer_size[1024] = {0};
    char img_path[1024] = {0};

    char buffer[1024];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(json->UPLOAD_ID, buffer, &save_ptr, ',');

    while(str) {

	if (ad_system_redis_get_img(str, img_name, img_size, img_buffer_size, img_path)) {

	    ad_system_sql_rollback(&sql_conn);
	    ad_system_sql_free(&sql_conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_add_manager_dsp(&sql_conn,
		    json->CREATIVE_ID,
		    img_name,//const char *CREATIVE_NAME,
		    img_size,//const char *IMG_SIZE,
		    img_buffer_size,//const char *FILE_SIZE,
		    img_path//const char *FILE_PATH
		    )) {

	    ad_system_sql_rollback(&sql_conn);
	    ad_system_sql_free(&sql_conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_del_manager_dsp_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_manager_dsp(json->IMG_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

int ad_system_packet_set_manager_dsp_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!(json->STATUS == 0 || json->STATUS == 1))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_sql_set_manager_dsp(json->CREATIVE_ID, json->STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_stdout(ngx_http_request_t *r)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    fprintf(stdout, "[recv][%04d-%02d-%02d %02d:%02d:%02d][%d][%s]%s\n",
	    tm.tm_year + 1900,
	    tm.tm_mon + 1,
	    tm.tm_mday,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
	    r->connection->fd, r->ad_system_account_id, r->ad_system_buffer);
    fflush(stdout);
    fprintf(stderr, "[recv][%04d-%02d-%02d %02d:%02d:%02d][%d][%s]%s\n",
	    tm.tm_year + 1900,
	    tm.tm_mon + 1,
	    tm.tm_mday,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
	    r->connection->fd, r->ad_system_account_id, r->ad_system_buffer);
    fflush(stderr);
}

int ad_system_packet_parse(ngx_http_request_t *r)
{

    json_object *json_root = NULL;
    if (!(json_root = json_tokener_parse(r->ad_system_buffer)))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (is_error(json_root)) {
	json_object_put(json_root);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    const char *packet_type = NULL;
    if (!(packet_type = ad_system_json_packet_head_parse(json_root))) {
	json_object_put(json_root);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    /********************/
    /**** 登录 ****/
    /********************/

    if (strcmp(packet_type, "USER_LOGIN_CHECK") == 0) {
	ad_system_packet_login_check(r);

    } else if (strcmp(packet_type, "GET_CAPTCHA") == 0) {
	ad_system_packet_send_image(r);

    } else if (strcmp(packet_type, "LOGIN") == 0) {

	ad_system_packet_stdout(r);
	const char *account = ad_system_json_packet_value_parse(json_root, "ACCOUNT");
	const char *password = ad_system_json_packet_value_parse(json_root, "PASSWORD");
	const char *var_code = ad_system_json_packet_value_parse(json_root, "VAR_CODE");
	ad_system_packet_login(r, account, password, var_code);

	//调试
    } else if (strcmp(packet_type, "lcglcg") == 0) {
	ad_system_packet_stdout(r);
	ad_system_packet_lcglcg(r);

    } else {

	//解析json
	ad_system_packet_json_data_t json_data;
	ad_system_json_packet_data_parse(json_root, &json_data);

	//获取session
	if (ngx_ad_system_get_session(r)) {
	    json_object_put(json_root);
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	//获取session匹配的account
	r->ad_system_account_id = ngx_pcalloc(r->pool, 128);
	if (ad_system_redis_get_login_session(r->ad_system_session_id, r->ad_system_account_id)) {
	    json_object_put(json_root);
	    return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_ERR);
	}

	ad_system_packet_stdout(r);

	//登出
	if (strcmp(packet_type, "LOGOUT") == 0) {
	    ad_system_packet_logout(r);

	    //首页->获取用户名
	} else if (strcmp(packet_type, "GET_TITLE_INFO") == 0) {
	    ad_system_packet_get_title_info(r);

	    /************/
	    /*创意审核*/
	    /************/
	} else if (strcmp(packet_type, "GET_TEXT_CREATIVE") == 0) {
	    ad_system_packet_get_text_creative(r, &json_data);

	} else if (strcmp(packet_type, "GET_APPEND_CREATIVE") == 0) {
	    ad_system_packet_get_append_creative(r, &json_data);

	} else if (strcmp(packet_type, "GET_IMG_CREATIVE") == 0) {
	    ad_system_packet_get_img_creative(r, &json_data);

	} else if (strcmp(packet_type, "SET_CREATIVE_BATCH") == 0) {
	    ad_system_packet_set_creative_batch(r, &json_data);

	} else if (strcmp(packet_type, "GET_MEDIA_DOMAIN_LIST") == 0) {
	    ad_system_packet_get_media_domain_list(r, &json_data);

	    /************/
	    /*报告*/
	    /************/

	} else if (strcmp(packet_type, "REPORT_MEDIA") == 0) {
	    ad_system_packet_report_media(r, &json_data);

	} else if (strcmp(packet_type, "REPORT_ACCOUNT") == 0) {
	    ad_system_packet_report_account(r, &json_data);

	} else if (strcmp(packet_type, "REPORT_STATUS") == 0) {
	    ad_system_packet_report_status(r, &json_data);

	    /************/
	    /*帐号管理*/
	    /************/

	} else if (strcmp(packet_type, "GET_REGION_CODE") == 0) {
	    ad_system_http_send_buffer(r, get_ad_system_region_code_buffer(), get_ad_system_region_code_size());

	} else if (strcmp(packet_type, "GET_MANAGER_ACCOUNT_LIST") == 0) {
	    ad_system_packet_manager_get_account_list(r, &json_data);

	} else if (strcmp(packet_type, "SET_MANAGER_ACCOUNT_INFO") == 0) {
	    ad_system_packet_manager_set_account(r, &json_data);

	} else if (strcmp(packet_type, "ADD_MANAGER_ACCOUNT") == 0) {
	    ad_system_packet_manager_add_account(r, &json_data);

	} else if (strcmp(packet_type, "DEL_MANAGER_ACCOUNT") == 0) {
	    ad_system_packet_manager_del_account(r, &json_data);

	    /************/
	    /*公告管理*/
	    /************/

	} else if (strcmp(packet_type, "ADD_MANAGER_ANNOUNCEMENT") == 0) {
	    ad_system_packet_add_manager_announcement(r, &json_data);

	} else if (strcmp(packet_type, "SET_MANAGER_ANNOUNCEMENT") == 0) {
	    ad_system_packet_set_manager_announcement(r, &json_data);

	} else if (strcmp(packet_type, "DEL_MANAGER_ANNOUNCEMENT") == 0) {
	    ad_system_packet_del_manager_announcement(r, &json_data);

	} else if (strcmp(packet_type, "GET_MANAGER_ANNOUNCEMENT") == 0) {
	    ad_system_packet_get_manager_announcement(r, &json_data);

	    /************/
	    /**品牌广告**/
	    /************/

	} else if (strcmp(packet_type, "GET_DISEASE_CODE") == 0) {
	    ad_system_packet_get_disease_code(r, &json_data);

	} else if (strcmp(packet_type, "GET_BRAND") == 0) {
	    // ad_system_packet_get_brand(r, &json_data);

	} else if (strcmp(packet_type, "GET_BRAND_UNIQUE") == 0) {
	    // ad_system_packet_get_brand_unique(r, &json_data);

	} else if (strcmp(packet_type, "ADD_BRAND") == 0) {
	    // ad_system_packet_add_brand(r, &json_data);

	} else if (strcmp(packet_type, "FIND_BRAND_SCHEDULE") == 0) {
	    // ad_system_packet_find_brand_schedule(r, &json_data);

	} else if (strcmp(packet_type, "BRAND_INSPECTION") == 0) {
	    // ad_system_packet_brand_inspection(r, &json_data);

	} else if (strcmp(packet_type, "GET_BRAND_CREATIVE") == 0) {
	    // ad_system_packet_get_brand_creative(r, &json_data);

	} else if (strcmp(packet_type, "SET_BRAND_CREATIVE") == 0) {
	    // ad_system_packet_set_brand_creative(r, &json_data);

	} else if (strcmp(packet_type, "GET_ACCOUNT_NAME") == 0) {
	    ad_system_packet_get_account_name(r, &json_data);

	    /************/
	    /*充值管理*/
	    /************/

	} else if (strcmp(packet_type, "ADD_MONEY_OPERATION") == 0) {
	    ad_system_packet_add_money_operation(r, &json_data);

	} else if (strcmp(packet_type, "ADD_MONEY_OPERATION_CONFIRM") == 0) {
	    ad_system_packet_add_money_operation_confirm(r, &json_data);

	} else if (strcmp(packet_type, "GET_MONEY_OPERATION") == 0) {
	    ad_system_packet_get_money_operation(r, &json_data);

	} else if (strcmp(packet_type, "CREATE_REPORT_MONEY_OPERATION") == 0) { 
	    ad_system_packet_report_money_operation(r, &json_data);

	    /************/
	    /*充值查询*/
	    /************/

	} else if (strcmp(packet_type, "GET_MONEY_LIST") == 0) {
	    ad_system_packet_get_money_list(r, &json_data);

	} else if (strcmp(packet_type, "CREATE_REPORT_MONEY_RECHARGE") == 0) {
	    ad_system_packet_report_money_list(r, &json_data);

	    /************/
	    /*DPS运营管理*/
	    /************/

	} else if (strcmp(packet_type, "GET_MANAGER_DSP_LIST") == 0) {
	    ad_system_packet_get_manager_dsp_list(r, &json_data);

	} else if (strcmp(packet_type, "GET_MANAGER_DSP_IMG") == 0) {
	    ad_system_packet_get_manager_dsp_img(r, &json_data);

	} else if (strcmp(packet_type, "ADD_MANAGER_DSP_IMG") == 0) {
	    ad_system_packet_add_manager_dsp_img(r, &json_data);

	} else if (strcmp(packet_type, "DEL_MANAGER_DSP_IMG") == 0) {
	    ad_system_packet_del_manager_dsp_img(r, &json_data);

	} else if (strcmp(packet_type, "SET_MANAGER_DSP_IMG") == 0) {
	    ad_system_packet_set_manager_dsp_img(r, &json_data);

	    /************/
	    /*其他*/
	    /************/

	} else if (strcmp(packet_type, "BATCH_OPERATION") == 0) {
	    ad_system_packet_batch_operation(r, &json_data);

	} else if (strcmp(packet_type, "DELIVERY_CENTER") == 0) {
	    ad_system_packet_delivery_center(r, &json_data);

	} else {
	    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    json_object_put(json_root);
    return 0;
}




