
#include <string.h>
#include "ad_system_json.h"

#define	GET_JSON_VALUE_NUMBER(o,s) p->o=lg_json_value_to_number(lg_json_get_object(root, s));
#define	GET_JSON_VALUE_DOUBLE(o,s) p->o=lg_json_value_to_double(lg_json_get_object(root, s));
#define GET_JSON_VALUE_STRING(o,s) p->o=lg_json_value_to_string(lg_json_get_object(root, s));

lg_json_t *ad_system_json_parse(ad_system_json_t *p, char *buffer)
{

    lg_json_t *root = lg_json_parse(buffer);
    if (!root) return NULL;

    memset(p, 0, sizeof(ad_system_json_t));

    GET_JSON_VALUE_STRING(PACKET_TYPE,		"PACKET_TYPE");
    GET_JSON_VALUE_STRING(ACCOUNT,                	"ACCOUNT");
    GET_JSON_VALUE_STRING(PASSWORD,               	"PASSWORD");
    GET_JSON_VALUE_STRING(VAR_CODE,               	"VAR_CODE");
    GET_JSON_VALUE_STRING(USER_LOGIN_NAME,        	"USER_LOGIN_NAME");
    GET_JSON_VALUE_STRING(COMPANY_NAME,           	"COMPANY_NAME");
    GET_JSON_VALUE_STRING(BALANCE,                	"BALANCE");
    GET_JSON_VALUE_STRING(INVEST,                 	"INVEST");
    GET_JSON_VALUE_STRING(CONSUMPTION,            	"CONSUMPTION");
    GET_JSON_VALUE_STRING(DOMAIN,                 	"DOMAIN");
    GET_JSON_VALUE_STRING(GUAHAO_TOKEN,           	"GUAHAO_TOKEN");
    GET_JSON_VALUE_STRING(PLACE_REGION,           	"PLACE_REGION");
















    GET_JSON_VALUE_STRING(PASS_OLD,		"PASS_OLD");
    GET_JSON_VALUE_STRING(PASS_NEW,		"PASS_NEW");

    GET_JSON_VALUE_NUMBER(FIND_TYPE,		"FIND_TYPE");
    GET_JSON_VALUE_STRING(FIND_NAME,		"FIND_NAME");
    GET_JSON_VALUE_NUMBER(PAGE_COUNT,		"PAGE_COUNT");
    GET_JSON_VALUE_NUMBER(PAGE_INDEX,		"PAGE_INDEX");

    if (p->PAGE_COUNT && p->PAGE_INDEX) {
	p->page_index_s = p->PAGE_COUNT * p->PAGE_INDEX - p->PAGE_COUNT + 1;
	p->page_index_e = p->page_index_s + p->PAGE_COUNT - 1;
    }

    GET_JSON_VALUE_NUMBER(DATE_TYPE,		"DATE_TYPE");
    GET_JSON_VALUE_NUMBER(DISPLAY_STATUS,		"DISPLAY_STATUS");

    GET_JSON_VALUE_NUMBER(SORT_COLUMN,		"SORT_COLUMN");
    GET_JSON_VALUE_NUMBER(SORT_TYPE,		"SORT_TYPE");

    GET_JSON_VALUE_NUMBER(FIND_NAME_TYPE,		"FIND_NAME_TYPE");
    GET_JSON_VALUE_NUMBER(FIND_STATUS,		"FIND_STATUS");
    GET_JSON_VALUE_NUMBER(UPDATE_TYPE,		"UPDATE_TYPE");
    GET_JSON_VALUE_NUMBER(STATUS,			"STATUS");

    GET_JSON_VALUE_NUMBER(TIME_TYPE,		"TIME_TYPE");
    GET_JSON_VALUE_STRING(BEGIN_TIME,		"BEGIN_TIME");
    GET_JSON_VALUE_STRING(END_TIME,		"END_TIME");

    GET_JSON_VALUE_STRING(CAMPAIGN_ID,		"CAMPAIGN_ID");
    GET_JSON_VALUE_STRING(CAMPAIGN_NAME,		"CAMPAIGN_NAME");
    GET_JSON_VALUE_DOUBLE(CAMPAIGN_BUDGET,		"CAMPAIGN_BUDGET");
    GET_JSON_VALUE_NUMBER(CAMPAIGN_STATUS,		"CAMPAIGN_STATUS");

    GET_JSON_VALUE_NUMBER(REGION_TYPE,		"REGION_TYPE");
    GET_JSON_VALUE_STRING(REGION_SERIALIZE,	"REGION_SERIALIZE");
    GET_JSON_VALUE_NUMBER(SCHEDULE_TYPE,		"SCHEDULE_TYPE");
    GET_JSON_VALUE_STRING(SCHEDULE_SERIALIZE,	"SCHEDULE_SERIALIZE");

    GET_JSON_VALUE_STRING(GROUP_ID,		"GROUP_ID");
    GET_JSON_VALUE_NUMBER(GROUP_NAME_TYPE,		"GROUP_NAME_TYPE");
    GET_JSON_VALUE_STRING(GROUP_NAME,		"GROUP_NAME");
    GET_JSON_VALUE_NUMBER(GROUP_PRICE_TYPE,	"GROUP_PRICE_TYPE");
    GET_JSON_VALUE_STRING(GROUP_CPC_PRICE,		"GROUP_CPC_PRICE");
    GET_JSON_VALUE_STRING(GROUP_CPM_PRICE,		"GROUP_CPM_PRICE");
    GET_JSON_VALUE_NUMBER(GROUP_TARGETING_TYPE,	"GROUP_TARGETING_TYPE");
    GET_JSON_VALUE_STRING(GROUP_DISEASE,		"GROUP_DISEASE");
    GET_JSON_VALUE_STRING(GROUP_KEYWORD_ID,	"GROUP_KEYWORD_ID");
    GET_JSON_VALUE_STRING(GROUP_KEYWORD,		"GROUP_KEYWORD");

    GET_JSON_VALUE_STRING(CREATIVE_ID,		"CREATIVE_ID");
    GET_JSON_VALUE_STRING(CREATIVE_NAME,		"CREATIVE_NAME");
    GET_JSON_VALUE_NUMBER(CREATIVE_STATUS,		"CREATIVE_STATUS");
    GET_JSON_VALUE_STRING(CREATIVE_DESCRIPTION,	"CREATIVE_DESCRIPTION");
    GET_JSON_VALUE_STRING(MOBILE_DESTINATION_URL,	"MOBILE_DESTINATION_URL");
    GET_JSON_VALUE_STRING(MOBILE_DISPLAY_URL,	"MOBILE_DISPLAY_URL");
    GET_JSON_VALUE_STRING(CREATIVE_DESTINATION_URL,"CREATIVE_DESTINATION_URL");
    GET_JSON_VALUE_STRING(CREATIVE_DISPLAY_URL,	"CREATIVE_DISPLAY_URL");
    GET_JSON_VALUE_STRING(CREATIVE_IMG_ID,		"CREATIVE_IMG_ID");


    GET_JSON_VALUE_STRING(CREATIVE_APPEND_ID,	"CREATIVE_APPEND_ID");
    GET_JSON_VALUE_NUMBER(APPEND_STATUS,		"APPEND_STATUS");
    GET_JSON_VALUE_STRING(APPEND_CONTENT,		"APPEND_CONTENT");
    GET_JSON_VALUE_NUMBER(CREATIVE_APPEND_TYPE,	"CREATIVE_APPEND_TYPE");

    GET_JSON_VALUE_STRING(ID,			"ID");
    GET_JSON_VALUE_NUMBER(ID_TYPE,			"ID_TYPE");
    GET_JSON_VALUE_NUMBER(BATCH_TYPE,		"BATCH_TYPE");

    GET_JSON_VALUE_STRING(REPORT_ID,		"REPORT_ID");
    GET_JSON_VALUE_NUMBER(REPORT_SPECIES,		"REPORT_SPECIES");
    GET_JSON_VALUE_NUMBER(REPORT_TYPE,		"REPORT_TYPE");
    GET_JSON_VALUE_STRING(REPORT_NAME,		"REPORT_NAME");
    GET_JSON_VALUE_STRING(TIME_START,		"TIME_START");
    GET_JSON_VALUE_STRING(TIME_END,		"TIME_END");
    GET_JSON_VALUE_NUMBER(REPORT_BELONG,		"REPORT_BELONG");

    GET_JSON_VALUE_NUMBER(MONEY_TYPE,		"MONEY_TYPE");
    GET_JSON_VALUE_STRING(COMPANY_NAME,		"COMPANY_NAME");
    GET_JSON_VALUE_STRING(COMPANY_SITE,		"COMPANY_SITE");
    GET_JSON_VALUE_STRING(INDUSTRY ,		"INDUSTRY");
    GET_JSON_VALUE_STRING(PROVINCE_ID,		"PROVINCE_ID");
    GET_JSON_VALUE_STRING(CITY_ID,			"CITY_ID");
    GET_JSON_VALUE_STRING(ADDRESS,			"ADDRESS");
    GET_JSON_VALUE_STRING(POSTCODE,		"POSTCODE");
    GET_JSON_VALUE_STRING(CONTACT,			"CONTACT");
    GET_JSON_VALUE_STRING(TELEPHONE,		"TELEPHONE");
    GET_JSON_VALUE_STRING(FAX,			"FAX");
    GET_JSON_VALUE_STRING(MOBILE,			"MOBILE");
    GET_JSON_VALUE_STRING(EMAIL,			"EMAIL");
    GET_JSON_VALUE_STRING(WEBIM,			"WEBIM");
    GET_JSON_VALUE_STRING(KEY,			"KEY");

    GET_JSON_VALUE_STRING(ACCOUNT_ID,		"ACCOUNT_ID");
    GET_JSON_VALUE_NUMBER(CREATIVE_TYPE,		"CREATIVE_TYPE");

    GET_JSON_VALUE_NUMBER(DEPT_TYPE,		"DEPT_TYPE");
    GET_JSON_VALUE_STRING(DEPT_SERIALIZE,		"DEPT_SERIALIZE");

    GET_JSON_VALUE_STRING(SHOW_TIME_START,		"SHOW_TIME_START");
    GET_JSON_VALUE_STRING(SHOW_TIME_END,		"SHOW_TIME_END");
    GET_JSON_VALUE_STRING(PRICE,			"PRICE");
    GET_JSON_VALUE_STRING(BRAND_ID,		"BRAND_ID");

    GET_JSON_VALUE_NUMBER(INSERT_TYPE,		"INSERT_TYPE");
    GET_JSON_VALUE_NUMBER(MOD_STATUS,		"MOD_STATUS");


    GET_JSON_VALUE_STRING(TITLE,			"TITLE");
    GET_JSON_VALUE_STRING(DESCRIPTION,		"DESCRIPTION");
    GET_JSON_VALUE_STRING(DESTINATION_URL,		"DESTINATION_URL");
    GET_JSON_VALUE_STRING(DISPLAY_URL,		"DISPLAY_URL");
    GET_JSON_VALUE_STRING(WEB_SITE,		"WEB_SITE");
    GET_JSON_VALUE_STRING(WEB_IM,			"WEB_IM");
    GET_JSON_VALUE_STRING(GUAHAO_URL,		"GUAHAO_URL");

    GET_JSON_VALUE_STRING(IMG0_ID,	   		"IMG0_ID");
    GET_JSON_VALUE_STRING(IMG0_URL,   		"IMG0_URL");
    GET_JSON_VALUE_STRING(IMG1_ID,    		"IMG1_ID");
    GET_JSON_VALUE_STRING(IMG1_URL,   		"IMG1_URL");
    GET_JSON_VALUE_STRING(IMG2_ID,    		"IMG2_ID");
    GET_JSON_VALUE_STRING(IMG2_URL,   		"IMG2_URL");
    GET_JSON_VALUE_STRING(IMG3_ID,    		"IMG3_ID");
    GET_JSON_VALUE_STRING(IMG3_URL,   		"IMG3_URL");
    GET_JSON_VALUE_STRING(IMG4_ID,    		"IMG4_ID");
    GET_JSON_VALUE_STRING(IMG4_URL,   		"IMG4_URL");
    GET_JSON_VALUE_STRING(IMG5_ID,    		"IMG5_ID");
    GET_JSON_VALUE_STRING(IMG5_URL,   		"IMG5_URL");

    GET_JSON_VALUE_STRING(BRAND_CREATIVE_ID,   	"BRAND_CREATIVE_ID");
    GET_JSON_VALUE_NUMBER(SHOW_TIME_TYPE,		"SHOW_TIME_TYPE");

    return root;
}


