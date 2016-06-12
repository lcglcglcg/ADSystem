
#include "lg_string.h"
#include "ad_system_packet.h"
#include "ad_system_json.h"
#include "ad_system_redis.h"
#include "ad_system_sql.h"

char *sql_user = "lcg";
char *sql_pass = "lcg";
// char *sql_host = "192.168.49.106/xe";
char *sql_host = "188.188.3.152/dev";

//返回CODE
void ad_system_packet_send_code(ngx_http_request_t *r, int code)
{

    char *p_buffer = ngx_pcalloc(r->pool, 128);
    size_t size = sprintf(p_buffer, "{\"CODE\":\"%u\"}", code);
    return lg_ngx_network_buffer_post_send(r, NULL, p_buffer, size);
}

//登录
void ad_system_packet_login(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

#if 0
    // 获取session
    if (lg_ngx_network_get_session(r, "39-KEY", &r->session_id))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_IMG_ERR);

    char img_result[128] = {0};
    if (ad_system_redis_get_var_code(r->session_id, img_result))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_IMG_ERR);

    if (json->VAR_CODE && strcmp(json->VAR_CODE, img_result) != 0)
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_IMG_ERR);
#endif

    if (ad_system_sql_login(sql_conn, json->ACCOUNT, json->PASSWORD))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    char *account_id = lg_ocilib_get_string(sql_conn, 1);
    char *status = lg_ocilib_get_string(sql_conn, 2);

    if (!account_id || !status)
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    if (strcmp(status, "1") != 0)
	return ad_system_packet_send_code(r, PACKET_CODE_USER_FROZEN);

    //更新session
    char session_id[128];
    lg_string_uuid(session_id);
    lg_ngx_network_set_session(r, "39-KEY", session_id);
    ad_system_redis_set_login_session(session_id, account_id);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//检测是否登录
void ad_system_packet_login_check(ngx_http_request_t *r)
{

    // 获取session
    if (lg_ngx_network_get_session(r, "39-KEY", &r->session_id))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_ERR);

    if (ad_system_redis_get_login_session(r->session_id, NULL))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_OK);
}

//发送验证码
void ad_system_packet_captcha(ngx_http_request_t *r)
{

    lg_captcha_t img;
    lg_captcha_create(&img);

    lg_ngx_network_t network;
    network.head = NULL;
    network.next = NULL;
    network.content_type.data = LG_NGX_URI_PNG_STR;
    network.content_type.len = LG_NGX_URI_PNG_SIZE;
    return lg_ngx_network_buffer_post_send(r, &network, img.buffer, img.size);
}

//登出
void ad_system_packet_logout(ngx_http_request_t *r)
{
    ad_system_redis_del_login_session(r->session_id);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//获取右上角标题
void ad_system_packet_get_title_info(ngx_http_request_t *r, lg_ocilib_t *sql_conn)
{
    if ((ad_system_sql_get_title_info(sql_conn, r->account_id)))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[40960];
    size_t size = lg_json_sprintf(buffer, "{", "}", 11,

	    "%s",    "CODE", "0",
	    "%s",    "ACCOUNT_ID", 	lg_ocilib_get_string(sql_conn, 1),
	    "%s",    "USER_LOGIN_NAME", lg_ocilib_get_string(sql_conn, 2),
	    "%s",    "COMPANY_NAME",    lg_ocilib_get_string(sql_conn, 3),
	    "%f",    "BALANCE",         lg_ocilib_get_double(sql_conn, 4),
	    "%f",    "INVEST",          lg_ocilib_get_double(sql_conn, 5),
	    "%f",    "CONSUMPTION",     lg_ocilib_get_double(sql_conn, 6),
	    "%s",    "CREATE_TIME",     lg_ocilib_get_string(sql_conn, 7),
	    "%s",    "DOMAIN",          lg_ocilib_get_string(sql_conn, 8),
	    "%s",    "GUAHAO_TOKEN",    "",
	    "%s",    "WEBIM",           "",
	    "%s",    "PLACE_REGION",    "");

    return lg_ngx_network_buffer_post_send(r, NULL, buffer, size);
}

//首页->获取公告
void ad_system_packet_get_announcement(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
}

//首页->曲线图
void ad_system_packet_get_stat_trend(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
}

//首页->曲线图->TEXT
void ad_system_packet_get_stat_trend_text(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
}

//首页->曲线图->IMG
void ad_system_packet_get_stat_trend_img(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
}

//推广->获取地域CODE
void ad_system_packet_get_region_code(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_region_code(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char comma[4] = "{";
    char buffer[40960];
    while(lg_ocilib_fetch_next(sql_conn)) {

	size_t size = sprintf(buffer, "%s\"%s\":\"%s\"", 
		comma, 
		lg_ocilib_get_string(sql_conn, 1),
		lg_ocilib_get_string(sql_conn, 2));

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	comma[0] = ',';
    }

    lg_ngx_network_buffer_add(r, &network, "}", 1);
    lg_ngx_network_post_send(r, &network);
}

//推广->获取科室病种CODE
void ad_system_packet_get_disease_code(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_disease_code(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char comma[4] = "{";
    char buffer[40960];
    while(lg_ocilib_fetch_next(sql_conn)) {

	size_t size = sprintf(buffer, "%s\"%s\":\"%s\"", 
		comma, 
		lg_ocilib_get_string(sql_conn, 1),
		lg_ocilib_get_string(sql_conn, 2));

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	comma[0] = ',';
    }

    lg_ngx_network_buffer_add(r, &network, "}", 1);
    lg_ngx_network_post_send(r, &network);
}

//推广->左侧列表
void ad_system_packet_get_sperad_list(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    char buffer[4096];
    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_sperad_list(sql_conn, r->account_id, json->DISPLAY_STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    int size = sprintf(buffer, "{\"CODE\":\"0\",\"CAMPAIGN_LIST\":[");
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = {0};
    char comma_group[4] = {0};
    const char *CAMPAIGN_ID = NULL;
    char LAST_CAMPAIGN_ID[1024] = {0};
    while(lg_ocilib_fetch_next(sql_conn)) {

	CAMPAIGN_ID = lg_ocilib_get_string(sql_conn, 1);
	const char *CAMPAIGN_NAME = lg_ocilib_get_string(sql_conn, 2);
	int CAMPAIGN_STATUS = lg_ocilib_get_number(sql_conn, 3);
	const char *GROUP_ID = lg_ocilib_get_string(sql_conn, 4);
	const char *GROUP_NAME = lg_ocilib_get_string(sql_conn, 5);

	int GROUP_STATUS = 0;
	if (CAMPAIGN_STATUS == 1) {

	    GROUP_STATUS = 2;

	} else {

	    GROUP_STATUS = lg_ocilib_get_number(sql_conn, 6);
	    if (GROUP_STATUS == 21)
		GROUP_STATUS = 1;
	    else if (GROUP_STATUS == 22)
		GROUP_STATUS = 2;
	}

	if (strcmp(LAST_CAMPAIGN_ID, CAMPAIGN_ID) != 0) {

	    strcpy(LAST_CAMPAIGN_ID, CAMPAIGN_ID);
	    size = sprintf(buffer, "%s{\"CAMPAIGN_ID\":\"%s\",\"CAMPAIGN_NAME\":\"%s\",\"CAMPAIGN_STATUS\":\"%u\",\"GROUP_LIST\":[",
		    comma,
		    CAMPAIGN_ID,
		    CAMPAIGN_NAME,
		    CAMPAIGN_STATUS);
	    lg_ngx_network_buffer_add(r, &network, buffer, size);
	    comma_group[0] = 0;
	}

	if (GROUP_ID && *GROUP_ID) {

	    size = sprintf(buffer, "%s{\"GROUP_ID\":\"%s\", \"GROUP_NAME\":\"%s\", \"GROUP_STATUS\":\"%u\"}",
		    comma_group,
		    GROUP_ID,
		    GROUP_NAME,
		    GROUP_STATUS);
	    lg_ngx_network_buffer_add(r, &network, buffer, size);
	}

	strcpy(comma, "]},");
	comma_group[0] = ',';
    }

    if (CAMPAIGN_ID) {

	lg_ngx_network_buffer_add(r, &network, "]}", 2);
    }

    lg_ngx_network_buffer_add(r, &network, "]}", 2);
    lg_ngx_network_post_send(r, &network);
}

//推广->计划->重名验证
void ad_system_packet_campaign_name_check(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_campaign_name_check(sql_conn, r->account_id, json->CAMPAIGN_NAME, json->DISPLAY_STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_NAME_REPEAT);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//推广->计划->添加
void ad_system_packet_add_campaign(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_campaign_name_check(sql_conn, r->account_id, json->CAMPAIGN_NAME, json->DISPLAY_STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_NAME_REPEAT);

    if (json->CAMPAIGN_BUDGET < 50)
	return ad_system_packet_send_code(r, PACKET_CODE_BUDGET_ERR);

    if (ad_system_sql_create_campaign_id(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *campaign_id = lg_ocilib_get_string(sql_conn, 1);
    if (!campaign_id) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_sql_add_campaign(sql_conn,
		json->ACCOUNT_ID,
		campaign_id,
		json->CAMPAIGN_NAME,
		json->CAMPAIGN_BUDGET,
		json->REGION_TYPE,
		json->REGION_SERIALIZE,
		json->SCHEDULE_TYPE,
		json->SCHEDULE_SERIALIZE,
		json->BEGIN_TIME,
		json->END_TIME,
		json->DISPLAY_STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[256];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"CAMPAIGN_ID\":\"%s\"}", campaign_id);
    lg_ngx_network_buffer_post_send(r, NULL, buffer, size);
}

//推广->计划->获取
void ad_system_packet_get_campaign(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_campaign(sql_conn,
		r->account_id, 
		json->DISPLAY_STATUS,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    //发送数据
    char buffer[40960];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\","
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
	    100, //CAMPAIGN_COUNT,
	    0, 	//CAMPAIGN_ENABLED,
	    0.00, //CAMPAIGN_BUDGET,
	    0, //CAMPAIGN_IMPRESSION,
	    0, //CAMPAIGN_CLICK,
	    0.00, //CAMPAIGN_CTR * 100,
	    0.00, //CAMPAIGN_CPC,
	    0.00, //CAMPAIGN_COST,
	    0.00); //CAMPAIGN_CPM

    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(sql_conn)) {

	size = lg_json_sprintf(buffer, comma, "}", 16,
		"%s", 		       "CAMPAIGN_ID",          	lg_ocilib_get_string(sql_conn, 1),
		"%s", 		       "CAMPAIGN_NAME",        	lg_ocilib_get_string(sql_conn, 2),
		"%s", 		       "CAMPAIGN_STATUS",      	lg_ocilib_get_string(sql_conn, 3),
		"%s", 		       "CAMPAIGN_BUDGET",      	lg_ocilib_get_string(sql_conn, 4),
		"%s", 		       "SCHEDULE_TYPE",        	lg_ocilib_get_string(sql_conn, 5),
		"%s", 		       "SCHEDULE_SERIALIZE",   	lg_ocilib_get_string(sql_conn, 6),
		"%s", 		       "REGION_TYPE",          	lg_ocilib_get_string(sql_conn, 7),
		"%s", 		       "REGION_SERIALIZE",     	lg_ocilib_get_string(sql_conn, 8),
		"%s", 		       "BEGIN_TIME",           	lg_ocilib_get_string(sql_conn, 9),
		"%s", 		       "END_TIME",             	lg_ocilib_get_string(sql_conn, 10),
		"%d", 		       "CAMPAIGN_IMPRESSION",  	0,
		"%d", 		       "CAMPAIGN_CLICK",       	0,
		"%e", 		       "CAMPAIGN_CTR",         	0.00,
		"%f", 		       "CAMPAIGN_CPC",         	0.00,
		"%f", 		       "CAMPAIGN_COST",        	0.00,
		"%f", 		       "CAMPAIGN_CPM",         	0.00);
	lg_ngx_network_buffer_add(r, &network, buffer, size);
	strcpy(comma, ",{");
    }

    lg_ngx_network_buffer_add(r, &network, "]}", 2);
    lg_ngx_network_post_send(r, &network);
}

//推广->计划->修改
void ad_system_packet_set_campaign(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
}

//推广->计划->删除
void ad_system_packet_del_campaign(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_del_campaign(sql_conn, json->CAMPAIGN_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//推广->组->重名验证
void ad_system_packet_group_name_check(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_group_name_check(sql_conn, json->CAMPAIGN_ID, json->DISPLAY_STATUS, json->GROUP_NAME))
	return ad_system_packet_send_code(r, PACKET_CODE_NAME_REPEAT);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//推广->组->添加
void ad_system_packet_add_group(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_create_group_id(sql_conn)) 
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *group_id = lg_ocilib_get_string(sql_conn, 1);
    if (!group_id) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_sql_add_group(sql_conn,
		json->ACCOUNT_ID,
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		group_id,
		json->GROUP_NAME,
		json->GROUP_PRICE_TYPE,
		json->GROUP_CPC_PRICE,
		json->GROUP_CPM_PRICE,
		json->GROUP_TARGETING_TYPE,
		json->GROUP_KEYWORD,
		json->GROUP_DISEASE))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//推广->组->获取
void ad_system_packet_get_group(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_group(sql_conn,
		r->account_id, 
		json->DISPLAY_STATUS,
		json->FIND_TYPE,
		json->CAMPAIGN_ID,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[40960];
    int size = sprintf(buffer, "{\"CODE\":\"0\","
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
	    100, 	//GROUP_COUNT,
	    0,		//GROUP_ENABLED,
	    0,		//GROUP_IMPRESSION,
	    0,		//GROUP_CLICK,
	    0.00,	//GROUP_CTR * 100,
	    0.00,	//GROUP_CPC,
	    0.00,	//GROUP_COST,
	    0.00);	//GROUP_CPM);

    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(sql_conn)) {

	size = lg_json_sprintf(buffer, comma, "}", 14,

		"%s", "CAMPAIGN_ID",        		lg_ocilib_get_string(sql_conn, 1), 
		"%s", "CAMPAIGN_NAME",                  lg_ocilib_get_string(sql_conn, 2),
		"%s", "GROUP_ID",                       lg_ocilib_get_string(sql_conn, 3),
		"%s", "GROUP_NAME",                     lg_ocilib_get_string(sql_conn, 4),
		"%s", "GROUP_STATUS",                   lg_ocilib_get_string(sql_conn, 5),
		"%s", "GROUP_CPC_PRICE",                lg_ocilib_get_string(sql_conn, 6),
		"%s", "GROUP_CPM_PRICE",                lg_ocilib_get_string(sql_conn, 7),
		"%s", "GROUP_TARGETING_TYPE",           lg_ocilib_get_string(sql_conn, 8),
		"%u", "GROUP_IMPRESSION",    		0,
		"%u", "GROUP_CLICK",         		0,
		"%e", "GROUP_CTR",           		0.00,
		"%f", "GROUP_CPC",           		0.00,
		"%f", "GROUP_COST",          		0.00,
		"%f", "GROUP_CPM",           		0.00);

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	strcpy(comma, ",{");
    }

    lg_ngx_network_buffer_add(r, &network, "]}", 2);
    lg_ngx_network_post_send(r, &network);
}

void ad_system_packet_add_group_keyword(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

#if 0
    if (!json->GROUP_ID || !json->GROUP_KEYWORD)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);


    if (ad_system_sql_connect(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[40960];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(json->GROUP_KEYWORD, buffer, &save_ptr, ',');

    while(str) {

	ad_system_sql_add_group_keyword(sql_conn, r->account_id, json->GROUP_ID, str);
	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    ad_system_sql_commit(sql_conn);

    ad_system_sql_group_synch_state(r->account_id, json->GROUP_ID, NULL);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
#endif
}

void ad_system_packet_get_group_keyword(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    lg_ngx_network_t network;
    memset(&network, 0, sizeof(lg_ngx_network_t));

    if (ad_system_sql_get_group_keyword(sql_conn, r->account_id,
		json->page_index_s,
		json->page_index_e,
		json->DISPLAY_STATUS,
		json->CAMPAIGN_ID,
		json->GROUP_ID,
		json->FIND_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char comma[4] = "{";
    char buffer[40960];
    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%d\",\"COUNT\":\"%s\"},\"LIST\":[",
	    json->PAGE_INDEX, 
	    lg_ocilib_get_string(sql_conn, 1));
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    while(lg_ocilib_fetch_next(sql_conn)) {

	size = lg_json_sprintf(buffer, comma, "}", 7,
		"%s", "CAMPAIGN_ID",      lg_ocilib_get_string(sql_conn, 2),
		"%s", "CAMPAIGN_NAME",    lg_ocilib_get_string(sql_conn, 3),
		"%s", "GROUP_ID",         lg_ocilib_get_string(sql_conn, 4),
		"%s", "GROUP_NAME",       lg_ocilib_get_string(sql_conn, 5),
		"%s", "GROUP_CPC_PRICE",  lg_ocilib_get_string(sql_conn, 6),
		"%s", "GROUP_CPM_PRICE",  lg_ocilib_get_string(sql_conn, 7),
		"%s", "GROUP_KEYWORD",    lg_ocilib_get_string(sql_conn, 8));
	lg_ngx_network_buffer_add(r, &network, buffer, size);
	strcpy(comma, ",{");
    }

    lg_ngx_network_buffer_add(r, &network, "]}", 2);
    lg_ngx_network_post_send(r, &network);
}

//推广->获取文字创意
void  ad_system_packet_get_creative_text(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;


    if (json->FIND_STATUS == 6) {

	if (ad_system_sql_get_creative_del_text(sql_conn,
		    r->account_id, 
		    json->FIND_TYPE, 
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    } else {

	if (ad_system_sql_get_creative_text(sql_conn, 
		    r->account_id, 
		    json->FIND_TYPE, 
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    //获取mongo数据
    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_DAY_TEMP, 2, r->account_id, "creative_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_YESTERDAY_TEMP, 2, r->account_id, "creative_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_SEVENDAY_TEMP, 2, r->account_id, "creative_id");
    else
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    //合并数据
    lg_list_t merger_head;
    ad_system_merger_creative_info_t info;
    ad_system_merger_creative_text(
	    sql_conn, 
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
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    int page_index = 1;
    char comma[4] = "{";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_creative_text_t *node = NULL;
	node = (ad_system_merger_creative_text_t *)p;

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_json_sprintf(buffer, comma, "}", 21,
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

	    lg_ngx_network_buffer_add(r, &network, buffer, size);
	    strcpy(comma, ",{");
	}

	page_index++;
    }

    max += 2;
    lg_ngx_network_buffer_add(r, &network, buffer, size);


    ad_system_merger_free(&merger_head);
    return lg_ngx_network_post_send(r, &network);
}

#if 0
void  ad_system_packet_set_creative_text(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{
    if (!json->UPDATE_TYPE)
	return ad_system_packet_send_code(r, PACKET_CODE_OK);

    if (json->UPDATE_TYPE == 1) {

	if (ad_system_sql_set_creative_text(json->CREATIVE_ID, json->STATUS))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	if (ad_system_sql_del_creative_text(json->CREATIVE_ID))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_add_creative_text(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    char campaign_id[1024];
    if (ad_system_sql_get_campaign_id_from_group_or_creative(campaign_id, (char *)json->GROUP_ID, NULL))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_sql_add_creative_text(
		r->account_id,
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
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//删除文字创意
void  ad_system_packet_del_creative_text(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_del_creative_text(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_get_img_creative(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[40960];
    size_t size, max = 0;


    if (json->FIND_STATUS == 6) {

	if (ad_system_sql_get_creative_del_img(sql_conn,
		    r->account_id, 
		    json->FIND_TYPE,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    } else {

	if (ad_system_sql_get_img_creative(sql_conn,
		    r->account_id, 
		    json->FIND_TYPE,
		    json->FIND_STATUS,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID,
		    json->FIND_NAME_TYPE,
		    json->FIND_NAME))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    //获取mongo数据
    lg_list_t mongo_head;
    if (json->DATE_TYPE == 1)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_DAY_TEMP, 1, r->account_id, "creative_id");
    else if (json->DATE_TYPE == 2)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_YESTERDAY_TEMP, 1, r->account_id, "creative_id");
    else if (json->DATE_TYPE == 3)
	ad_system_mongo_stat_list_load(&mongo_head, CREATIVE_SEVENDAY_TEMP, 1, r->account_id, "creative_id");
    else
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    //合并数据
    lg_list_t merger_head;
    ad_system_merger_creative_info_t info;
    ad_system_merger_creative_img(
	    sql_conn,
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
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    int page_index = 1;
    char  comma[4] = "{";
    lg_list_t *p = NULL;
    lg_list_for_each(p, &merger_head) {

	ad_system_merger_creative_img_t *node = NULL;
	node = (ad_system_merger_creative_img_t*)p;

	if (page_index >= json->page_index_s && page_index <= json->page_index_e) {

	    max += size = lg_json_sprintf(buffer, comma, "}", 22,
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
	    lg_ngx_network_buffer_add(r, &network, buffer, size);

	    strcpy(comma, ",{");
	}

	page_index++;
    }

    max += 2;
    lg_ngx_network_buffer_add(r, &network, buffer, size);


    ad_system_merger_free(&merger_head);
    return lg_ngx_network_post_send(r, &network);
}

void  ad_system_packet_set_upload_img_info (ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    redisContext *conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
    if (!conn) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    redisReply *reply = redisCommand(conn, "ZRANGE %s 0 -1", json->CREATIVE_IMG_ID);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	freeReplyObject(reply);
	redisFree(conn);
	return ad_system_packet_send_code(r, PACKET_CODE_IMG_ID_ERR);
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
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_add_creative_img(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    char buffer[40960];
    char *save_ptr = NULL;
    char *img_id = lg_string_strtok(json->CREATIVE_IMG_ID, buffer, &save_ptr, ',');

    while(img_id) {

	//查询上传的图片信息
	redisContext *conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
	if (!conn) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	redisReply *reply = redisCommand(conn, "ZRANGE %s 0 -1", img_id);
	if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	    freeReplyObject(reply);
	    redisFree(conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_IMG_ID_ERR);
	}

	//查询所属组与计划
	char campaign_id[1024];
	if (ad_system_sql_get_campaign_id_from_group_or_creative(campaign_id, (char *)json->GROUP_ID, NULL))
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	//创建新的图片创意
	if (ad_system_sql_add_creative_img(
		    r->account_id,
		    campaign_id,
		    json->GROUP_ID,
		    reply->element[0]->str,//name
		    reply->element[1]->str,//img_size
		    reply->element[2]->str,//img_buffer_size
		    reply->element[3]->str,//img_path
		    json->CREATIVE_DESTINATION_URL,
		    json->CREATIVE_DISPLAY_URL)) {

	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	img_id = lg_string_strtok(NULL, buffer, &save_ptr, ',');
	freeReplyObject(reply);
	redisFree(conn);
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//删除图片创意
void  ad_system_packet_del_creative_img(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_del_creative_img(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//获取充值记录
void  ad_system_packet_get_money_recharge(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_sql_t sql_count;
    if (ad_system_sql_get_money_recharge_count(&sql_count,
		r->account_id,
		json->FIND_TYPE,
		json->MONEY_TYPE,
		json->FIND_NAME_TYPE,
		json->FIND_NAME,
		json->TIME_START,
		json->TIME_END))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    ad_system_sql_t sql_data;
    if (ad_system_sql_get_money_recharge(&sql_data,
		r->account_id,
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


	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (!(lg_ocilib_fetch_next(&sql_count))) {



	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%u\"},\"LIST\":[", 
	    json->PAGE_INDEX, 
	    ad_system_sql_get_number(&sql_count, NULL, 1));
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(&sql_data)) {

	max += size = lg_json_sprintf(buffer, comma, "}", 8,

		"%s", "SWIFT_NUMBER",			lg_ocilib_get_string(&sql_data, NULL, 1),
		"%s", "MONEY",                          lg_ocilib_get_string(&sql_data, NULL, 2),
		"%s", "PAYMENT_TYPE",                   lg_ocilib_get_string(&sql_data, NULL, 3),
		"%s", "ACCOUNTING_DOCUMENT_NUMBER",     lg_ocilib_get_string(&sql_data, NULL, 4),
		"%s", "ACCOUNTING_DOCUMENT_FILE",       lg_ocilib_get_string(&sql_data, NULL, 5),
		"%s", "INVEST",                         lg_ocilib_get_string(&sql_data, NULL, 6),
		"%s", "BALANCE",                        lg_ocilib_get_string(&sql_data, NULL, 7),
		"%s", "CREATE_TIME",                    lg_ocilib_get_string(&sql_data, NULL, 8));

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	strcpy(comma, ",{");
    }




    max += 2;
    lg_ngx_network_buffer_add(r, &network, buffer, size);
    return lg_ngx_network_post_send(r, &network);
}

void  ad_system_packet_get_creative_append(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    char buffer[4096];
    size_t size, max = 0;


    if (ad_system_sql_get_creative_append(sql_conn,
		r->account_id, 
		json->FIND_TYPE,
		json->SORT_TYPE,
		json->SORT_COLUMN,
		json->FIND_NAME,
		json->FIND_STATUS))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":[");
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(sql_conn)) {

	max += size = lg_json_sprintf(buffer, comma, "}", 4,
		"%s","APPEND_ID",		lg_ocilib_get_string(sql_conn, 1),
		"%s","APPEND_CONTENT",       lg_ocilib_get_string(sql_conn, 3),
		"%s","APPEND_STATUS",        lg_ocilib_get_string(sql_conn, 2),
		"%s","APPEND_REFUSE_REASON", lg_ocilib_get_string(sql_conn, 4));

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	strcpy(comma, ",{");
    }

    max += 2;
    lg_ngx_network_buffer_add(r, &network, buffer, size);


    return lg_ngx_network_post_send(r, &network);
}

void  ad_system_packet_set_creative_append(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_set_creative_append(
		r->account_id,
		json->CREATIVE_APPEND_ID,
		json->UPDATE_TYPE,
		json->APPEND_STATUS,
		json->APPEND_CONTENT))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_add_creative_append(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_add_creative_append(r->account_id, json->APPEND_CONTENT))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//删除附加创意
void ad_system_packet_del_creative_append(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_del_creative_append(json->CREATIVE_APPEND_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_get_creative_append_unique(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{


    if (ad_system_sql_get_creative_append_unique(sql_conn, json->CREATIVE_APPEND_ID))
	ad_system_packet_send_code(r, PACKET_CODE_OK);

    if (!lg_ocilib_fetch_next(sql_conn))
	ad_system_packet_send_code(r, PACKET_CODE_OK);

    char buffer[4096];
    size_t size = lg_json_sprintf(buffer, "{", "}", 5,
	    "%s", "CODE", "0",		
	    "%s", "APPEND_ID",		lg_ocilib_get_string(sql_conn, 1),
	    "%s", "APPEND_STATUS",            lg_ocilib_get_string(sql_conn, 2),
	    "%s", "APPEND_CONTENT",           lg_ocilib_get_string(sql_conn, 3),
	    "%s", "APPEND_REFUSE_REASON",     lg_ocilib_get_string(sql_conn, 4));


    return lg_ngx_network_buffer_post_send(r, buffer, size);
}

void ad_system_packet_get_user_info(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{


    if ((ad_system_sql_get_user_info(sql_conn, r->account_id)))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (!lg_ocilib_fetch_next(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char buffer[40960];
    size_t size = lg_json_sprintf(buffer, "{", "}", 20,

	    "%s", "CODE", "0",
	    "%s", "USER_LOGIN_NAME",    lg_ocilib_get_string(sql_conn, 1),
	    "%s", "COMPANY_NAME",       lg_ocilib_get_string(sql_conn, 2),
	    "%s", "COMPANY_SITE",       lg_ocilib_get_string(sql_conn, 3),
	    "%s", "INDUSTRY",          	lg_ocilib_get_string(sql_conn, 4),
	    "%s", "API_TOKEN",         	lg_ocilib_get_string(sql_conn, 5),
	    "%s", "PROVINCE_ID",       	lg_ocilib_get_string(sql_conn, 6),
	    "%s", "CITY_ID",           	lg_ocilib_get_string(sql_conn, 7),
	    "%s", "ADDRESS",           	lg_ocilib_get_string(sql_conn, 8),
	    "%s", "POSTCODE",          	lg_ocilib_get_string(sql_conn, 9),
	    "%s", "CONTACT",           	lg_ocilib_get_string(sql_conn, 10),  			
	    "%s", "TELEPHONE",         	lg_ocilib_get_string(sql_conn, 11),  			
	    "%s", "FAX",               	lg_ocilib_get_string(sql_conn, 12),
	    "%s", "MOBILE",            	lg_ocilib_get_string(sql_conn, 13),
	    "%s", "EMAIL",              lg_ocilib_get_string(sql_conn, 14),
	    "%s", "WEBIM",              lg_ocilib_get_string(sql_conn, 15),
	    "%s", "LATE_LOGIN_TIME",    lg_ocilib_get_string(sql_conn, 16),
	    "%s", "LATE_LOGIN_IP",      lg_ocilib_get_string(sql_conn, 17),
	    "%s", "LATE_LOGIN_ADDRESS", lg_ocilib_get_string(sql_conn, 18),
	    "%s", "PLACE_REGION", 	lg_ocilib_get_string(sql_conn, 19));


    return lg_ngx_network_buffer_post_send(r, buffer, size);
}

//设置用户信息
void ad_system_packet_set_user_info(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_set_user_info(sql_conn,
		r->account_id,
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
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//账户信息->修改密码
void ad_system_packet_set_user_pass(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (!json->PASS_OLD || !json->PASS_OLD)
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);


    if (ad_system_sql_get_user_pass(sql_conn, r->account_id))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (!lg_ocilib_fetch_next(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    const char *PASS_OLD = NULL;
    if (!(PASS_OLD = lg_ocilib_get_string(sql_conn, 1)))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (strcmp(json->PASS_OLD, PASS_OLD))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    if (ad_system_sql_set_user_pass(r->account_id, json->PASS_NEW))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

//创建标准报告
void ad_system_packet_report_standard(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_report_arg_t report_arg;
    ad_system_report_arg_t *p = &report_arg;
    memset(p, 0, sizeof(ad_system_report_arg_t));
    p->report_type = json->REPORT_TYPE;

    if (ad_system_report_path_create(p->report_key, p->report_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (json->REPORT_SPECIES & 1) p->display_style = 2;
    else p->display_style = 1;
    strcpy(p->account_id, r->account_id);

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

    if (ad_system_mongo_report_list_add(r->account_id,
		p->report_key,
		json->REPORT_NAME,
		json->REPORT_TYPE,
		p->display_style,
		time_range,
		down_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    ad_system_packet_send_code(r, PACKET_CODE_OK);
    ad_system_report_standard_create(p);
    return 0;
}

void ad_system_packet_report_complex(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    ad_system_report_arg_t report_arg;
    ad_system_report_arg_t *p = &report_arg;
    memset(p, 0, sizeof(ad_system_report_arg_t));
    p->report_type = json->REPORT_TYPE;

    if (ad_system_report_path_create(p->report_key, p->report_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (json->REPORT_SPECIES == 1) p->display_style = 2;
    else p->display_style = 1;
    strcpy(p->account_id, r->account_id);

    p->time_type = json->TIME_TYPE;
    strcpy(p->time_s, json->TIME_START);
    strcpy(p->time_e, json->TIME_END);

    char time_range[128];
    sprintf(time_range, "%s->%s", json->TIME_START, json->TIME_END);

    char down_path[128];
    sprintf(down_path, "%s/report.zip", p->report_path);

    if (ad_system_mongo_report_list_add(r->account_id,
		p->report_key,
		json->REPORT_NAME,
		json->REPORT_TYPE + 4,
		p->display_style,
		time_range,
		down_path))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    ad_system_packet_send_code(r, PACKET_CODE_OK);
    ad_system_report_complex_create(p);
    return 0;
}

void ad_system_packet_get_report_list(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{
    char buffer[40960];
    size_t size, max = 0;
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;

    lg_list_t head;
    if (ad_system_mongo_report_list_find(r->account_id, json->REPORT_BELONG, &head) != 0)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"LIST\":[");
    lg_ngx_network_buffer_add(r, &network, buffer, size);

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

	lg_ngx_network_buffer_add(r, &network, buffer, size);
	comma[0] = ',';
    }

    max += size = sprintf(buffer, "]}");
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    ad_system_mongo_list_free(&head);
    return lg_ngx_network_post_send(r, &network);
}

void ad_system_packet_del_report(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_mongo_report_list_del(r->account_id, json->REPORT_ID) != 0)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_batch_operation(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
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
	    if (ad_system_sql_get_campaign_budget(r->account_id, NULL, group_id, budget, bidding_model))
		return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	    if (json->GROUP_PRICE_TYPE & 1)
		if (atof(group_cpc_price) > atof(budget))
		    return ad_system_packet_send_code(r, PACKET_CODE_PRICE_EXPENSIVE);

	    if (json->GROUP_PRICE_TYPE & 2)
		if (atof(group_cpm_price) > atof(budget))
		    return ad_system_packet_send_code(r, PACKET_CODE_PRICE_EXPENSIVE);

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
	    if (ad_system_sql_get_campaign_budget(r->account_id, NULL, group_id, budget, bidding_model))
		return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	    ad_system_sql_ad_system_batch_operation_group_price(group_id,
		    json->GROUP_PRICE_TYPE,
		    group_cpc_price,
		    group_cpm_price);

	    group_id = lg_string_strtok(NULL, group_id_buffer, &group_id_save_ptr, ',');
	    group_cpc_price = lg_string_strtok(NULL, group_cpc_price_buffer, &group_cpc_price_save_ptr, ',');
	    group_cpm_price = lg_string_strtok(NULL, group_cpm_price_buffer, &group_cpm_price_save_ptr, ',');
	}

	return ad_system_packet_send_code(r, PACKET_CODE_OK);
    }

    char *save_ptr = NULL;
    char buffer[40960] = {0};
    char *id = lg_string_strtok(json->ID, buffer, &save_ptr, ',');
    while(id) {

	if (json->BATCH_TYPE == 1) {

	    switch(json->ID_TYPE){

		case 1:
		    ad_system_sql_set_campaign(r->account_id, id, 1, json->STATUS, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL);
		    break;
		case 2:
		    ad_system_sql_set_group(r->account_id, id, 1, json->STATUS, NULL, NULL, 0, 0, NULL);
		    break;
		case 3:
		    ad_system_sql_set_creative_text(id, json->STATUS);
		    break;
		case 4:
		    ad_system_sql_set_creative_append(r->account_id, id, 1, json->STATUS, NULL);
		    break;
		case 5:
		    ad_system_sql_set_creative_img(r->account_id, id, json->STATUS);
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

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_copy_creative(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_copy_creative_check(
		r->account_id,
		json->CAMPAIGN_ID,
		json->GROUP_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *save_ptr = NULL;
    char buffer[40960] = {0};
    char *creative_id = lg_string_strtok(json->ID, buffer, &save_ptr, ',');
    while(creative_id) {

	if (json->CREATIVE_TYPE == 1)
	    ad_system_sql_copy_creative_img_r(
		    creative_id,
		    r->account_id,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID);

	else if (json->CREATIVE_TYPE == 2)
	    ad_system_sql_copy_creative_text_r(
		    creative_id,
		    r->account_id,
		    json->CAMPAIGN_ID,
		    json->GROUP_ID);

	creative_id= lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_get_brand(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
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


    int ret = ad_system_sql_get_brand(sql_conn,
	    r->account_id,
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


	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return lg_ngx_network_buffer_post_send(r, buffer, size);

    } else if (ret != 0) {

	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    char count[128] = {0};
    if (ad_system_sql_get_brand_count(count,
		r->account_id,
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


	size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":\"{}\",\"LIST\":[]}");
	return lg_ngx_network_buffer_post_send(r, buffer, size);
    }

    max = size = sprintf(buffer, "{\"CODE\":\"0\",\"INFO\":{\"PAGE_INDEX\":\"%u\",\"COUNT\":\"%s\"},\"LIST\":[", json->PAGE_INDEX, count);
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(sql_conn)) {

	max += size = lg_json_sprintf(buffer, comma, "}", 15,
		"%s", "BRAND_ID", 		lg_ocilib_get_string(sql_conn, 1),
		"%s", "AD_ORDER_NO",            lg_ocilib_get_string(sql_conn, 2),
		"%s", "DEPT_CODE",              lg_ocilib_get_string(sql_conn, 3),
		"%s", "REGION_TYPE",            lg_ocilib_get_string(sql_conn, 4),
		"%s", "REGION_SERIALIZE",       lg_ocilib_get_string(sql_conn, 5),
		"%s", "DAYS",                   lg_ocilib_get_string(sql_conn, 6),
		"%s", "SHOW_TIME_START",        lg_ocilib_get_string(sql_conn, 7),
		"%s", "SHOW_TIME_END",          lg_ocilib_get_string(sql_conn, 8),
		"%s", "PRICE",                  lg_ocilib_get_string(sql_conn, 9),
		"%s", "CREATE_TIME",            lg_ocilib_get_string(sql_conn, 10),
		"%s", "BRAND_STATUS",           lg_ocilib_get_string(sql_conn, 11),
		"%s", "BRAND_REFUSE_REASON",    lg_ocilib_get_string(sql_conn, 12),
		"%s", "CREATIVE_STATE",         lg_ocilib_get_string(sql_conn, 13),
		"%s", "CREATIVE_REFUSE_REASON", lg_ocilib_get_string(sql_conn, 14),
		"%s", "INSPECTION_STATUS", 	lg_ocilib_get_string(sql_conn, 15)
		);

	strcpy(comma, ",{");
	lg_ngx_network_buffer_add(r, &network, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    lg_ngx_network_buffer_add(r, &network, buffer, size);


    return lg_ngx_network_post_send(r, &network);
}

void ad_system_packet_add_brand(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (json->REGION_SERIALIZE && strlen(json->REGION_SERIALIZE) > 2048)
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_REGION_LEN_ERR);

    if (json->REGION_SERIALIZE && strlen(json->DEPT_SERIALIZE) > 2048)
	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_REGION_LEN_ERR);

    if (json->PRICE && !(atoi(json->PRICE) > 0))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    if (ad_system_sql_add_brand(
		r->account_id,
		json->REGION_TYPE,
		json->REGION_SERIALIZE,
		json->SHOW_TIME_START,
		json->SHOW_TIME_END,
		json->DEPT_SERIALIZE,
		json->PRICE,
		r->account_id))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_del_brand(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (ad_system_sql_del_brand(json->BRAND_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_get_brand_creative(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    char buffer[40960];
    size_t size, max = 0;
    ngx_chain_t *send_head = NULL;
    ngx_chain_t *send_next = NULL;


    if (ad_system_sql_get_brand_creative(sql_conn, json->MOD_STATUS, json->BRAND_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (!lg_ocilib_fetch_next(sql_conn)) {


	return ad_system_packet_send_code(r, PACKET_CODE_BRAND_CREATIVE_ERR);
	//return ad_system_packet_send_code(r, PACKET_CODE_OK);
    }

    max = size = lg_json_sprintf(buffer, "{", ",\"BRAND_CREATIVE\":[", 16,
	    "%s", "CODE", "0",
	    "%s", "BRAND_ID",		lg_ocilib_get_string(sql_conn, 1),
	    "%s", "BRAND_CREATIVE_ID",  lg_ocilib_get_string(sql_conn, 2),
	    "%s", "TITLE",              lg_ocilib_get_string(sql_conn, 3),
	    "%s", "DESCRIPTION",        lg_ocilib_get_string(sql_conn, 4),
	    "%s", "DESTINATION_URL",    lg_ocilib_get_string(sql_conn, 5),
	    "%s", "DISPLAY_URL",        lg_ocilib_get_string(sql_conn, 6),
	    "%s", "TELEPHONE",          lg_ocilib_get_string(sql_conn, 7),
	    "%s", "WEB_SITE",           lg_ocilib_get_string(sql_conn, 8),
	    "%s", "WEB_IM",             lg_ocilib_get_string(sql_conn, 9),
	    "%s", "GUAHAO_URL",         lg_ocilib_get_string(sql_conn, 10),
	    "%s", "CREATIVE_STATE",     lg_ocilib_get_string(sql_conn, 11),
	    "%s", "REFUSE_REASON",      lg_ocilib_get_string(sql_conn, 12),
	    "%s", "CREATE_TIME",        lg_ocilib_get_string(sql_conn, 13),
	    "%s", "LAST_UPDATE_TIME",   lg_ocilib_get_string(sql_conn, 14),
	    "%s", "INSPECTION_STATUS,", lg_ocilib_get_string(sql_conn, 15));
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    max += size = lg_json_sprintf(buffer, "{", "},", 3,
	    "%s", "SEQUENCE",		lg_ocilib_get_string(sql_conn, 16),
	    "%s", "IMG_PATH",  		lg_ocilib_get_string(sql_conn, 17),
	    "%s", "IMG_URL",            lg_ocilib_get_string(sql_conn, 18));
    lg_ngx_network_buffer_add(r, &network, buffer, size);

    char comma[4] = "{";
    while(lg_ocilib_fetch_next(sql_conn)) {

	max += size = lg_json_sprintf(buffer, comma, "}", 3,
		"%s", "SEQUENCE",		lg_ocilib_get_string(sql_conn, 16),
		"%s", "IMG_PATH",  		lg_ocilib_get_string(sql_conn, 17),
		"%s", "IMG_URL",            lg_ocilib_get_string(sql_conn, 18));

	strcpy(comma, ",{");
	lg_ngx_network_buffer_add(r, &network, buffer, size);
    }

    max += size = sprintf(buffer, "]}");
    lg_ngx_network_buffer_add(r, &network, buffer, size);


    return lg_ngx_network_post_send(r, &network);
}

void ad_system_packet_add_brand_creative(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (!json->WEB_SITE
	    || !json->IMG0_URL
	    || !json->IMG1_URL
	    || !json->IMG2_URL
	    || !json->IMG3_URL
	    || !json->IMG4_URL
	    || !json->IMG5_URL)
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

    char domain[4096] = {0};
    if (ad_system_sql_get_account_domain(r->account_id, domain))
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);

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
	return ad_system_packet_send_code(r, PACKET_CODE_PARAMETER_ERR);
    }


    if (ad_system_sql_connect(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (ad_system_sql_add_brand_creative(sql_conn,
		json->INSERT_TYPE,
		json->BRAND_ID,
		r->account_id,
		json->TITLE,
		json->DESCRIPTION,
		json->DESTINATION_URL,
		json->DISPLAY_URL,
		json->TELEPHONE,
		json->WEB_SITE,
		json->WEB_IM,
		json->GUAHAO_URL)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    char img_size[128] = {0}, img_path[1024] = {0};

    if (ad_system_redis_get_img(json->IMG0_ID, NULL, img_size, NULL, img_path)) {


	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG0_URL,
		0)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_redis_get_img(json->IMG1_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG1_URL,
		1)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_redis_get_img(json->IMG2_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG2_URL,
		2)) {
	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_redis_get_img(json->IMG3_ID, NULL, img_size, NULL, img_path)) {
	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG3_URL,
		3)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_redis_get_img(json->IMG4_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG4_URL,
		4)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_redis_get_img(json->IMG5_ID, NULL, img_size, NULL, img_path)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    if (ad_system_sql_add_brand_creative_append(sql_conn,
		json->BRAND_ID, 
		img_path,
		img_size,
		json->IMG5_URL,
		5)) {

	ad_system_sql_rollback(sql_conn);
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    ad_system_sql_commit(sql_conn);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_set_brand_creative(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (!json->BRAND_CREATIVE_ID)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }

    char img_size[128] = {0}, img_path[1024] = {0};

    //IMG0_ID
    if (json->UPDATE_TYPE & 256) {

	if (ad_system_redis_get_img(json->IMG0_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->01\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG0_URL,
		    0)) {
	    fprintf(stderr, "--->02\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    //IMG1_ID
    if (json->UPDATE_TYPE & 1024) {

	if (ad_system_redis_get_img(json->IMG1_ID, NULL, img_size, NULL, img_path)) {
	    fprintf(stderr, "--->04\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG1_URL,
		    1)) {

	    fprintf(stderr, "--->05\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    //IMG2_ID
    if (json->UPDATE_TYPE & 4096) {

	if (ad_system_redis_get_img(json->IMG2_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->07\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG2_URL,
		    2)) {

	    fprintf(stderr, "--->08\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    //IMG3_ID
    if (json->UPDATE_TYPE & 16384) {

	if (ad_system_redis_get_img(json->IMG3_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->010\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG3_URL,
		    3)) {

	    fprintf(stderr, "--->011\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    //IMG4_ID
    if (json->UPDATE_TYPE & 65536) {

	if (ad_system_redis_get_img(json->IMG4_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->013\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG4_URL,
		    4)) {

	    fprintf(stderr, "--->014\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    //IMG5_ID
    if (json->UPDATE_TYPE & 262144) {

	if (ad_system_redis_get_img(json->IMG5_ID, NULL, img_size, NULL, img_path)) {

	    fprintf(stderr, "--->016\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}

	if (ad_system_sql_set_brand_creative_append(
		    1,
		    json->BRAND_CREATIVE_ID, 
		    img_path,
		    img_size,
		    json->IMG5_URL,
		    5)) {

	    fprintf(stderr, "--->017\n");
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
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
	    return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void  ad_system_packet_set_creative_img(ngx_http_request_t *r, lg_ocilib_t *sql_conn, ad_system_json_t *json)
{

    if (!json->UPDATE_TYPE)
	return ad_system_packet_send_code(r, PACKET_CODE_OK);

    if (json->UPDATE_TYPE & 1) {

	if (ad_system_sql_set_creative_img(
		    r->account_id,
		    json->CREATIVE_ID,
		    json->CREATIVE_STATUS))
	    return ad_system_packet_send_code(r, PACKET_CODE_OK);
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
	if (!conn) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

	reply = redisCommand(conn, "ZRANGE %s 0 -1", json->CREATIVE_IMG_ID);
	if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	    freeReplyObject(reply);
	    redisFree(conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_IMG_ID_ERR);
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
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    if (json->UPDATE_TYPE & 16) {

	freeReplyObject(reply);
	redisFree(conn);
    }

    //删除原有创意
    if (ad_system_sql_del_creative_img(json->CREATIVE_ID))
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_manager_account_check(ngx_http_request_t *r, const char *data)
{

    if (!data)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[buffer]%s\n", buffer);

    char *saveptr = NULL;
    char *account_name = strtok_r(buffer, ":", &saveptr);
    if (!account_name)return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *password = strtok_r(NULL, ":", &saveptr);
    if (!password)return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[MA][%s][%s]\n", account_name, password);

    if (ad_system_sql_login(sql_conn, account_name, password))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    if (!lg_ocilib_fetch_next(sql_conn))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    const char *account_id = lg_ocilib_get_string(sql_conn, 1);
    if (!account_id || !(*account_id))
	return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_ERR);

    size_t size = sprintf(buffer, "{\"CODE\":\"0\",\"ACCOUNT_ID\":\"%s\"}", account_id);


    return lg_ngx_network_buffer_post_send(r, buffer, size);
}

void ad_system_packet_manager_account_session(ngx_http_request_t *r, const char *data)
{

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *saveptr = NULL;
    char *session_id = strtok_r(buffer, ":", &saveptr);
    if (!session_id)return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *account_id = strtok_r(NULL, ":", &saveptr);
    if (!account_id)return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    // fprintf(stdout, "[MA][%s][%s]\n", session_id, account_id);

    ad_system_redis_set_login_session(session_id, account_id);
    return ad_system_packet_send_code(r, PACKET_CODE_OK);
}

void ad_system_packet_manager_account_login(ngx_http_request_t *r, const char *data)
{

    if (!data)
	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    ngx_ad_system_set_session(r, (char *)data);

    char buffer[1024];
    size_t size = sprintf(buffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=http://wangmeng.haomeit.com\"></html>");
    return lg_ngx_network_buffer_post_send(r, buffer, size);
}

void ad_system_packet_manager_account_logout(ngx_http_request_t *r, const char *data)
{

    if (!data)	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    int len = 0;
    char temp[4096] = {0};
    char buffer[4096] = {0};
    len = lg_string_url_decode(buffer, (char *)data, strlen(data));
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len = rsa_public_decrypt("/etc/rsa_pub.key", temp, buffer);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    len =lg_string_url_decode(buffer, temp, len);
    if (len == -1) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    ad_system_redis_del_login_session(buffer);
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
	    r->connection->fd, 
	    r->account_id, 
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
	    r->account_id, 
	    r->ad_system_buffer);
    fflush(stderr);
}
#endif

void ad_system_packet_parse(ngx_http_request_t *r, char *buffer, size_t size)
{


    lg_string_url_decode(buffer, buffer, size);
    fprintf(stdout, "==>%s\n", buffer);

    // json parse
    ad_system_json_t json;
    lg_json_t *json_root = ad_system_json_parse(&json, buffer);
    if (!json_root) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    char *packet_type = json.PACKET_TYPE;
    if (!packet_type) return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);

    // sql conn
    lg_ocilib_t sql_conn;
    if (lg_ocilib_conn(&sql_conn, sql_host, sql_user, sql_pass)) {

	return ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
    }


    fprintf(stdout, "ok\n");

    /********************/
    /**** 登录 ****/
    /********************/

    //登录
    if (strcmp(packet_type, "USER_LOGIN") == 0) {
	ad_system_packet_login(r, &sql_conn, &json);

	//检测登录
    } else if (strcmp(packet_type, "USER_LOGIN_CHECK") == 0) {
	ad_system_packet_login_check(r);

	//获取验证码
    } else if (strcmp(packet_type, "GET_CAPTCHA") == 0) {
	ad_system_packet_captcha(r);

	//后台直接登录
    } else if (strcmp(packet_type, "DELIVERY_CENTER_LOGIN") == 0) {


    } else {

	//获取session匹配的account id
	r->account_id = ngx_pcalloc(r->pool, 128);
	lg_ngx_network_get_session(r, "39-KEY", &r->session_id);
	if (ad_system_redis_get_login_session(r->session_id, r->account_id)) {

	    lg_json_free(json_root);
	    lg_ocilib_free(&sql_conn);
	    return ad_system_packet_send_code(r, PACKET_CODE_USER_LOGIN_CHECK_ERR);
	}

	//登出
	if (strcmp(packet_type, "USER_LOGOUT") == 0) {
	    ad_system_packet_logout(r);

	    /********************/
	    /**** 首页 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_TITLE_INFO") == 0) {
	    ad_system_packet_get_title_info(r, &sql_conn);

	} else if (strcmp(packet_type, "GET_ANNOUNCEMENT") == 0) {
	    ad_system_packet_get_announcement(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_STAT_TREND") == 0) {
	    ad_system_packet_get_stat_trend(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_STAT_TREND_TEXT") == 0) {
	    ad_system_packet_get_stat_trend_text(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_STAT_TREND_IMG") == 0) {
	    ad_system_packet_get_stat_trend_img(r, &sql_conn, &json);

	    /********************/
	    /**** 推广 ****/
	    /********************/

	    // 地域字典
	} else if (strcmp(packet_type, "GET_REGION_CODE") == 0) {
	    ad_system_packet_get_region_code(r, &sql_conn, &json);

	    //科室字典
	} else if (strcmp(packet_type, "GET_DISEASE_CODE") == 0) {
	    ad_system_packet_get_disease_code(r, &sql_conn, &json);

	    //左侧列表
	} else if (strcmp(packet_type, "GET_SPERAD_LIST") == 0) {
	    ad_system_packet_get_sperad_list(r, &sql_conn, &json);

	    //推广->计划重命名验证
	} else if (strcmp(packet_type, "CAMPAIGN_NAME_CHECK") == 0) {
	    ad_system_packet_campaign_name_check(r, &sql_conn, &json);

	    //推广->计划添加
	} else if (strcmp(packet_type, "ADD_CAMPAIGN") == 0) {
	    ad_system_packet_add_campaign(r, &sql_conn, &json);

	    //推广->计划获取
	} else if (strcmp(packet_type, "GET_CAMPAIGN") == 0) {
	    ad_system_packet_get_campaign(r, &sql_conn, &json);

	    //推广->计划修改
	} else if (strcmp(packet_type, "SET_CAMPAIGN") == 0) {
	    ad_system_packet_set_campaign(r, &sql_conn, &json);

	    //推广->计划删除
	} else if (strcmp(packet_type, "DEL_CAMPAIGN") == 0) {
	    ad_system_packet_del_campaign(r, &sql_conn, &json);

	    //推广->组->重名验证
	} else if (strcmp(packet_type, "GROUP_NAME_CHECK") == 0) {
	    ad_system_packet_group_name_check(r, &sql_conn, &json);

	    //推广->组->添加
	} else if (strcmp(packet_type, "ADD_GROUP") == 0) {
	    ad_system_packet_add_group(r, &sql_conn, &json);

	    //推广->组->获取
	} else if (strcmp(packet_type, "GET_GROUP") == 0) {
	    ad_system_packet_get_group(r, &sql_conn, &json);

	    //推广->组->修改
	} else if (strcmp(packet_type, "SET_GROUP") == 0) {
	    // ad_system_packet_set_group(r, &sql_conn, &json);

	    //推广->组->删除
	} else if (strcmp(packet_type, "DEL_GROUP") == 0) {
	    // ad_system_packet_del_group(r, &sql_conn, &json);

	    //推广->组->添加关键词
	} else if (strcmp(packet_type, "ADD_GROUP_KEYWORD") == 0) {
	    ad_system_packet_add_group_keyword(r, &sql_conn, &json);

	    //推广->组->获取关键词
	} else if (strcmp(packet_type, "GET_GROUP_KEYWORD") == 0) {
	    ad_system_packet_get_group_keyword(r, &sql_conn, &json);

	    //推广->组->删除关键词
	} else if (strcmp(packet_type, "DEL_GROUP_KEYWORD") == 0) {
	    // ad_system_packet_del_group_keyword(r, &sql_conn, &json);

	    //推广->文字创意->添加
	} else if (strcmp(packet_type, "ADD_CREATIVE") == 0) {
	    // ad_system_packet_add_creative_text(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_CREATIVE") == 0) {
	    ad_system_packet_get_creative_text(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_CREATIVE") == 0) {
	    // ad_system_packet_set_creative_text(r, &sql_conn, &json);

	    //图片创意
	} else if (strcmp(packet_type, "GET_IMG_CREATIVE") == 0) {
	    // ad_system_packet_get_img_creative(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_IMG_CREATIVE") == 0) {
	    // ad_system_packet_set_creative_img(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "ADD_IMG_CREATIVE") == 0) {
	    // ad_system_packet_add_creative_img(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "DEL_IMG_CREATIVE") == 0) {
	    // ad_system_packet_del_creative_img(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_UPLOAD_IMG_INFO") == 0) {
	    // ad_system_packet_set_upload_img_info (r, &sql_conn, &json);

	    //附加创意
	} else if (strcmp(packet_type, "GET_CREATIVE_APPEND") == 0) {
	    // ad_system_packet_get_creative_append(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_CREATIVE_APPEND") == 0) {
	    // ad_system_packet_set_creative_append(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "ADD_CREATIVE_APPEND") == 0) {
	    // ad_system_packet_add_creative_append(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "DEL_CREATIVE_APPEND") == 0) {
	    // ad_system_packet_del_creative_append(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_CREATIVE_APPEND_UNIQUE") == 0) {
	    // ad_system_packet_get_creative_append_unique(r, &sql_conn, &json);

	    //批量操作
	} else if (strcmp(packet_type, "BATCH_OPERATION") == 0) {
	    // ad_system_packet_batch_operation(r, &sql_conn, &json);

	    //批量复制创意
	} else if (strcmp(packet_type, "COPY_CREATIVE") == 0) {
	    // ad_system_packet_copy_creative(r, &sql_conn, &json);

	    /********************/
	    /**** 财务管理 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_MONEY_TREND") == 0) {
	    // ad_system_packet_get_money_trend(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "GET_MONEY_RECHARGE") == 0) {
	    // ad_system_packet_get_money_recharge(r, &sql_conn, &json);

	    /********************/
	    /**** 账户信息 ****/
	    /********************/

	} else if (strcmp(packet_type, "GET_USER_INFO") == 0) {
	    // ad_system_packet_get_user_info(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_USER_INFO") == 0) {
	    // ad_system_packet_set_user_info(r, &sql_conn, &json);

	} else if (strcmp(packet_type, "SET_USER_PASS") == 0) {
	    // ad_system_packet_set_user_pass(r, &sql_conn, &json);

	} else {

	    ad_system_packet_send_code(r, PACKET_CODE_FORMAT_ERR);
	}
    }

    lg_json_free(json_root);
    lg_ocilib_free(&sql_conn);
}




