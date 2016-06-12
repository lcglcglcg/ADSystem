
#include <ngx_core.h>
#include <ngx_http.h>
#include <pthread.h>

#include "gd.h"
#include "gdfontl.h"

#include "lg_rsa.h"
#include "lg_string.h"
#include "lg_string_json.h"
#include "lg_config_file.h"

#include "ad_system_init.h"
#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_redis.h"
#include "ad_system_disease_code.h"

#include "ad_system_packet.h"
#include "ad_system_json_packet.h"
#include "ad_system_merger.h"
#include "ad_system_report.h"

//返回CODE
int ad_system_packet_send_code(ngx_http_request_t *r, ad_system_sql_t *free_sql_conn, int code)
{

    if (free_sql_conn)
	ad_system_sql_free(free_sql_conn);

    char *p_buffer = ngx_pcalloc(r->pool, 1024);
    size_t size = sprintf(p_buffer, "{\"CODE\":\"%u\"}", code);
    ngx_chain_t *buffer = ad_system_buffer_to_ngx_chain(r, p_buffer, size);
    return ad_system_http_send(r, buffer, size);
}

int ad_system_packet_lcglcg(ngx_http_request_t *r)
{
    return ad_system_packet_send_code(r, NULL, 0);
}

//检测是否登录
int ad_system_packet_login_check(ngx_http_request_t *r)
{

    if (ngx_ad_system_get_session(r))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    if (ad_system_redis_get_login_session(r->ad_system_session_id, NULL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_CHECK_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_CHECK_OK);
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
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    free(img_buffer);
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = size;
    int rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	return rc;

    return ngx_http_output_filter(r, buffer);
}

//登录
int ad_system_packet_login(ngx_http_request_t *r, const char *account, const char *password, const char *var_code)
{

    //获取session
    if (ngx_ad_system_get_session(r))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char img_result[128] = {0};
    ad_system_redis_get_var_code(r->ad_system_session_id, img_result);

    if (strcmp(var_code, img_result))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_IMG_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_login(&sql_conn, account, password))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    const char *account_id = ad_system_sql_get_string(&sql_conn, NULL, 1);
    const char *status = ad_system_sql_get_string(&sql_conn, NULL, 2);
    if (!account_id || !status)
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    if (strcmp(status, "1"))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_FROZEN);

    //更新登录时间IP地址
    ad_system_sql_set_login(account_id, ngx_ad_system_get_ipaddr(r));

    //更新session
    char session_id[128];
    lg_string_uuid(session_id);
    ad_system_redis_set_login_session(session_id, account_id);
    ngx_ad_system_set_session(r, session_id);
    return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_OK);
}

int ad_system_packet_delivery_center_login(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char account_id[128] = {0};
    if (ad_system_redis_get_delivery_center(json->KEY, account_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    //更新session
    char session_id[128];
    lg_string_uuid(session_id);
    ad_system_redis_set_login_session(session_id, account_id);
    ngx_ad_system_set_session(r, session_id);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_manager_center_login(ngx_http_request_t *r, ad_system_packet_json_data_t *json, const char *account_name, const char *password)
{


    if (ad_system_redis_get_manager_center(json->KEY))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_NOT_KEY);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_login(&sql_conn, account_name, password))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    const char *account_id = ad_system_sql_get_string(&sql_conn, NULL, 1);
    const char *status = ad_system_sql_get_string(&sql_conn, NULL, 2);
    if (!account_id || !(*account_id) || !status)
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    if (strcmp(status, "1"))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_FROZEN);

    //更新session
    char session_id[128];
    lg_string_uuid(session_id);

    ad_system_redis_set_login_session(session_id, account_id);
    ngx_ad_system_set_session(r, session_id);

    char buffer[2048];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=http://wangmeng.haomeit.com\"></html>");
    return ad_system_http_send_buffer(r, buffer, size);
}

//登出
int ad_system_packet_logout(ngx_http_request_t *r)
{
    ad_system_redis_del_login_session(r->ad_system_session_id);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//获取右上角标题
int ad_system_packet_get_title_info(ngx_http_request_t *r)
{
    ad_system_sql_t sql_conn;
    if ((ad_system_sql_get_title_info(&sql_conn, r->ad_system_account_id)))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    char buffer[40960];
    size_t size = lg_string_json_sprintf(buffer, "{", "}", 11,

	    "%s",    "CODE", "0",
	    "%s",    "USER_LOGIN_NAME", ad_system_sql_get_string(&sql_conn, NULL, 1),
	    "%s",    "COMPANY_NAME",    ad_system_sql_get_string(&sql_conn, NULL, 2),
	    "%f",    "BALANCE",         ad_system_sql_get_double(&sql_conn, NULL, 3),
	    "%f",    "INVEST",          ad_system_sql_get_double(&sql_conn, NULL, 4),
	    "%f",    "CONSUMPTION",     ad_system_sql_get_double(&sql_conn, NULL, 5),
	    "%s",    "CREATE_TIME",     ad_system_sql_get_string(&sql_conn, NULL, 6),
	    "%s",    "DOMAIN",          ad_system_sql_get_string(&sql_conn, NULL, 7),
	    "%s",    "GUAHAO_TOKEN",    ad_system_sql_get_string(&sql_conn, NULL, 8),
	    "%s",    "WEBIM",           ad_system_sql_get_string(&sql_conn, NULL, 9),
	    "%s",    "PLACE_REGION",    ad_system_sql_get_string(&sql_conn, NULL, 10));

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

//首页->获取公告
int ad_system_packet_get_announcement(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn_count;
    if (ad_system_sql_get_announcement_count(&sql_conn_count, json->FIND_TYPE == 2 ? json->FIND_NAME : NULL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn_count))
	return ad_system_packet_send_code(r, &sql_conn_count, PACKET_CODE_ERR);

    int count = ad_system_sql_get_number(&sql_conn_count, NULL, 1);
    ad_system_sql_free(&sql_conn_count);

    if (!count) {

	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{},\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    ngx_chain_t *head = NULL;
    ngx_chain_t *next = NULL;

    max += size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"COUNT\":\"%u\",\"PAGE_INDEX\":\"%u\"},\"LIST\":[", count, json->PAGE_INDEX);
    ad_system_chain_add_r(r, &head, &next, buffer, size);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_announcement(&sql_conn, 
		json->page_index_s, 
		json->page_index_e, 
		json->FIND_TYPE==2 ? json->FIND_NAME : NULL))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 3,

		"%s", "DATE", ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "TITLE", ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "CONTENT", ad_system_sql_get_string(&sql_conn, NULL, 3)
		);

	ad_system_chain_add_r(r, &head, &next, buffer, size);
	strcpy(comma, ",{");
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &head, &next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, head, max);
}

//首页->曲线图
int ad_system_packet_get_stat_trend(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!(json->DATE_TYPE == 1 || json->DATE_TYPE == 2 || json->DATE_TYPE == 3))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    /* 获取出价 */
    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_budget(&sql_conn, r->ad_system_account_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    char img_budget[128] = "0"; 
    char text_budget[128] = "0"; 
    while(ad_system_sql_fetch_next(&sql_conn)) {

	const char *name = ad_system_sql_get_string(&sql_conn, NULL, 1);
	const char *value = ad_system_sql_get_string(&sql_conn, NULL, 2);
	if (name && value) {

	    if (strcmp(name, "1") == 0)
		strcpy(img_budget, value);
	    else if (strcmp(name, "2") == 0)
		strcpy(text_budget, value);
	}    
    }

    ad_system_sql_free(&sql_conn);

    /* 获取 展现 点击 消费 */

    ad_system_mongo_stat_t img_data;
    ad_system_mongo_stat_t text_data;
    switch(json->DATE_TYPE) {

	case 1:
	    ad_system_mongo_stat_load(&img_data, ACCOUNT_DAY_TEMP, 1, r->ad_system_account_id);
	    ad_system_mongo_stat_load(&text_data, ACCOUNT_DAY_TEMP, 2, r->ad_system_account_id);
	    break;

	case 2:
	    ad_system_mongo_stat_load(&img_data, ACCOUNT_YESTERDAY_TEMP, 1, r->ad_system_account_id);
	    ad_system_mongo_stat_load(&text_data, ACCOUNT_YESTERDAY_TEMP, 2, r->ad_system_account_id);
	    break;

	case 3:
	    ad_system_mongo_stat_load(&img_data, ACCOUNT_SEVENDAY_TEMP, 1, r->ad_system_account_id);
	    ad_system_mongo_stat_load(&text_data, ACCOUNT_SEVENDAY_TEMP, 2, r->ad_system_account_id);
	    break;
    }

    char buffer[4096];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\","
	    "\"ALL\":{"
	    "\"IMG_BUDGET\":\"%s\","
	    "\"TEXT_BUDGET\":\"%s\","
	    "\"IMPRESSION\":\"%u\","
	    "\"CLICK\":\"%u\","
	    "\"COST\":\"%0.2f\"},"
	    "\"TEXT\":{"
	    "\"IMPRESSION\":\"%u\","
	    "\"CLICK\":\"%u\","
	    "\"COST\":\"%0.2f\"},"
	    "\"IMG\":{\"IMPRESSION\":\"%u\","
	    "\"CLICK\":\"%u\","
	    "\"COST\":\"%0.2f\"}}",

	    img_budget,
	    text_budget,
	    text_data.impression + img_data.impression,
	    text_data.click + img_data.click,
	    text_data.cost + img_data.cost,
	    text_data.impression,
	    text_data.click,
	    text_data.cost,
	    img_data.impression,
	    img_data.click,
	    img_data.cost);

    return ad_system_http_send_buffer(r, buffer, size);
}

//首页->曲线图->TEXT
int ad_system_packet_get_stat_trend_text(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    lg_list_t head;
    ad_system_mongo_stat_list_load(&head, ACCOUNT_SEVENDAY_DAILY_TEMP, 2, r->ad_system_account_id, "time");


    char buffer[4096], srv_time[128];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"TIME\":\"%s\",\"LIST\":[", lg_string_get_time_string(srv_time));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    size_t max = size;

    char comma[4] = "";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &head) { 

	ad_system_mongo_stat_t *node = (ad_system_mongo_stat_t *)p;

	double ctr = 0;
	if (node->click && node->impression)
	    ctr = (double)node->click / (double)node->impression;
	double cpc = 0;
	if (node->cost && node->click)
	    cpc = node->cost / (double)node->click;
	double cpm = 0;
	if (node->cost && node->impression)
	    cpm = node->cost / (double)node->impression * (double)1000;

	max += size = sprintf(buffer, "%s{"
		"\"DATE\":\"%s\","
		"\"IMPRESSION\":\"%u\","
		"\"CLICK\":\"%u\","
		"\"COST\":\"%0.2f\","
		"\"CTR\":\"%0.2f\","
		"\"CPC\":\"%0.2f\","
		"\"CPM\":\"%0.2f\"}",

		comma,
		node->name,
		node->impression,
		node->click,
		node->cost,
		ctr * 100,
		cpc,
		cpm);

	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	comma[0] = ','; 
    }    

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_mongo_list_free(&head);
    return ad_system_http_send(r, send_head, max);
}

//首页->曲线图->IMG
int ad_system_packet_get_stat_trend_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    lg_list_t head;
    ad_system_mongo_stat_list_load(&head, ACCOUNT_SEVENDAY_DAILY_TEMP, 1, r->ad_system_account_id, "time");

    char buffer[4096],srv_time[128];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"TIME\":\"%s\",\"LIST\":[", lg_string_get_time_string(srv_time));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    size_t max = size;

    char comma[4] = "";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &head) { 

	ad_system_mongo_stat_t *node = (ad_system_mongo_stat_t *)p;

	double ctr = 0;
	if (node->click && node->impression)
	    ctr = (double)node->click / (double)node->impression;
	double cpc = 0;
	if (node->cost && node->click)
	    cpc = node->cost / (double)node->click;
	double cpm = 0;
	if (node->cost && node->impression)
	    cpm = node->cost / (double)node->impression * (double)1000;

	max += size = sprintf(buffer, "%s{"
		"\"DATE\":\"%s\","
		"\"IMPRESSION\":\"%u\","
		"\"CLICK\":\"%u\","
		"\"COST\":\"%0.2f\","
		"\"CTR\":\"%0.2f\","
		"\"CPC\":\"%0.2f\","
		"\"CPM\":\"%0.2f\"}",

		comma,
		node->name,
		node->impression,
		node->click,
		node->cost,
		ctr * 100,
		cpc,
		cpm);
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

	comma[0] = ','; 
    }    

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_mongo_list_free(&head);
    return ad_system_http_send(r, send_head, max);
}

//推广->获取科室病种CODE
int ad_system_packet_get_disease_code(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_disease_code(&sql_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    char comma[4] = "{";
    char buffer[40960];
    size_t size, max = 0;

    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = sprintf(buffer, "%s\"%s\":\"%s\"", 
		comma, 
		ad_system_sql_get_string(&sql_conn, NULL, 2),
		ad_system_sql_get_string(&sql_conn, NULL, 1));

	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	comma[0] = ',';
    }

    ad_system_chain_add_r(r, &send_head, &send_next, "}", 1);
    max++;

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

//推广->左侧列表
int ad_system_packet_get_sperad_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (ad_system_oci_get_sperad_list(&sql_conn, r->ad_system_account_id, json->DISPLAY_STATUS))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"CAMPAIGN_LIST\":[");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = {0};
    char comma_group[4] = {0};
    const char *CAMPAIGN_ID = NULL;
    char LAST_CAMPAIGN_ID[1024] = {0};
    while(ad_system_sql_fetch_next(&sql_conn)) {

	CAMPAIGN_ID = ad_system_sql_get_string(&sql_conn, NULL, 1);
	const char *CAMPAIGN_NAME = ad_system_sql_get_string(&sql_conn, NULL, 2);
	int CAMPAIGN_STATUS = ad_system_sql_get_number(&sql_conn, NULL, 3);
	const char *GROUP_ID = ad_system_sql_get_string(&sql_conn, NULL, 4);
	const char *GROUP_NAME = ad_system_sql_get_string(&sql_conn, NULL, 5);

	int GROUP_STATUS = 0;
	if (CAMPAIGN_STATUS == 1) {

	    // CAMPAIGN_STATUS = 2;
	    GROUP_STATUS = 2;

	} else {

	    // CAMPAIGN_STATUS = 1;
	    GROUP_STATUS = ad_system_sql_get_number(&sql_conn, NULL, 6);
	    if (GROUP_STATUS == 21)
		GROUP_STATUS = 1;
	    else if (GROUP_STATUS == 22)
		GROUP_STATUS = 2;
	}

	if (strcmp(LAST_CAMPAIGN_ID, CAMPAIGN_ID) != 0) {

	    strcpy(LAST_CAMPAIGN_ID, CAMPAIGN_ID);
	    max += size = sprintf(buffer, "%s{\"CAMPAIGN_ID\":\"%s\",\"CAMPAIGN_NAME\":\"%s\",\"CAMPAIGN_STATUS\":\"%u\",\"GROUP_LIST\":[",
		    comma,
		    CAMPAIGN_ID,
		    CAMPAIGN_NAME,
		    CAMPAIGN_STATUS);
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    comma_group[0] = 0;
	}

	if (GROUP_ID && *GROUP_ID) {

	    max += size = sprintf(buffer, "%s{\"GROUP_ID\":\"%s\", \"GROUP_NAME\":\"%s\", \"GROUP_STATUS\":\"%u\"}",
		    comma_group,
		    GROUP_ID,
		    GROUP_NAME,
		    GROUP_STATUS);
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	}

	strcpy(comma, "]},");
	comma_group[0] = ',';
    }

    if (CAMPAIGN_ID) {

	max += 2;
	ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);
    }

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);
    return ad_system_http_send(r, send_head, max);
}

//计划重命名验证
int ad_system_packet_campaign_name_check(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int ret = ad_system_sql_campaign_name_check(r->ad_system_account_id, json->CAMPAIGN_NAME, json->DISPLAY_STATUS);
    if (ret == -1) 
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    else if (ret != 100)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_NAME_REPEAT);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//推广->获取计划
int ad_system_packet_get_campaign(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_campaign(&sql_conn,
		r->ad_system_account_id, 
		json->DISPLAY_STATUS,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    //获取mongo数据
    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, CAMPAIGN_DAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "campaign_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, CAMPAIGN_YESTERDAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "campaign_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, CAMPAIGN_SEVENDAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "campaign_id");
    else
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    //合并数据
    lg_list_t merger_head;
    ad_system_merger_campaign_info_t info;
    ad_system_merger_campaign(
	    &sql_conn, 
	    &mongo_head, 
	    json,
	    &merger_head,
	    &info);

    //发送数据
    max = size = sprintf(buffer, "{\"CODE\":\"0\","
	    "\"INFO\":{"
	    "\"PAGE_INDEX\":\"%u\","
	    "\"CAMPAIGN_COUNT\":\"%u\","
	    "\"CAMPAIGN_ENABLED\":\"%u\","
	    "\"CAMPAIGN_BUDGET\":\"%0.2f\","
	    "\"CAMPAIGN_IMPRESSION\":\"%u\","
	    "\"CAMPAIGN_CLICK\":\"%u\","
	    "\"CAMPAIGN_CTR\":\"%0.2f%%\","
	    "\"CAMPAIGN_CPC\":\"%0.2f\","
	    "\"CAMPAIGN_COST\":\"%0.2f\","
	    "\"CAMPAIGN_CPM\":\"%0.2f\""
	    "},\"LIST\":[",

	    json->PAGE_INDEX,
	    info.CAMPAIGN_COUNT,
	    info.CAMPAIGN_ENABLED,
	    info.CAMPAIGN_BUDGET,
	    info.CAMPAIGN_IMPRESSION,
	    info.CAMPAIGN_CLICK,
	    info.CAMPAIGN_CTR * 100,
	    info.CAMPAIGN_CPC,
	    info.CAMPAIGN_COST,
	    info.CAMPAIGN_CPM);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    int page_index = 1;
    char comma[4] = "{";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_campaign_t *node = NULL;
	node = (ad_system_merger_campaign_t *)p;

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_string_json_sprintf(buffer, comma, "}", 17,

		    "%s", 		       "CAMPAIGN_ID",          node->CAMPAIGN_ID,
		    "%s", 		       "CAMPAIGN_NAME",        node->CAMPAIGN_NAME,
		    "%d", 		       "CAMPAIGN_STATUS",      node->CAMPAIGN_STATE,
		    "%f", 		       "CAMPAIGN_BUDGET",      node->CAMPAIGN_BUDGET,
		    "%d", 		       "SCHEDULE_TYPE",        node->SCHEDULE_TYPE,
		    "%s", 		       "SCHEDULE_SERIALIZE",   node->SCHEDULE_SERIALIZE,
		    "%d", 		       "REGION_TYPE",          node->REGION_TYPE,
		    "%s", 		       "REGION_SERIALIZE",     node->REGION_SERIALIZE,
		    "%d", 		       "TIME_TYPE",            node->TIME_TYPE,
		    "%s", 		       "BEGIN_TIME",           node->BEGIN_TIME,
		    "%s", 		       "END_TIME",             node->END_TIME,
		    "%d", 		       "CAMPAIGN_IMPRESSION",  node->CAMPAIGN_IMPRESSION,
		    "%d", 		       "CAMPAIGN_CLICK",       node->CAMPAIGN_CLICK,
		    "%e", 		       "CAMPAIGN_CTR",         node->CAMPAIGN_CTR,
		    "%f", 		       "CAMPAIGN_CPC",         node->CAMPAIGN_CPC,
		    "%f", 		       "CAMPAIGN_COST",        node->CAMPAIGN_COST,
		    "%f", 		       "CAMPAIGN_CPM",         node->CAMPAIGN_CPM);

	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    strcpy(comma, ",{");
	}

	page_index++;
    }

    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);
    max += 2;

    ad_system_sql_free(&sql_conn);
    ad_system_merger_free(&merger_head);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_set_campaign(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->UPDATE_TYPE & 2) {

	double budget_number = atof(json->CAMPAIGN_BUDGET);
	if (budget_number < 50)
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_BUDGET_ERR);

	char cpc_price[128] = {0}, cpm_price[128] = {0};
	if (ad_system_sql_get_group_price(json->CAMPAIGN_ID, cpc_price, cpm_price))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	double cpc_price_number = atof(cpc_price);
	double cpm_price_number = atof(cpm_price);
	if (budget_number < cpc_price_number || budget_number < cpm_price_number)
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);
    }


    if (json->REGION_TYPE == 2) {

    }

    if (ad_system_sql_set_campaign(r->ad_system_account_id,
		json->CAMPAIGN_ID,
		json->UPDATE_TYPE,
		json->CAMPAIGN_STATUS,
		json->CAMPAIGN_BUDGET,
		json->REGION_TYPE,
		json->REGION_SERIALIZE,
		json->SCHEDULE_TYPE,
		json->SCHEDULE_SERIALIZE,
		json->TIME_TYPE,
		json->BEGIN_TIME,
		json->END_TIME,
		json->CAMPAIGN_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//推广->添加计划
int ad_system_packet_add_campaign(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int ret = ad_system_sql_campaign_name_check(r->ad_system_account_id, json->CAMPAIGN_NAME, json->DISPLAY_STATUS);
    if (ret == -1) 
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    else if (ret != 100)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_NAME_REPEAT);

    if (atof(json->CAMPAIGN_BUDGET) < 50)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_BUDGET_ERR);

    char campaign_id[1024] = {0};
    if (ad_system_sql_get_campaign_id(campaign_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (ad_system_sql_add_campaign(
		r->ad_system_account_id,
		json->DISPLAY_STATUS,
		campaign_id,
		json->CAMPAIGN_NAME,
		json->CAMPAIGN_BUDGET,
		json->REGION_TYPE,
		json->REGION_SERIALIZE,
		json->SCHEDULE_TYPE,
		json->SCHEDULE_SERIALIZE,
		json->TIME_TYPE,
		json->BEGIN_TIME,
		json->END_TIME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    char buffer[4096];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"CAMPAIGN_ID\":\"%s\"}", campaign_id);
    return ad_system_http_send_buffer(r, buffer, size);
}

//删除计划
int ad_system_packet_del_campaign(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_campaign(json->CAMPAIGN_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//推广->获取计划(单个)
int ad_system_packet_get_campaign_unique(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_campaign_unique(&sql_conn, json->CAMPAIGN_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    ad_system_merger_campaign_t temp;
    ad_system_merger_campaign_t *node = &temp;
    memset(&temp, 0, sizeof(ad_system_merger_campaign_t));
    sprintf(temp.CAMPAIGN_ID, "%s",  ad_system_sql_get_string(&sql_conn, NULL, 1));
    sprintf(temp.CAMPAIGN_NAME, "%s", ad_system_sql_get_string(&sql_conn, NULL, 2));
    int CAMPAIGN_STATE = ad_system_sql_get_number(&sql_conn, NULL, 3);
    int PAUSE = ad_system_sql_get_number(&sql_conn, NULL, 4);
    temp.CAMPAIGN_BUDGET = ad_system_sql_get_double(&sql_conn, NULL, 5);
    temp.SCHEDULE_TYPE = ad_system_sql_get_number(&sql_conn, NULL, 6);
    sprintf(temp.SCHEDULE_SERIALIZE, "%s", ad_system_sql_get_string(&sql_conn, NULL, 7));
    const char *SCHEDULE_SYS_TIME = ad_system_sql_get_string(&sql_conn, NULL, 8);
    temp.REGION_TYPE = ad_system_sql_get_number(&sql_conn, NULL, 9);
    sprintf(temp.REGION_SERIALIZE, "%s", ad_system_sql_get_string(&sql_conn, NULL, 10));
    sprintf(temp.BEGIN_TIME, "%s", ad_system_sql_get_string(&sql_conn, NULL, 11));
    sprintf(temp.END_TIME, "%s", ad_system_sql_get_string(&sql_conn, NULL, 12));
    int DATE_SUB_BEGIN_TIME = ad_system_sql_get_number(&sql_conn, NULL, 13);
    int DATE_SUB_END_TIME = ad_system_sql_get_number(&sql_conn, NULL, 14);

    if (PAUSE == 2) {
	temp.CAMPAIGN_STATE = 5;//暂停

    } else if (DATE_SUB_BEGIN_TIME == 2) {
	temp.CAMPAIGN_STATE = 4;//未开始

    } else if (DATE_SUB_END_TIME == 2) {
	temp.CAMPAIGN_STATE = 6;//下线

    } else if (CAMPAIGN_STATE == 13) {
	temp.CAMPAIGN_STATE = 3;//预算不足

    } else if (temp.SCHEDULE_TYPE == 2 
	    && ad_system_merger_schedule(SCHEDULE_SYS_TIME, temp.SCHEDULE_SERIALIZE) == 0) {
	temp.CAMPAIGN_STATE = 2;//暂停时段

    } else {
	temp.CAMPAIGN_STATE = 1;//有效
    }

    //结束时间类型
    if (*temp.END_TIME) temp.TIME_TYPE = 2;
    else temp.TIME_TYPE = 1;

    //发送数据
    char buffer[40900];
    size_t size = lg_string_json_sprintf(buffer, "{", "}", 12,
	    "%s", "CODE", "0",
	    "%s", "CAMPAIGN_ID",          node->CAMPAIGN_ID,
	    "%s", "CAMPAIGN_NAME",        node->CAMPAIGN_NAME,
	    "%d", "CAMPAIGN_STATUS",      node->CAMPAIGN_STATE,
	    "%f", "CAMPAIGN_BUDGET",      node->CAMPAIGN_BUDGET,
	    "%d", "SCHEDULE_TYPE",        node->SCHEDULE_TYPE,
	    "%s", "SCHEDULE_SERIALIZE",   node->SCHEDULE_SERIALIZE,
	    "%d", "REGION_TYPE",          node->REGION_TYPE,
	    "%s", "REGION_SERIALIZE",     node->REGION_SERIALIZE,
	    "%d", "TIME_TYPE",            node->TIME_TYPE,
	    "%s", "BEGIN_TIME",           node->BEGIN_TIME,
	    "%s", "END_TIME",             node->END_TIME);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

//组重命名验证
int ad_system_packet_group_name_check(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int ret = ad_system_sql_group_name_check(json->CAMPAIGN_ID, json->DISPLAY_STATUS, json->GROUP_NAME);
    if (ret == -1) 
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    else if (ret != 100)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_NAME_REPEAT);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_group(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[409600];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_group(&sql_conn,
		r->ad_system_account_id, 
		json->DISPLAY_STATUS,
		json->FIND_TYPE,
		json->CAMPAIGN_ID,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, GROUP_DAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "group_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, GROUP_YESTERDAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "group_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, GROUP_SEVENDAY_TEMP, json->DISPLAY_STATUS, r->ad_system_account_id, "group_id");
    else
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    lg_list_t merger_head;
    ad_system_merger_group_info_t info;
    ad_system_merger_group(
	    &sql_conn,
	    &mongo_head,
	    json,
	    &merger_head,
	    &info);

    max = size = sprintf(buffer, "{\"CODE\":\"0\","
	    "\"INFO\":{"
	    "\"PAGE_INDEX\":\"%u\","
	    "\"GROUP_COUNT\":\"%u\","
	    "\"GROUP_ENABLED\":\"%u\","
	    "\"GROUP_IMPRESSION\":\"%u\","
	    "\"GROUP_CLICK\":\"%u\","
	    "\"GROUP_CTR\":\"%0.2f%%\","
	    "\"GROUP_CPC\":\"%0.2f\","
	    "\"GROUP_COST\":\"%0.2f\","
	    "\"GROUP_CPM\":\"%0.2f\""
	    "},\"LIST\":[",

	    json->PAGE_INDEX,
	    info.GROUP_COUNT,
	    info.GROUP_ENABLED,
	    info.GROUP_IMPRESSION,
	    info.GROUP_CLICK,
	    info.GROUP_CTR * 100,
	    info.GROUP_CPC,
	    info.GROUP_COST,
	    info.GROUP_CPM);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    int page_index = 1;
    char comma[4] = "{"; 
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_group_t *node = NULL;
	node = (ad_system_merger_group_t *)p; 

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_string_json_sprintf(buffer, comma, "}", 15,

		    "%s", "CAMPAIGN_ID",         node->CAMPAIGN_ID,
		    "%s", "CAMPAIGN_NAME",       node->CAMPAIGN_NAME,
		    "%s", "GROUP_ID",            node->GROUP_ID,
		    "%s", "GROUP_NAME",          node->GROUP_NAME,
		    "%u", "GROUP_STATUS",        node->GROUP_STATE,
		    "%f", "GROUP_CPC_PRICE",     node->GROUP_CPC_PRICE,
		    "%f", "GROUP_CPM_PRICE",     node->GROUP_CPM_PRICE,
		    "%u", "GROUP_TARGETING_TYPE",node->GROUP_TARGETING_TYPE,
		    "%s", "GROUP_DISEASE",       node->GROUP_DISEASE,
		    "%u", "GROUP_IMPRESSION",    node->GROUP_IMPRESSION,
		    "%u", "GROUP_CLICK",         node->GROUP_CLICK,
		    "%e", "GROUP_CTR",           node->GROUP_CTR,
		    "%f", "GROUP_CPC",           node->GROUP_CPC,
		    "%f", "GROUP_COST",          node->GROUP_COST,
		    "%f", "GROUP_CPM",           node->GROUP_CPM);

	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    strcpy(comma, ",{");
	}

	page_index++;
    }

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);

    ad_system_sql_free(&sql_conn);
    ad_system_merger_free(&merger_head);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_add_group_keyword(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->GROUP_ID || !json->GROUP_KEYWORD)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    char buffer[40960];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(json->GROUP_KEYWORD, buffer, &save_ptr, ',');

    while(str) {

	ad_system_sql_add_group_keyword(&sql_conn, r->ad_system_account_id, json->GROUP_ID, str);
	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    ad_system_sql_group_synch_state(r->ad_system_account_id, json->GROUP_ID, NULL);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_group_keyword(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;

    if (!(json->DISPLAY_STATUS == 1 || json->DISPLAY_STATUS == 2))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    if (!json->page_index_s || !json->page_index_e)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    ad_system_sql_t sql_count_conn;
    if (ad_system_sql_get_group_keyword_count(&sql_count_conn, r->ad_system_account_id, 
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		json->GROUP_ID,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_count_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%d\",\"COUNT\":\"%s\"},\"LIST\":[",
	    json->PAGE_INDEX, 
	    ad_system_sql_get_string(&sql_count_conn, NULL, 1));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_count_conn);
    ad_system_sql_t sql_conn;

    if (ad_system_sql_get_group_keyword(&sql_conn, r->ad_system_account_id,
		json->page_index_s,
		json->page_index_e,
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		json->GROUP_ID,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME) == 0) {


	char comma[4] = "{";
	while(ad_system_sql_fetch_next(&sql_conn)) {

	    max += size = lg_string_json_sprintf(buffer, comma, "}", 8,

		    "%s", "GROUP_KEYWORD_ID", ad_system_sql_get_string(&sql_conn, NULL, 2),
		    "%s", "GROUP_KEYWORD",    ad_system_sql_get_string(&sql_conn, NULL, 3),
		    "%s", "CAMPAIGN_ID",      ad_system_sql_get_string(&sql_conn, NULL, 4),
		    "%s", "CAMPAIGN_NAME",    ad_system_sql_get_string(&sql_conn, NULL, 5),
		    "%s", "GROUP_ID",         ad_system_sql_get_string(&sql_conn, NULL, 6),
		    "%s", "GROUP_NAME",       ad_system_sql_get_string(&sql_conn, NULL, 7),
		    "%f", "GROUP_CPC_PRICE",  ad_system_sql_get_double(&sql_conn, NULL, 8),
		    "%f", "GROUP_CPM_PRICE",  ad_system_sql_get_double(&sql_conn, NULL, 9));
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    strcpy(comma, ",{");
	}

#if 0
	char comma[4] = {0};
	while(ad_system_sql_fetch_next(&sql_conn)) {

	    max += size = sprintf(buffer, "%s{"
		    "\"GROUP_KEYWORD_ID\":\"%s\","
		    "\"GROUP_KEYWORD\":\"%s\","
		    "\"CAMPAIGN_ID\":\"%s\","
		    "\"CAMPAIGN_NAME\":\"%s\","
		    "\"GROUP_ID\":\"%s\","
		    "\"GROUP_NAME\":\"%s\","
		    "\"GROUP_CPC_PRICE\":\"%0.2f\","
		    "\"GROUP_CPM_PRICE\":\"%0.2f\"}",
		    comma,
		    ad_system_sql_get_string(&sql_conn, NULL, 2),
		    ad_system_sql_get_string(&sql_conn, NULL, 3),
		    ad_system_sql_get_string(&sql_conn, NULL, 4),
		    ad_system_sql_get_string(&sql_conn, NULL, 5),
		    ad_system_sql_get_string(&sql_conn, NULL, 6),
		    ad_system_sql_get_string(&sql_conn, NULL, 7),
		    ad_system_sql_get_double(&sql_conn, NULL, 8),
		    ad_system_sql_get_double(&sql_conn, NULL, 9));

	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    comma[0] = ',';
	}
#endif
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_get_report_group_keyword(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!(json->DISPLAY_STATUS == 1 || json->DISPLAY_STATUS == 2))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    if (!json->page_index_s || !json->page_index_e)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char file_path[1024] = {0};
    char down_path[1024] = {0};
    if (ad_system_report_path_create_r(file_path, down_path))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_group_keyword_report(&sql_conn, r->ad_system_account_id,
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		json->GROUP_ID,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    ad_system_report_csv_group_keyword(&sql_conn, file_path);
    ad_system_sql_free(&sql_conn);

    char buffer[2048];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=/getPic%s\"></html>", down_path);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_del_group_keyword(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->GROUP_KEYWORD_ID)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);


    char buffer[40960];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(json->GROUP_KEYWORD_ID, buffer, &save_ptr, ',');

    while(str) {

	ad_system_sql_group_synch_state(r->ad_system_account_id, NULL, str);
	ad_system_sql_del_group_keyword(str);
	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_set_group(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    int disease_code_type = 0;
    if (ad_system_sql_get_disease_code_type(r->ad_system_account_id, &disease_code_type))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_ERR);

    fprintf(stdout, "disease_code_type=%d\n", disease_code_type);

    char disease_code[409600] = {0};
    if (json->UPDATE_TYPE & 4 && json->GROUP_TARGETING_TYPE == 1) {
	if (disease_code_type) {
	    ad_system_disease_code_perse(disease_code, json->GROUP_DISEASE);
	} else {
	    ad_system_disease_code_perse_r(disease_code, json->GROUP_DISEASE);
	}
    }

    ad_system_sql_t sql_budget_and_bidding;
    if (ad_system_sql_get_budget_and_bidding(&sql_budget_and_bidding, r->ad_system_account_id, json->GROUP_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    if (!ad_system_sql_fetch_next(&sql_budget_and_bidding))
	return ad_system_packet_send_code(r, &sql_budget_and_bidding, PACKET_CODE_USER_LOGIN_ERR);

    double CAMPAIGN_BUDGET = ad_system_sql_get_double(&sql_budget_and_bidding, NULL, 1);
    int GROUP_BIDDING_MODEL = ad_system_sql_get_number(&sql_budget_and_bidding, NULL, 2);
    ad_system_sql_free(&sql_budget_and_bidding);

    if (json->UPDATE_TYPE & 2) {

	if (strcmp(json->GROUP_CPC_PRICE, "null") == 0)
	    if (!(GROUP_BIDDING_MODEL & 1))
		return ad_system_packet_send_code(r, NULL, PACKET_CODE_PIRCE_ERR);

	if (atof(json->GROUP_CPC_PRICE) > CAMPAIGN_BUDGET)
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);
    }

    if (json->UPDATE_TYPE & 16) {

	if (strcmp(json->GROUP_CPM_PRICE, "null") == 0)
	    if (!(GROUP_BIDDING_MODEL & 2))
		return ad_system_packet_send_code(r, NULL, PACKET_CODE_PIRCE_ERR);

	if (atof(json->GROUP_CPM_PRICE) > CAMPAIGN_BUDGET * 1000)
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);
    }

    if (ad_system_sql_set_group(r->ad_system_account_id,
		json->GROUP_ID,
		json->UPDATE_TYPE,
		json->STATUS,
		json->GROUP_CPC_PRICE,
		json->GROUP_CPM_PRICE,
		GROUP_BIDDING_MODEL,
		json->GROUP_TARGETING_TYPE,
		json->GROUP_NAME))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (json->UPDATE_TYPE & 4)
	if (ad_system_sql_set_group_targeting(
		    r->ad_system_account_id,
		    json->GROUP_ID,
		    json->UPDATE_TYPE, 
		    json->GROUP_TARGETING_TYPE, 
		    disease_code, 
		    json->GROUP_KEYWORD))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_add_group(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char CAMPAIGN_BUDGET[128] = {0}; 
    if (ad_system_sql_get_campaign_budget(r->ad_system_account_id, json->CAMPAIGN_ID, NULL, CAMPAIGN_BUDGET, NULL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (json->GROUP_PRICE_TYPE & 1)
	if (atof(json->GROUP_CPM_PRICE) > atof(CAMPAIGN_BUDGET))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);
    if (json->GROUP_PRICE_TYPE & 2)
	if (atof(json->GROUP_CPC_PRICE) > atof(CAMPAIGN_BUDGET))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);
    
    int disease_code_type = 0;
    if (ad_system_sql_get_disease_code_type(r->ad_system_account_id, &disease_code_type))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_ERR);

    char disease_code[409600] = {0};
    if (json->GROUP_TARGETING_TYPE == 1) {
    if (json->UPDATE_TYPE & 4 && json->GROUP_TARGETING_TYPE == 1) {
	if (disease_code_type) {
	    ad_system_disease_code_perse(disease_code, json->GROUP_DISEASE);
	} else {
	    ad_system_disease_code_perse_r(disease_code, json->GROUP_DISEASE);
	}
    }
    }

    int ret = ad_system_sql_group_name_check(json->CAMPAIGN_ID, json->DISPLAY_STATUS, json->GROUP_NAME);
    if (ret == -1) 
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    else if (ret != 100)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_NAME_REPEAT);

    char group_id[1024] = {0};
    if (ad_system_sql_get_group_id(group_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (json->DISPLAY_STATUS == 2)//文字没有CPM
	json->GROUP_PRICE_TYPE = 2;

    if (ad_system_sql_add_group(
		r->ad_system_account_id,
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		group_id,
		json->GROUP_NAME,
		json->GROUP_PRICE_TYPE,
		json->GROUP_CPC_PRICE,
		json->GROUP_CPM_PRICE,
		json->GROUP_TARGETING_TYPE))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (ad_system_sql_add_group_targeting(
		r->ad_system_account_id,
		json->CAMPAIGN_ID,
		group_id,
		json->GROUP_TARGETING_TYPE,
		disease_code,
		json->GROUP_KEYWORD))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    char buffer[40960];
    if (json->GROUP_TARGETING_TYPE && json->GROUP_TARGETING_TYPE == 2) {

	if (!(json->GROUP_KEYWORD && *json->GROUP_KEYWORD))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

	ad_system_sql_t sql_keyword_conn;
	if (ad_system_sql_connect(&sql_keyword_conn))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	char *save_ptr = NULL;
	char *str = lg_string_strtok(json->GROUP_KEYWORD, buffer, &save_ptr, ',');

	while(str) {

	    ad_system_sql_add_group_keyword(&sql_keyword_conn, r->ad_system_account_id, group_id, str);
	    str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
	}

	ad_system_sql_commit(&sql_keyword_conn);
	ad_system_sql_free(&sql_keyword_conn);
	ad_system_sql_group_synch_state(r->ad_system_account_id, json->GROUP_ID, NULL);
    }

    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"GROUP_ID\":\"%s\"}", group_id);
    return ad_system_http_send_buffer(r, buffer, size);
}

//删除组
int  ad_system_packet_del_group(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_group(json->GROUP_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_group_unique(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_group_unique(&sql_conn, json->GROUP_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    const char *CAMPAIGN_ID = ad_system_sql_get_string(&sql_conn, NULL, 1);
    const char *CAMPAIGN_NAME = ad_system_sql_get_string(&sql_conn, NULL, 2);
    const char *GROUP_ID = ad_system_sql_get_string(&sql_conn, NULL, 3);
    const char *GROUP_NAME = ad_system_sql_get_string(&sql_conn, NULL, 4);
    double GROUP_CPC_PRICE = ad_system_sql_get_double(&sql_conn, NULL, 5);
    double GROUP_CPM_PRICE = ad_system_sql_get_double(&sql_conn, NULL, 6);
    int GROUP_STATE = ad_system_sql_get_number(&sql_conn, NULL, 7);
    int CAMPAIGN_PAUSE = ad_system_sql_get_number(&sql_conn, NULL, 8);
    int CAMPAIGN_STATE = ad_system_sql_get_number(&sql_conn, NULL, 9);
    int SCHEDULE_TYPE = ad_system_sql_get_number(&sql_conn, NULL, 10);
    const char *SCHEDULE_SYS_TIME = ad_system_sql_get_string(&sql_conn, NULL, 11);
    const char *SCHEDULE_SERIALIZE = ad_system_sql_get_string(&sql_conn, NULL, 12);
    int DATE_SUB_BEGIN_TIME = ad_system_sql_get_number(&sql_conn, NULL, 13);
    int DATE_SUB_END_TIME = ad_system_sql_get_number(&sql_conn, NULL, 14);
    int GROUP_TARGETING_TYPE = ad_system_sql_get_number(&sql_conn, NULL, 15);
    const char *GROUP_DISEASE = ad_system_sql_get_string(&sql_conn, NULL, 16);

    if (GROUP_STATE == 2) {
	GROUP_STATE = 8;//组暂停

    } else if (CAMPAIGN_PAUSE == 2) {
	GROUP_STATE = 5;//计划暂停

    } else if (DATE_SUB_BEGIN_TIME == 2) {
	GROUP_STATE = 4;//未开始

    } else if (DATE_SUB_END_TIME == 2) {
	GROUP_STATE = 6;//下线

    } else if (CAMPAIGN_STATE == 13) {
	GROUP_STATE = 3;//预算不足

    } else if (SCHEDULE_TYPE == 2 
	    && ad_system_merger_schedule(SCHEDULE_SYS_TIME, SCHEDULE_SERIALIZE) == 0) {
	GROUP_STATE = 2;//暂停时段

    } else {
	GROUP_STATE = 1;//有效
    }

    char buffer[40960];
    size_t size = lg_string_json_sprintf(buffer, "{", "}", 10,

	    "%s", "CODE", "0",
	    "%s", "CAMPAIGN_ID",                CAMPAIGN_ID,
	    "%s", "CAMPAIGN_NAME",              CAMPAIGN_NAME,
	    "%s", "GROUP_ID",                   GROUP_ID,
	    "%s", "GROUP_NAME",                 GROUP_NAME,
	    "%u", "GROUP_STATUS",               GROUP_STATE,
	    "%f", "GROUP_CPC_PRICE",            GROUP_CPC_PRICE,
	    "%f", "GROUP_CPM_PRICE",            GROUP_CPM_PRICE,
	    "%u", "GROUP_TARGETING_TYPE",       GROUP_TARGETING_TYPE,
	    "%s", "GROUP_DISEASE",              GROUP_DISEASE,
	    "%u", "GROUP_IMPRESSION",           0,
	    "%u", "GROUP_CLICK",                0,
	    "%d", "GROUP_CTR",                  0,
	    "%f", "GROUP_CPC",                  0,
	    "%f", "GROUP_COST",                 0,
	    "%f", "GROUP_CPM",                  0);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

//推广->获取文字创意
int  ad_system_packet_get_creative_text(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (json->FIND_STATUS == 6) {

	if (ad_system_sql_get_creative_del_text(&sql_conn,
		    r->ad_system_account_id, 
		    json->FIND_TYPE, 
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    } else {

	if (ad_system_sql_get_creative_text(&sql_conn, 
		    r->ad_system_account_id, 
		    json->FIND_TYPE, 
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    //获取mongo数据
    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_DAY_TEMP, 2, r->ad_system_account_id, "creative_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_YESTERDAY_TEMP, 2, r->ad_system_account_id, "creative_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_SEVENDAY_TEMP, 2, r->ad_system_account_id, "creative_id");
    else
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    //合并数据
    lg_list_t merger_head;
    ad_system_merger_creative_info_t info;
    ad_system_merger_creative_text(
	    &sql_conn, 
	    &mongo_head, 
	    json,
	    &merger_head,
	    &info);

    max = size = sprintf(buffer, "{\"CODE\":\"0\","
	    "\"INFO\":{"
	    "\"PAGE_INDEX\":\"%u\","
	    "\"CREATIVE_COUNT\":\"%u\","
	    "\"CREATIVE_ENABLED\":\"%u\","
	    "\"CREATIVE_IMPRESSION\":\"%u\","
	    "\"CREATIVE_CLICK\":\"%u\","
	    "\"CREATIVE_CTR\":\"%0.2f%%\","
	    "\"CREATIVE_CPC\":\"%0.2f\","
	    "\"CREATIVE_COST\":\"%0.2f\","
	    "\"CREATIVE_CPM\":\"%0.2f\""
	    "},\"LIST\":[", 

	    json->PAGE_INDEX,
	    info.CREATIVE_COUNT,
	    info.CREATIVE_ENABLED,
	    info.CREATIVE_IMPRESSION,
	    info.CREATIVE_CLICK,
	    info.CREATIVE_CTR * 100,
	    info.CREATIVE_CPC,
	    info.CREATIVE_COST,
	    info.CREATIVE_CPM);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    int page_index = 1;
    char comma[4] = "{";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_creative_text_t *node = NULL;
	node = (ad_system_merger_creative_text_t *)p;

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_string_json_sprintf(buffer, comma, "}", 21,
		    "%s", "CAMPAIGN_ID",              node->CAMPAIGN_ID,
		    "%s", "CAMPAIGN_NAME",            node->CAMPAIGN_NAME,
		    "%s", "GROUP_ID",                 node->GROUP_ID,
		    "%s", "GROUP_NAME",               node->GROUP_NAME,
		    "%s", "CREATIVE_ID",              node->CREATIVE_ID,
		    "%s", "CREATIVE_NAME",            node->CREATIVE_NAME,
		    "%u", "CREATIVE_STATUS",          node->CREATIVE_STATE,
		    "%s", "CREATIVE_REFUSE_REASON",   node->CREATIVE_REFUSE_REASON,
		    "%s", "CREATIVE_DESCRIPTION",     node->CREATIVE_DESCRIPTION,
		    "%s", "CREATIVE_DESTINATION_URL", node->CREATIVE_DESTINATION_URL,
		    "%s", "CREATIVE_DISPLAY_URL",     node->CREATIVE_DISPLAY_URL,
		    "%u", "CREATIVE_APPEND_TYPE",     node->CREATIVE_APPEND_TYPE,
		    "%s", "CREATIVE_APPEND_ID",       node->CREATIVE_APPEND_ID,
		    "%u", "CREATIVE_IMPRESSION",      node->CREATIVE_IMPRESSION,
		    "%u", "CREATIVE_CLICK",           node->CREATIVE_CLICK,
		    "%e", "CREATIVE_CTR",             node->CREATIVE_CTR,
		    "%f", "CREATIVE_CPC",             node->CREATIVE_CPC,
		    "%f", "CREATIVE_COST",            node->CREATIVE_COST,
		    "%f", "CREATIVE_CPM",             node->CREATIVE_CPM,
		    "%s", "MOBILE_DESTINATION_URL",   node->MOBILE_DESTINATION_URL,
		    "%s", "MOBILE_DISPLAY_URL",       node->MOBILE_DISPLAY_URL
			);

	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	    strcpy(comma, ",{");
	}

	page_index++;
    }

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);

    ad_system_sql_free(&sql_conn);
    ad_system_merger_free(&merger_head);
    return ad_system_http_send(r, send_head, max);
}

int  ad_system_packet_set_creative_text(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{
    if (!json->UPDATE_TYPE)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (json->UPDATE_TYPE == 1) {

	if (ad_system_sql_set_creative_text(json->CREATIVE_ID, json->STATUS))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    } else {

	if (ad_system_sql_copy_creative_text(
		    json->UPDATE_TYPE,
		    json->CREATIVE_ID,
		    json->CREATIVE_NAME,
		    json->CREATIVE_DESCRIPTION,
		    json->CREATIVE_DESTINATION_URL,
		    json->CREATIVE_DISPLAY_URL,
		    json->CREATIVE_APPEND_TYPE,
		    json->CREATIVE_APPEND_ID,
		    json->MOBILE_DESTINATION_URL,
		    json->MOBILE_DISPLAY_URL))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	if (ad_system_sql_del_creative_text(json->CREATIVE_ID))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_add_creative_text(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char campaign_id[1024];
    if (ad_system_sql_get_campaign_id_from_group_or_creative(campaign_id, (char *)json->GROUP_ID, NULL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (ad_system_sql_add_creative_text(
		r->ad_system_account_id,
		campaign_id,
		json->GROUP_ID,
		json->CREATIVE_NAME,
		json->CREATIVE_DESCRIPTION,
		json->CREATIVE_DESTINATION_URL,
		json->CREATIVE_DISPLAY_URL,
		json->CREATIVE_APPEND_TYPE,
		json->CREATIVE_APPEND_ID,
		json->MOBILE_DESTINATION_URL,
		json->MOBILE_DISPLAY_URL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//删除文字创意
int  ad_system_packet_del_creative_text(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_creative_text(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_img_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (json->FIND_STATUS == 6) {

	if (ad_system_sql_get_creative_del_img(&sql_conn,
		    r->ad_system_account_id, 
		    json->FIND_TYPE,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    } else {

	if (ad_system_sql_get_img_creative(&sql_conn,
		    r->ad_system_account_id, 
		    json->FIND_TYPE,
		    json->FIND_STATUS,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    //获取mongo数据
    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_DAY_TEMP, 1, r->ad_system_account_id, "creative_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_YESTERDAY_TEMP, 1, r->ad_system_account_id, "creative_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_SEVENDAY_TEMP, 1, r->ad_system_account_id, "creative_id");
    else
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    //合并数据
    lg_list_t merger_head;
    ad_system_merger_creative_info_t info;
    ad_system_merger_creative_img(
	    &sql_conn,
	    &mongo_head, 
	    json,
	    &merger_head,
	    &info);

    //发送数据
    max = size = sprintf(buffer, "{\"CODE\":\"0\","
	    "\"INFO\":{"
	    "\"PAGE_INDEX\":\"%u\","
	    "\"CREATIVE_COUNT\":\"%u\","
	    "\"CREATIVE_ENABLED\":\"%u\","
	    "\"CREATIVE_IMPRESSION\":\"%u\","
	    "\"CREATIVE_CLICK\":\"%u\","
	    "\"CREATIVE_CTR\":\"%0.2f%%\","
	    "\"CREATIVE_CPC\":\"%0.2f\","
	    "\"CREATIVE_COST\":\"%0.2f\","
	    "\"CREATIVE_CPM\":\"%0.2f\""
	    "},\"LIST\":[", 
	    json->PAGE_INDEX,
	    info.CREATIVE_COUNT,
	    info.CREATIVE_ENABLED,
	    info.CREATIVE_IMPRESSION,
	    info.CREATIVE_CLICK,
	    info.CREATIVE_CTR * 100,
	    info.CREATIVE_CPC,
	    info.CREATIVE_COST,
	    info.CREATIVE_CPM);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    int page_index = 1;
    char  comma[4] = "{";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_creative_img_t *node = NULL;
	node = (ad_system_merger_creative_img_t*)p;

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_string_json_sprintf(buffer, comma, "}", 22,
		    "%s", "CAMPAIGN_ID",              	    node->CAMPAIGN_ID,
		    "%s", "CAMPAIGN_ID",		    node->CAMPAIGN_ID,                  
		    "%s", "CAMPAIGN_NAME",                  node->CAMPAIGN_NAME, 
		    "%s", "GROUP_ID",                       node->GROUP_ID, 
		    "%s", "GROUP_NAME",                     node->GROUP_NAME, 
		    "%s", "CREATIVE_ID",                    node->CREATIVE_ID, 
		    "%s", "CREATIVE_NAME",                  node->CREATIVE_NAME, 
		    "%u", "CREATIVE_STATUS",                node->CREATIVE_STATE, 
		    "%s", "CREATIVE_REFUSE_REASON",         node->CREATIVE_REFUSE_REASON, 
		    "%u", "CREATIVE_DSP_STATE",             node->CREATIVE_DSP_STATE, 
		    "%s", "CREATIVE_DSP_REFUSE_REASON",     node->CREATIVE_DSP_REFUSE_REASON, 
		    "%s", "CREATIVE_IMG_SIZE",              node->CREATIVE_IMG_SIZE, 
		    "%s", "CREATIVE_IMG_SIZE_IMGLENGTH",    node->CREATIVE_IMG_SIZE_IMGLENGTH, 
		    "%s", "CREATIVE_FILE_PATH",             node->CREATIVE_FILE_PATH, 
		    "%s", "CREATIVE_DESTINATION_URL",       node->CREATIVE_DESTINATION_URL, 
		    "%s", "CREATIVE_DISPLAY_URL",           node->CREATIVE_DISPLAY_URL, 
		    "%u", "CREATIVE_IMPRESSION",            node->CREATIVE_IMPRESSION, 
		    "%u", "CREATIVE_CLICK",                 node->CREATIVE_CLICK, 
		    "%e", "CREATIVE_CTR",                   node->CREATIVE_CTR, 
		    "%f", "CREATIVE_CPC",                   node->CREATIVE_CPC, 
		    "%f", "CREATIVE_COST",                  node->CREATIVE_COST, 
		    "%f", "CREATIVE_CPM",                   node->CREATIVE_CPM
			);
	    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

	    strcpy(comma, ",{");
	}

	page_index++;
    }

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);

    ad_system_sql_free(&sql_conn);
    ad_system_merger_free(&merger_head);
    return ad_system_http_send(r, send_head, max);
}

int  ad_system_packet_set_upload_img_info (ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    redisContext *conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
    if (!conn) return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    redisReply *reply = redisCommand(conn, "ZRANGE %s 0 -1", json->CREATIVE_IMG_ID);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	freeReplyObject(reply);
	redisFree(conn);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_IMG_ID_ERR);
    }

    redisReply *del = redisCommand(conn, "DEL %s", json->CREATIVE_IMG_ID);
    freeReplyObject(del);

    redisReply *new_reply = redisCommand(conn, "ZADD %s 0 %s 1 %s 2 %s 3 %s",
	    json->CREATIVE_IMG_ID,
	    reply->element[0]->str,//name
	    reply->element[1]->str,//img_size
	    reply->element[2]->str,//img_buffer_size
	    reply->element[3]->str);

    freeReplyObject(new_reply);
    freeReplyObject(reply);
    redisFree(conn);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_add_creative_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char buffer[40960];
    char *save_ptr = NULL;
    char *img_id = lg_string_strtok(json->CREATIVE_IMG_ID, buffer, &save_ptr, ',');

    while(img_id) {

	//查询上传的图片信息
	redisContext *conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
	if (!conn) return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	redisReply *reply = redisCommand(conn, "ZRANGE %s 0 -1", img_id);
	if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	    freeReplyObject(reply);
	    redisFree(conn);
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_IMG_ID_ERR);
	}

	//查询所属组与计划
	char campaign_id[1024];
	if (ad_system_sql_get_campaign_id_from_group_or_creative(campaign_id, (char *)json->GROUP_ID, NULL))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	//创建新的图片创意
	if (ad_system_sql_add_creative_img(
		    r->ad_system_account_id,
		    campaign_id,
		    json->GROUP_ID,
		    reply->element[0]->str,//name
		    reply->element[1]->str,//img_size
		    reply->element[2]->str,//img_buffer_size
		    reply->element[3]->str,//img_path
		    json->CREATIVE_DESTINATION_URL,
		    json->CREATIVE_DISPLAY_URL)) {

	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	img_id = lg_string_strtok(NULL, buffer, &save_ptr, ',');
	freeReplyObject(reply);
	redisFree(conn);
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//删除图片创意
int  ad_system_packet_del_creative_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_creative_img(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//获取充值记录
int  ad_system_packet_get_money_recharge(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_count;
    if (ad_system_sql_get_money_recharge_count(&sql_count,
		r->ad_system_account_id,
		json->FIND_TYPE,
		json->MONEY_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    ad_system_sql_t sql_data;
    if (ad_system_sql_get_money_recharge(&sql_data,
		r->ad_system_account_id,
		json->FIND_TYPE,
		json->MONEY_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	ad_system_sql_free(&sql_count);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    if (!(ad_system_sql_fetch_next(&sql_count))) {

	ad_system_sql_free(&sql_count);
	ad_system_sql_free(&sql_data);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%u\"},\"LIST\":[", 
	    json->PAGE_INDEX, 
	    ad_system_sql_get_number(&sql_count, NULL, 1));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_data)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 8,

		"%s", "SWIFT_NUMBER",			ad_system_sql_get_string(&sql_data, NULL, 1),
		"%s", "MONEY",                          ad_system_sql_get_string(&sql_data, NULL, 2),
		"%s", "PAYMENT_TYPE",                   ad_system_sql_get_string(&sql_data, NULL, 3),
		"%s", "ACCOUNTING_DOCUMENT_NUMBER",     ad_system_sql_get_string(&sql_data, NULL, 4),
		"%s", "ACCOUNTING_DOCUMENT_FILE",       ad_system_sql_get_string(&sql_data, NULL, 5),
		"%s", "INVEST",                         ad_system_sql_get_string(&sql_data, NULL, 6),
		"%s", "BALANCE",                        ad_system_sql_get_string(&sql_data, NULL, 7),
		"%s", "CREATE_TIME",                    ad_system_sql_get_string(&sql_data, NULL, 8));

	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	strcpy(comma, ",{");
    }

    ad_system_sql_free(&sql_count);
    ad_system_sql_free(&sql_data);

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);
    return ad_system_http_send(r, send_head, max);
}

int  ad_system_packet_get_creative_append(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_creative_append(&sql_conn,
		r->ad_system_account_id, 
		json->FIND_TYPE,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->FIND_NAME,
		json->FIND_STATUS))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":[");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 4,
		"%s","APPEND_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s","APPEND_CONTENT",       ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s","APPEND_STATUS",        ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s","APPEND_REFUSE_REASON", ad_system_sql_get_string(&sql_conn, NULL, 4));

	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	strcpy(comma, ",{");
    }

    max += 2;
    ad_system_chain_add_r(r, &send_head, &send_next, "]}", 2);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int  ad_system_packet_set_creative_append(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_set_creative_append(
		r->ad_system_account_id,
		json->CREATIVE_APPEND_ID,
		json->UPDATE_TYPE,
		json->APPEND_STATUS,
		json->APPEND_CONTENT))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_add_creative_append(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_add_creative_append(r->ad_system_account_id, json->APPEND_CONTENT))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//删除附加创意
int ad_system_packet_del_creative_append(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_creative_append(json->CREATIVE_APPEND_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_creative_append_unique(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_creative_append_unique(&sql_conn, json->CREATIVE_APPEND_ID))
	ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (!ad_system_sql_fetch_next(&sql_conn))
	ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    char buffer[4096];
    size_t size = lg_string_json_sprintf(buffer, "{", "}", 5,
	    "%s", "CODE", "0",		
	    "%s", "APPEND_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
	    "%s", "APPEND_STATUS",            ad_system_sql_get_string(&sql_conn, NULL, 2),
	    "%s", "APPEND_CONTENT",           ad_system_sql_get_string(&sql_conn, NULL, 3),
	    "%s", "APPEND_REFUSE_REASON",     ad_system_sql_get_string(&sql_conn, NULL, 4));

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_get_user_info(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_sql_t sql_conn;
    if ((ad_system_sql_get_user_info(&sql_conn, r->ad_system_account_id)))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    char buffer[40960];
    size_t size = lg_string_json_sprintf(buffer, "{", "}", 20,

	    "%s", "CODE", "0",
	    "%s", "USER_LOGIN_NAME",    ad_system_sql_get_string(&sql_conn, NULL, 1),
	    "%s", "COMPANY_NAME",       ad_system_sql_get_string(&sql_conn, NULL, 2),
	    "%s", "COMPANY_SITE",       ad_system_sql_get_string(&sql_conn, NULL, 3),
	    "%s", "INDUSTRY",          	ad_system_sql_get_string(&sql_conn, NULL, 4),
	    "%s", "API_TOKEN",         	ad_system_sql_get_string(&sql_conn, NULL, 5),
	    "%s", "PROVINCE_ID",       	ad_system_sql_get_string(&sql_conn, NULL, 6),
	    "%s", "CITY_ID",           	ad_system_sql_get_string(&sql_conn, NULL, 7),
	    "%s", "ADDRESS",           	ad_system_sql_get_string(&sql_conn, NULL, 8),
	    "%s", "POSTCODE",          	ad_system_sql_get_string(&sql_conn, NULL, 9),
	    "%s", "CONTACT",           	ad_system_sql_get_string(&sql_conn, NULL, 10),  			
	    "%s", "TELEPHONE",         	ad_system_sql_get_string(&sql_conn, NULL, 11),  			
	    "%s", "FAX",               	ad_system_sql_get_string(&sql_conn, NULL, 12),
	    "%s", "MOBILE",            	ad_system_sql_get_string(&sql_conn, NULL, 13),
	    "%s", "EMAIL",              ad_system_sql_get_string(&sql_conn, NULL, 14),
	    "%s", "WEBIM",              ad_system_sql_get_string(&sql_conn, NULL, 15),
	    "%s", "LATE_LOGIN_TIME",    ad_system_sql_get_string(&sql_conn, NULL, 16),
	    "%s", "LATE_LOGIN_IP",      ad_system_sql_get_string(&sql_conn, NULL, 17),
	    "%s", "LATE_LOGIN_ADDRESS", ad_system_sql_get_string(&sql_conn, NULL, 18),
	    "%s", "PLACE_REGION", 	ad_system_sql_get_string(&sql_conn, NULL, 19));

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

//设置用户信息
int ad_system_packet_set_user_info(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{
    ad_system_sql_t sql_conn;
    if (ad_system_sql_set_user_info(&sql_conn,
		r->ad_system_account_id,
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
		json->WEBIM))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

//账户信息->修改密码
int ad_system_packet_set_user_pass(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->PASS_OLD || !json->PASS_OLD)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_user_pass(&sql_conn, r->ad_system_account_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    const char *PASS_OLD = NULL;
    if (!(PASS_OLD = ad_system_sql_get_string(&sql_conn, NULL, 1)))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    if (strcmp(json->PASS_OLD, PASS_OLD))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    if (ad_system_sql_set_user_pass(r->ad_system_account_id, json->PASS_NEW))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_OK);
}

//创建标准报告
int ad_system_packet_report_standard(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_report_arg_t report_arg;
    ad_system_report_arg_t *p = &report_arg;
    memset(p, 0, sizeof(ad_system_report_arg_t));
    p->report_type = json->REPORT_TYPE;

    if (ad_system_report_path_create(p->report_key, p->report_path))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (json->REPORT_SPECIES & 1) p->display_style = 2;
    else p->display_style = 1;
    strcpy(p->account_id, r->ad_system_account_id);

    p->time_type = json->TIME_TYPE;
    strcpy(p->time_s, json->TIME_START);
    strcpy(p->time_e, json->TIME_END);

    char time_range[128];
    sprintf(time_range, "%s->%s", json->TIME_START, json->TIME_END);

    char down_path[128];
    if (json->REPORT_SPECIES & 1) 
	sprintf(down_path, "%s/cpc.csv", p->report_path);
    else
	sprintf(down_path, "%s/report.zip", p->report_path);

    if (ad_system_mongo_report_list_add(r->ad_system_account_id,
		p->report_key,
		json->REPORT_NAME,
		json->REPORT_TYPE,
		p->display_style,
		time_range,
		down_path))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
    ad_system_report_standard_create(p);
    return 0;
}

int ad_system_packet_report_complex(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ad_system_report_arg_t report_arg;
    ad_system_report_arg_t *p = &report_arg;
    memset(p, 0, sizeof(ad_system_report_arg_t));
    p->report_type = json->REPORT_TYPE;

    if (ad_system_report_path_create(p->report_key, p->report_path))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (json->REPORT_SPECIES == 1) p->display_style = 2;
    else p->display_style = 1;
    strcpy(p->account_id, r->ad_system_account_id);

    p->time_type = json->TIME_TYPE;
    strcpy(p->time_s, json->TIME_START);
    strcpy(p->time_e, json->TIME_END);

    char time_range[128];
    sprintf(time_range, "%s->%s", json->TIME_START, json->TIME_END);

    char down_path[128];
    sprintf(down_path, "%s/report.zip", p->report_path);

    if (ad_system_mongo_report_list_add(r->ad_system_account_id,
		p->report_key,
		json->REPORT_NAME,
		json->REPORT_TYPE + 4,
		p->display_style,
		time_range,
		down_path))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
    ad_system_report_complex_create(p);
    return 0;
}

int ad_system_packet_get_report_list(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{
    char buffer[40960];
    size_t size, max = 0;
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    lg_list_t head;
    if (ad_system_mongo_report_list_find(r->ad_system_account_id, json->REPORT_BELONG, &head) != 0)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":[");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "";
    lg_list_t *p = NULL;
    lg_list_for_tail(p, &head) {

	ad_system_mongo_report_list_t *node = NULL;
	node = (ad_system_mongo_report_list_t *)p;

	max += size = sprintf(buffer, "%s{"
		"\"REPORT_ID\":\"%s\","
		"\"REPORT_NAME\":\"%s\","
		"\"REPORT_TYPE\":\"%s\","
		"\"RANGE_TIME\":\"%s\","
		"\"CREATE_TIME\":\"%s\","
		"\"STATUS\":\"%s\","
		"\"PATH\":\"%s\"}",
		comma,
		node->report_id,
		node->report_name,
		node->report_type,
		node->range_time,
		node->create_time,
		node->status,
		node->path);

	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
	comma[0] = ',';
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_mongo_list_free(&head);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_del_report(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_mongo_report_list_del(r->ad_system_account_id, json->REPORT_ID) != 0)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_batch_operation(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (json->BATCH_TYPE == 3) {

	char *group_id_save_ptr = NULL;
	char group_id_buffer[40960] = {0};

	char *group_cpc_price_save_ptr = NULL;
	char group_cpc_price_buffer[40960] = {0};

	char *group_cpm_price_save_ptr = NULL;
	char group_cpm_price_buffer[40960] = {0};

	char *group_id = lg_string_strtok(json->ID, group_id_buffer, &group_id_save_ptr, ',');
	char *group_cpc_price = lg_string_strtok(json->GROUP_CPC_PRICE, group_cpc_price_buffer, &group_cpc_price_save_ptr, ',');
	char *group_cpm_price = lg_string_strtok(json->GROUP_CPM_PRICE, group_cpm_price_buffer, &group_cpm_price_save_ptr, ',');
	while(group_id) {

	    char budget[128] = {0};
	    char bidding_model[128] = {0};
	    if (ad_system_sql_get_campaign_budget(r->ad_system_account_id, NULL, group_id, budget, bidding_model))
		return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

	    if (json->GROUP_PRICE_TYPE & 1)
		if (atof(group_cpc_price) > atof(budget))
		    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);

	    if (json->GROUP_PRICE_TYPE & 2)
		if (atof(group_cpm_price) > atof(budget))
		    return ad_system_packet_send_code(r, NULL, PACKET_CODE_PRICE_EXPENSIVE);

	    group_id = lg_string_strtok(NULL, group_id_buffer, &group_id_save_ptr, ',');
	    group_cpc_price = lg_string_strtok(NULL, group_cpc_price_buffer, &group_cpc_price_save_ptr, ',');
	    group_cpm_price = lg_string_strtok(NULL, group_cpm_price_buffer, &group_cpm_price_save_ptr, ',');
	}

	group_id = lg_string_strtok(json->ID, group_id_buffer, &group_id_save_ptr, ',');
	group_cpc_price = lg_string_strtok(json->GROUP_CPC_PRICE, group_cpc_price_buffer, &group_cpc_price_save_ptr, ',');
	group_cpm_price = lg_string_strtok(json->GROUP_CPM_PRICE, group_cpm_price_buffer, &group_cpm_price_save_ptr, ',');
	while(group_id) {

	    char budget[128] = {0};
	    char bidding_model[128] = {0};
	    if (ad_system_sql_get_campaign_budget(r->ad_system_account_id, NULL, group_id, budget, bidding_model))
		return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

	    ad_system_sql_ad_system_batch_operation_group_price(group_id,
		    json->GROUP_PRICE_TYPE,
		    group_cpc_price,
		    group_cpm_price);

	    group_id = lg_string_strtok(NULL, group_id_buffer, &group_id_save_ptr, ',');
	    group_cpc_price = lg_string_strtok(NULL, group_cpc_price_buffer, &group_cpc_price_save_ptr, ',');
	    group_cpm_price = lg_string_strtok(NULL, group_cpm_price_buffer, &group_cpm_price_save_ptr, ',');
	}

	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
    }

    char *save_ptr = NULL;
    char buffer[40960] = {0};
    char *id = lg_string_strtok(json->ID, buffer, &save_ptr, ',');
    while(id) {

	if (json->BATCH_TYPE == 1) {

	    switch(json->ID_TYPE){

		case 1:
		    ad_system_sql_set_campaign(r->ad_system_account_id, id, 1, json->STATUS, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL);
		    break;
		case 2:
		    ad_system_sql_set_group(r->ad_system_account_id, id, 1, json->STATUS, NULL, NULL, 0, 0, NULL);
		    break;
		case 3:
		    ad_system_sql_set_creative_text(id, json->STATUS);
		    break;
		case 4:
		    ad_system_sql_set_creative_append(r->ad_system_account_id, id, 1, json->STATUS, NULL);
		    break;
		case 5:
		    ad_system_sql_set_creative_img(r->ad_system_account_id, id, json->STATUS);
		    break;
	    }

	} else if (json->BATCH_TYPE == 2) {

	    switch(json->ID_TYPE){

		case 1:
		    ad_system_sql_del_campaign(id);
		    break;
		case 2:
		    ad_system_sql_del_group(id);
		    break;
		case 3:
		    ad_system_sql_del_creative_text(id);
		    break;
		case 4:
		    ad_system_sql_del_creative_append(id);
		    break;
		case 5:
		    ad_system_sql_del_creative_img(id);
		    break;
	    }

	} else if (json->BATCH_TYPE == 3) {

	    //	    ad_system_sql_ad_system_batch_operation_group_price(id,
	    //		    json->GROUP_PRICE_TYPE,
	    //		    GROUP_CPC_PRICE,
	    //		    GROUP_CPM_PRICE);
	}

	id = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_copy_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_copy_creative_check(
		r->ad_system_account_id,
		json->CAMPAIGN_ID,
		json->GROUP_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char *save_ptr = NULL;
    char buffer[40960] = {0};
    char *creative_id = lg_string_strtok(json->ID, buffer, &save_ptr, ',');
    while(creative_id) {

	if (json->CREATIVE_TYPE == 1)
	    ad_system_sql_copy_creative_img_r(
		    creative_id,
		    r->ad_system_account_id,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID);

	else if (json->CREATIVE_TYPE == 2)
	    ad_system_sql_copy_creative_text_r(
		    creative_id,
		    r->ad_system_account_id,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID);

	creative_id= lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_brand(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;

    char DEPT_ARRAY[2048] = {0};
    if (json->DEPT_TYPE == 2)
	lg_string_json_to_sql_array(json->DEPT_SERIALIZE, DEPT_ARRAY);

    char REGION_ARRAY[2048] = {0};
    if (json->REGION_TYPE == 2)
	lg_string_json_to_sql_array(json->REGION_SERIALIZE, REGION_ARRAY);

    ad_system_sql_t sql_conn;
    int ret = ad_system_sql_get_brand(&sql_conn,
	    r->ad_system_account_id,
	    json->STATUS,
	    json->CREATIVE_STATUS,
	    json->DEPT_TYPE, 
	    DEPT_ARRAY,
	    json->REGION_TYPE, 
	    REGION_ARRAY,
	    json->SHOW_TIME_TYPE,
	    json->SHOW_TIME_START,
	    json->SHOW_TIME_END,
	    json->SORT_TYPE,
	    json->SORT_COLUMN,
	    json->page_index_s,
	    json->page_index_e);

    if (ret == 100) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    char count[128] = {0};
    if (ad_system_sql_get_brand_count(count,
		r->ad_system_account_id,
		json->STATUS,
		json->CREATIVE_STATUS,
		json->DEPT_TYPE, 
		DEPT_ARRAY,
		json->REGION_TYPE, 
		REGION_ARRAY,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->page_index_s,
		json->page_index_e)) {

	ad_system_sql_free(&sql_conn);
	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return ad_system_http_send_buffer(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 15,
		"%s", "BRAND_ID", 		ad_system_sql_get_string(&sql_conn, NULL, 1),
		"%s", "AD_ORDER_NO",            ad_system_sql_get_string(&sql_conn, NULL, 2),
		"%s", "DEPT_CODE",              ad_system_sql_get_string(&sql_conn, NULL, 3),
		"%s", "REGION_TYPE",            ad_system_sql_get_string(&sql_conn, NULL, 4),
		"%s", "REGION_SERIALIZE",       ad_system_sql_get_string(&sql_conn, NULL, 5),
		"%s", "DAYS",                   ad_system_sql_get_string(&sql_conn, NULL, 6),
		"%s", "SHOW_TIME_START",        ad_system_sql_get_string(&sql_conn, NULL, 7),
		"%s", "SHOW_TIME_END",          ad_system_sql_get_string(&sql_conn, NULL, 8),
		"%s", "PRICE",                  ad_system_sql_get_string(&sql_conn, NULL, 9),
		"%s", "CREATE_TIME",            ad_system_sql_get_string(&sql_conn, NULL, 10),
		"%s", "BRAND_STATUS",           ad_system_sql_get_string(&sql_conn, NULL, 11),
		"%s", "BRAND_REFUSE_REASON",    ad_system_sql_get_string(&sql_conn, NULL, 12),
		"%s", "CREATIVE_STATE",         ad_system_sql_get_string(&sql_conn, NULL, 13),
		"%s", "CREATIVE_REFUSE_REASON", ad_system_sql_get_string(&sql_conn, NULL, 14),
		"%s", "INSPECTION_STATUS", 	ad_system_sql_get_string(&sql_conn, NULL, 15)
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

    if (json->REGION_SERIALIZE && strlen(json->REGION_SERIALIZE) > 2048)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_BRAND_REGION_LEN_ERR);

    if (json->REGION_SERIALIZE && strlen(json->DEPT_SERIALIZE) > 2048)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_BRAND_REGION_LEN_ERR);

    if (json->PRICE && !(atoi(json->PRICE) > 0))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    if (ad_system_sql_add_brand(
		r->ad_system_account_id,
		json->REGION_TYPE,
		json->REGION_SERIALIZE,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->DEPT_SERIALIZE,
		json->PRICE,
		r->ad_system_account_id))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_del_brand(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (ad_system_sql_del_brand(json->BRAND_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_get_brand_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    char buffer[40960];
    size_t size, max = 0;
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_get_brand_creative(&sql_conn, json->MOD_STATUS, json->BRAND_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_BRAND_CREATIVE_ERR);
	//return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
    }

    max = size = lg_string_json_sprintf(buffer, "{", ",\"BRAND_CREATIVE\":[", 16,
	    "%s", "CODE", "0",
	    "%s", "BRAND_ID",		ad_system_sql_get_string(&sql_conn, NULL, 1),
	    "%s", "BRAND_CREATIVE_ID",  ad_system_sql_get_string(&sql_conn, NULL, 2),
	    "%s", "TITLE",              ad_system_sql_get_string(&sql_conn, NULL, 3),
	    "%s", "DESCRIPTION",        ad_system_sql_get_string(&sql_conn, NULL, 4),
	    "%s", "DESTINATION_URL",    ad_system_sql_get_string(&sql_conn, NULL, 5),
	    "%s", "DISPLAY_URL",        ad_system_sql_get_string(&sql_conn, NULL, 6),
	    "%s", "TELEPHONE",          ad_system_sql_get_string(&sql_conn, NULL, 7),
	    "%s", "WEB_SITE",           ad_system_sql_get_string(&sql_conn, NULL, 8),
	    "%s", "WEB_IM",             ad_system_sql_get_string(&sql_conn, NULL, 9),
	    "%s", "GUAHAO_URL",         ad_system_sql_get_string(&sql_conn, NULL, 10),
	    "%s", "CREATIVE_STATE",     ad_system_sql_get_string(&sql_conn, NULL, 11),
	    "%s", "REFUSE_REASON",      ad_system_sql_get_string(&sql_conn, NULL, 12),
	    "%s", "CREATE_TIME",        ad_system_sql_get_string(&sql_conn, NULL, 13),
	    "%s", "LAST_UPDATE_TIME",   ad_system_sql_get_string(&sql_conn, NULL, 14),
	    "%s", "INSPECTION_STATUS,", ad_system_sql_get_string(&sql_conn, NULL, 15));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    max += size = lg_string_json_sprintf(buffer, "{", "},", 3,
	    "%s", "SEQUENCE",		ad_system_sql_get_string(&sql_conn, NULL, 16),
	    "%s", "IMG_PATH",  		ad_system_sql_get_string(&sql_conn, NULL, 17),
	    "%s", "IMG_URL",            ad_system_sql_get_string(&sql_conn, NULL, 18));
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    char comma[4] = "{";
    while(ad_system_sql_fetch_next(&sql_conn)) {

	max += size = lg_string_json_sprintf(buffer, comma, "}", 3,
		"%s", "SEQUENCE",		ad_system_sql_get_string(&sql_conn, NULL, 16),
		"%s", "IMG_PATH",  		ad_system_sql_get_string(&sql_conn, NULL, 17),
		"%s", "IMG_URL",            ad_system_sql_get_string(&sql_conn, NULL, 18));

	strcpy(comma, ",{");
	ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    ad_system_chain_add_r(r, &send_head, &send_next, buffer, size);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send(r, send_head, max);
}

int ad_system_packet_add_brand_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->WEB_SITE
	    || !json->IMG0_URL
	    || !json->IMG1_URL
	    || !json->IMG2_URL
	    || !json->IMG3_URL
	    || !json->IMG4_URL
	    || !json->IMG5_URL)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    char domain[4096] = {0};
    if (ad_system_sql_get_account_domain(r->ad_system_account_id, domain))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);

    if (!strstr(json->WEB_SITE, domain)
	    || !strstr(json->IMG0_URL, domain)
	    || !strstr(json->IMG1_URL, domain)
	    || !strstr(json->IMG2_URL, domain)
	    || !strstr(json->IMG3_URL, domain)
	    || !strstr(json->IMG4_URL, domain)
	    || !strstr(json->IMG5_URL, domain)
       ) {


	fprintf(stderr, "-->[%s][%s]<--\n", json->WEB_SITE, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG0_URL, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG1_URL, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG2_URL, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG3_URL, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG4_URL, domain);
	fprintf(stderr, "-->[%s][%s]<--\n", json->IMG5_URL, domain);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_PARAMETER_ERR);
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (ad_system_sql_add_brand_creative(&sql_conn,
		json->INSERT_TYPE,
		json->BRAND_ID,
		r->ad_system_account_id,
		json->TITLE,
		json->DESCRIPTION,
		json->DESTINATION_URL,
		json->DISPLAY_URL,
		json->TELEPHONE,
		json->WEB_SITE,
		json->WEB_IM,
		json->GUAHAO_URL)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    char img_size[128] = {0}, img_path[1024] = {0};

    if (ad_system_redis_get_img(json->IMG0_ID, NULL, img_size, NULL, img_path)) {


	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG0_URL,
		0)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_redis_get_img(json->IMG1_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG1_URL,
		1)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_redis_get_img(json->IMG2_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG2_URL,
		2)) {
	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_redis_get_img(json->IMG3_ID, NULL, img_size, NULL, img_path)) {
	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG3_URL,
		3)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_redis_get_img(json->IMG4_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG4_URL,
		4)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_redis_get_img(json->IMG5_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(&sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG5_URL,
		5)) {

	ad_system_sql_rollback(&sql_conn);
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_ERR);
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_set_brand_creative(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->BRAND_CREATIVE_ID)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    if (json->UPDATE_TYPE & 1 ||
	    json->UPDATE_TYPE & 2 ||
	    json->UPDATE_TYPE & 4 ||
	    json->UPDATE_TYPE & 8 ||
	    json->UPDATE_TYPE & 16 ||
	    json->UPDATE_TYPE & 32 ||
	    json->UPDATE_TYPE & 64 ||
	    json->UPDATE_TYPE & 128) {

	if (ad_system_sql_set_brand_creative(json->UPDATE_TYPE,
		    json->BRAND_CREATIVE_ID,
		    json->ACCOUNT_ID,
		    json->TITLE,
		    json->DESCRIPTION,
		    json->DESTINATION_URL,
		    json->DISPLAY_URL,
		    json->TELEPHONE,
		    json->WEB_SITE,
		    json->WEB_IM,
		    json->GUAHAO_URL))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
    }

    char img_size[128] = {0}, img_path[1024] = {0};

    //IMG0_ID
    if (json->UPDATE_TYPE & 256) {

	if (ad_system_redis_get_img(json->IMG0_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->01\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG0_URL,
		    0)) {
	    fprintf(stderr, "--->02\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG0_URL
    if (json->UPDATE_TYPE & 512) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG0_URL,
		    0)) {

	    fprintf(stderr, "--->03\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG1_ID
    if (json->UPDATE_TYPE & 1024) {

	if (ad_system_redis_get_img(json->IMG1_ID, NULL, img_size, NULL, img_path)) {
	    fprintf(stderr, "--->04\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG1_URL,
		    1)) {

	    fprintf(stderr, "--->05\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG1_URL
    if (json->UPDATE_TYPE & 2048) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG1_URL,
		    1)) {

	    fprintf(stderr, "--->06\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG2_ID
    if (json->UPDATE_TYPE & 4096) {

	if (ad_system_redis_get_img(json->IMG2_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->07\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG2_URL,
		    2)) {

	    fprintf(stderr, "--->08\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG2_URL
    if (json->UPDATE_TYPE & 8192) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG2_URL,
		    2)) {

	    fprintf(stderr, "--->09\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG3_ID
    if (json->UPDATE_TYPE & 16384) {

	if (ad_system_redis_get_img(json->IMG3_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->010\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG3_URL,
		    3)) {

	    fprintf(stderr, "--->011\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG3_URL
    if (json->UPDATE_TYPE & 32768) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG3_URL,
		    3)) {

	    fprintf(stderr, "--->012\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG4_ID
    if (json->UPDATE_TYPE & 65536) {

	if (ad_system_redis_get_img(json->IMG4_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->013\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG4_URL,
		    4)) {

	    fprintf(stderr, "--->014\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG4_URL
    if (json->UPDATE_TYPE & 131072) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG4_URL,
		    4)) {

	    fprintf(stderr, "--->015\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG5_ID
    if (json->UPDATE_TYPE & 262144) {

	if (ad_system_redis_get_img(json->IMG5_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->016\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG5_URL,
		    5)) {

	    fprintf(stderr, "--->017\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    //IMG5_URL
    if (json->UPDATE_TYPE & 524288) {

	if (ad_system_sql_set_brand_creative_append(
		    0,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG5_URL,
		    5)) {

	    fprintf(stderr, "--->018\n");
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);
	}
    }

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int  ad_system_packet_set_creative_img(ngx_http_request_t *r, ad_system_packet_json_data_t *json)
{

    if (!json->UPDATE_TYPE)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);

    if (json->UPDATE_TYPE & 1) {

	if (ad_system_sql_set_creative_img(
		    r->ad_system_account_id,
		    json->CREATIVE_ID,
		    json->CREATIVE_STATUS))
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
    }

    char *creative_name = NULL;
    char *creative_img_size = NULL;
    char *creative_img_buffer_size = NULL;
    char *creative_img_path = NULL;
    redisContext *conn = NULL;
    redisReply *reply = NULL;

    if (json->UPDATE_TYPE & 2)
	creative_name = json->CREATIVE_NAME;

    if (json->UPDATE_TYPE & 16) {

	//查询上传的图片信息
	conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
	if (!conn) return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

	reply = redisCommand(conn, "ZRANGE %s 0 -1", json->CREATIVE_IMG_ID);
	if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	    freeReplyObject(reply);
	    redisFree(conn);
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_IMG_ID_ERR);
	}

	creative_name = reply->element[0]->str;
	creative_img_size = reply->element[1]->str;
	creative_img_buffer_size = reply->element[2]->str;
	creative_img_path = reply->element[3]->str;
    }

    if (ad_system_sql_copy_creative_img(
		json->UPDATE_TYPE,
		json->CREATIVE_ID,
		creative_name,
		creative_img_size,
		creative_img_buffer_size,
		creative_img_path,
		json->CREATIVE_DESTINATION_URL,
		json->CREATIVE_DISPLAY_URL))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    if (json->UPDATE_TYPE & 16) {

	freeReplyObject(reply);
	redisFree(conn);
    }

    //删除原有创意
    if (ad_system_sql_del_creative_img(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_ERR);

    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_manager_account_check(ngx_http_request_t *r, const char *data)
{

    if (!data)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[buffer]%s\n", buffer);

    char *saveptr = NULL;
    char *account_name = strtok_r(buffer, ":", &saveptr);
    if (!account_name)return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char *password = strtok_r(NULL, ":", &saveptr);
    if (!password)return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[MA][%s][%s]\n", account_name, password);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_login(&sql_conn, account_name, password))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    if (!ad_system_sql_fetch_next(&sql_conn))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    const char *account_id = ad_system_sql_get_string(&sql_conn, NULL, 1);
    if (!account_id || !(*account_id))
	return ad_system_packet_send_code(r, &sql_conn, PACKET_CODE_USER_LOGIN_ERR);

    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"ACCOUNT_ID\":\"%s\"}", account_id);

    ad_system_sql_free(&sql_conn);
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_manager_account_session(ngx_http_request_t *r, const char *data)
{

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char *saveptr = NULL;
    char *session_id = strtok_r(buffer, ":", &saveptr);
    if (!session_id)return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    char *account_id = strtok_r(NULL, ":", &saveptr);
    if (!account_id)return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[MA][%s][%s]\n", session_id, account_id);

    ad_system_redis_set_login_session(session_id, account_id);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
}

int ad_system_packet_manager_account_login(ngx_http_request_t *r, const char *data)
{

    if (!data)
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    ngx_ad_system_set_session(r, (char *)data);

    char buffer[1024];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=http://wangmeng.haomeit.com\"></html>");
    return ad_system_http_send_buffer(r, buffer, size);
}

int ad_system_packet_manager_account_logout(ngx_http_request_t *r, const char *data)
{

    if (!data)	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    ad_system_redis_del_login_session(buffer);
    return ad_system_packet_send_code(r, NULL, PACKET_CODE_OK);
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
	    r->connection->fd, 
	    r->ad_system_account_id, 
	    r->ad_system_buffer);
    fflush(stdout);
    fprintf(stderr, "[recv][%04d-%02d-%02d %02d:%02d:%02d][%d][%s]%s\n",
	    tm.tm_year + 1900,
	    tm.tm_mon + 1,
	    tm.tm_mday,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
	    r->connection->fd, 
	    r->ad_system_account_id, 
	    r->ad_system_buffer);
    fflush(stderr);
}

int ad_system_packet_parse(ngx_http_request_t *r)
{
    json_object *json_root = NULL;
    if (!(json_root = json_tokener_parse(r->ad_system_buffer)))
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);

    if (is_error(json_root)) {
	json_object_put(json_root);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    }

    const char *packet_type = NULL;
    if (!(packet_type = ad_system_json_packet_head_parse(json_root))) {
	json_object_put(json_root);
	return ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
    }

    /********************/
    /**** 登录 ****/
    /********************/

    //检测登录
    if (strcmp(packet_type, "USER_LOGIN_CHECK") == 0) {
	ad_system_packet_login_check(r);

	//获取验证码
    } else if (strcmp(packet_type, "GET_CAPTCHA") == 0) {
	ad_system_packet_send_image(r);

	//登录
    } else if (strcmp(packet_type, "USER_LOGIN") == 0) {

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

	//后台直接登录
	if (strcmp(packet_type, "DELIVERY_CENTER_LOGIN") == 0) {
	    ad_system_packet_delivery_center_login(r, &json_data);
	    json_object_put(json_root);
	    return 0;
	}

	//管理中心直接登录
	if (strcmp(packet_type, "MA_CHECK") == 0) {

	    int ret = ad_system_packet_manager_account_check(r, ad_system_json_packet_value_parse(json_root, "DATA"));
	    json_object_put(json_root);
	    return ret;

	} else if (strcmp(packet_type, "MA_SESSION") == 0) {

	    int ret = ad_system_packet_manager_account_session(r, ad_system_json_packet_value_parse(json_root, "DATA"));
	    json_object_put(json_root);
	    return ret;

	} else if (strcmp(packet_type, "MA_LOGIN") == 0) {

	    int ret = ad_system_packet_manager_account_login(r, ad_system_json_packet_value_parse(json_root, "DATA"));
	    json_object_put(json_root);
	    return ret;

	} else if (strcmp(packet_type, "MA_LOGOUT") == 0) {

	    int ret = ad_system_packet_manager_account_logout(r, ad_system_json_packet_value_parse(json_root, "DATA"));
	    json_object_put(json_root);
	    return ret;
	}

	//获取session
	if (ngx_ad_system_get_session(r)) {
	    json_object_put(json_root);
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_CHECK_ERR);
	}

	//获取session匹配的account
	r->ad_system_account_id = ngx_pcalloc(r->pool, 128);
	if (ad_system_redis_get_login_session(r->ad_system_session_id, r->ad_system_account_id)) {
	    json_object_put(json_root);
	    return ad_system_packet_send_code(r, NULL, PACKET_CODE_USER_LOGIN_CHECK_ERR);
	}

	ad_system_packet_stdout(r);

	//登出
	if (strcmp(packet_type, "USER_LOGOUT") == 0) {
	    ad_system_packet_logout(r);

	    /********************/
	    /**** 首页 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_TITLE_INFO") == 0) {
	    ad_system_packet_get_title_info(r);

	} else if (strcmp(packet_type, "GET_ANNOUNCEMENT") == 0) {
	    ad_system_packet_get_announcement(r, &json_data);

	} else if (strcmp(packet_type, "GET_STAT_TREND") == 0) {
	    ad_system_packet_get_stat_trend(r, &json_data);

	} else if (strcmp(packet_type, "GET_STAT_TREND_TEXT") == 0) {
	    ad_system_packet_get_stat_trend_text(r, &json_data);

	} else if (strcmp(packet_type, "GET_STAT_TREND_IMG") == 0) {
	    ad_system_packet_get_stat_trend_img(r, &json_data);

	    /********************/
	    /**** 推广 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_REGION_CODE") == 0) {
	    ad_system_http_send_buffer(r, ad_system_global_region_code.buffer, ad_system_global_region_code.size);

	} else if (strcmp(packet_type, "GET_DISEASE_CODE") == 0) {
	    ad_system_packet_get_disease_code(r, &json_data);

	} else if (strcmp(packet_type, "GET_SPERAD_LIST") == 0) {
	    ad_system_packet_get_sperad_list(r, &json_data);

	    //计划
	} else if (strcmp(packet_type, "CAMPAIGN_NAME_CHECK") == 0) {
	    ad_system_packet_campaign_name_check(r, &json_data);

	} else if (strcmp(packet_type, "GET_CAMPAIGN") == 0) {
	    ad_system_packet_get_campaign(r, &json_data);

	} else if (strcmp(packet_type, "SET_CAMPAIGN") == 0) {
	    ad_system_packet_set_campaign(r, &json_data);

	} else if (strcmp(packet_type, "ADD_CAMPAIGN") == 0) {
	    ad_system_packet_add_campaign(r, &json_data);

	} else if (strcmp(packet_type, "DEL_CAMPAIGN") == 0) {
	    ad_system_packet_del_campaign(r, &json_data);

	} else if (strcmp(packet_type, "GET_CAMPAIGN_UNIQUE") == 0) {
	    ad_system_packet_get_campaign_unique(r, &json_data);

	    //组
	} else if (strcmp(packet_type, "GROUP_NAME_CHECK") == 0) {
	    ad_system_packet_group_name_check(r, &json_data);

	} else if (strcmp(packet_type, "GET_GROUP") == 0) {
	    ad_system_packet_get_group(r, &json_data);

	} else if (strcmp(packet_type, "ADD_GROUP_KEYWORD") == 0) {
	    ad_system_packet_add_group_keyword(r, &json_data);

	} else if (strcmp(packet_type, "GET_GROUP_KEYWORD") == 0) {
	    ad_system_packet_get_group_keyword(r, &json_data);

	} else if (strcmp(packet_type, "GET_REPORT_GROUP_KEYWORD") == 0) {
	    ad_system_packet_get_report_group_keyword(r, &json_data);

	} else if (strcmp(packet_type, "DEL_GROUP_KEYWORD") == 0) {
	    ad_system_packet_del_group_keyword(r, &json_data);

	} else if (strcmp(packet_type, "SET_GROUP") == 0) {
	    ad_system_packet_set_group(r, &json_data);

	} else if (strcmp(packet_type, "ADD_GROUP") == 0) {
	    ad_system_packet_add_group(r, &json_data);

	} else if (strcmp(packet_type, "DEL_GROUP") == 0) {
	    ad_system_packet_del_group(r, &json_data);

	} else if (strcmp(packet_type, "GET_GROUP_UNIQUE") == 0) {
	    ad_system_packet_get_group_unique(r, &json_data);

	    //文字创意
	} else if (strcmp(packet_type, "GET_CREATIVE") == 0) {
	    ad_system_packet_get_creative_text(r, &json_data);

	} else if (strcmp(packet_type, "SET_CREATIVE") == 0) {
	    ad_system_packet_set_creative_text(r, &json_data);

	} else if (strcmp(packet_type, "ADD_CREATIVE") == 0) {
	    ad_system_packet_add_creative_text(r, &json_data);

	} else if (strcmp(packet_type, "DEL_CREATIVE") == 0) {
	    ad_system_packet_del_creative_text(r, &json_data);

	    //图片创意
	} else if (strcmp(packet_type, "GET_IMG_CREATIVE") == 0) {
	    ad_system_packet_get_img_creative(r, &json_data);

	} else if (strcmp(packet_type, "SET_IMG_CREATIVE") == 0) {
	    ad_system_packet_set_creative_img(r, &json_data);

	} else if (strcmp(packet_type, "ADD_IMG_CREATIVE") == 0) {
	    ad_system_packet_add_creative_img(r, &json_data);

	} else if (strcmp(packet_type, "DEL_IMG_CREATIVE") == 0) {
	    ad_system_packet_del_creative_img(r, &json_data);

	} else if (strcmp(packet_type, "SET_UPLOAD_IMG_INFO") == 0) {
	    ad_system_packet_set_upload_img_info (r, &json_data);

	    //附加创意
	} else if (strcmp(packet_type, "GET_CREATIVE_APPEND") == 0) {
	    ad_system_packet_get_creative_append(r, &json_data);

	} else if (strcmp(packet_type, "SET_CREATIVE_APPEND") == 0) {
	    ad_system_packet_set_creative_append(r, &json_data);

	} else if (strcmp(packet_type, "ADD_CREATIVE_APPEND") == 0) {
	    ad_system_packet_add_creative_append(r, &json_data);

	} else if (strcmp(packet_type, "DEL_CREATIVE_APPEND") == 0) {
	    ad_system_packet_del_creative_append(r, &json_data);

	} else if (strcmp(packet_type, "GET_CREATIVE_APPEND_UNIQUE") == 0) {
	    ad_system_packet_get_creative_append_unique(r, &json_data);

	    //批量操作
	} else if (strcmp(packet_type, "BATCH_OPERATION") == 0) {
	    ad_system_packet_batch_operation(r, &json_data);

	    //批量复制创意
	} else if (strcmp(packet_type, "COPY_CREATIVE") == 0) {
	    ad_system_packet_copy_creative(r, &json_data);

	    /********************/
	    /**** 品牌广告****/
	    /********************/

	} else if (strcmp(packet_type, "GET_BRAND") == 0) {
	    ad_system_packet_get_brand(r, &json_data);

	} else if (strcmp(packet_type, "ADD_BRAND") == 0) {
	    ad_system_packet_add_brand(r, &json_data);

	} else if (strcmp(packet_type, "DEL_BRAND") == 0) {
	    ad_system_packet_del_brand(r, &json_data);

	} else if (strcmp(packet_type, "GET_BRAND_CREATIVE") == 0) {
	    ad_system_packet_get_brand_creative(r, &json_data);

	} else if (strcmp(packet_type, "ADD_BRAND_CREATIVE") == 0) {
	    ad_system_packet_add_brand_creative(r, &json_data);

	} else if (strcmp(packet_type, "SET_BRAND_CREATIVE") == 0) {
	    ad_system_packet_set_brand_creative(r, &json_data);

	    /********************/
	    /**** 报告 ****/
	    /********************/

	} else if (strcmp(packet_type, "REPORT_STANDARD_CREATE") == 0) {
	    ad_system_packet_report_standard(r, &json_data);

	} else if (strcmp(packet_type, "REPORT_COMPLEX_CREATE") == 0) {
	    ad_system_packet_report_complex(r, &json_data);

	} else if (strcmp(packet_type, "GET_REPORT_LIST") == 0) {
	    ad_system_packet_get_report_list(r, &json_data);

	} else if (strcmp(packet_type, "DET_REPORT") == 0) {
	    ad_system_packet_del_report(r, &json_data);


	    /********************/
	    /**** 财务管理 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_MONEY_TREND") == 0) {
	    // ad_system_packet_get_money_trend(r, &json_data);

	} else if (strcmp(packet_type, "GET_MONEY_RECHARGE") == 0) {
	    ad_system_packet_get_money_recharge(r, &json_data);

	    /********************/
	    /**** 账户信息 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_USER_INFO") == 0) {
	    ad_system_packet_get_user_info(r, &json_data);

	} else if (strcmp(packet_type, "SET_USER_INFO") == 0) {
	    ad_system_packet_set_user_info(r, &json_data);

	} else if (strcmp(packet_type, "SET_USER_PASS") == 0) {
	    ad_system_packet_set_user_pass(r, &json_data);

	} else {

	    ad_system_packet_send_code(r, NULL, PACKET_CODE_FORMAT_ERR);
	}
    }

    json_object_put(json_root);
    return 0;
}


