
#ifndef	__AD_SYSTEM_JSON_PACKET_H__
#define	__AD_SYSTEM_JSON_PACKET_H__

#include "lg_string.h"
#include "lg_string_json.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	const char *PASS_OLD;
	const char *PASS_NEW;

	int FIND_TYPE;
	char FIND_NAME[256];

	int PAGE_COUNT;
	int PAGE_INDEX;
	int page_index_s;
	int page_index_e;

	int DATE_TYPE;
	int DISPLAY_STATUS;

	int SORT_COLUMN;
	int SORT_TYPE;

	int FIND_NAME_TYPE;
	int FIND_STATUS;
	int UPDATE_TYPE;
	int STATUS;

	int TIME_TYPE;
	const char *BEGIN_TIME;
	const char *END_TIME;

	const char *CAMPAIGN_ID;
	char CAMPAIGN_NAME[1024];
	const char *CAMPAIGN_BUDGET;
	int CAMPAIGN_STATUS;

	int REGION_TYPE;
	const char *REGION_SERIALIZE;
	int SCHEDULE_TYPE;
	const char *SCHEDULE_SERIALIZE;

	const char *GROUP_ID;
	int GROUP_NAME_TYPE;
	char GROUP_NAME[1024];
	int GROUP_PRICE_TYPE;
	const char *GROUP_CPC_PRICE;
	const char *GROUP_CPM_PRICE;
	int GROUP_TARGETING_TYPE;
	const char *GROUP_DISEASE;
	const char *GROUP_KEYWORD_ID;
	const char *GROUP_KEYWORD;

	const char *CREATIVE_ID;
	char CREATIVE_NAME[1024];
	int CREATIVE_STATUS;
	char CREATIVE_DESCRIPTION[2048];
	const char *MOBILE_DESTINATION_URL;
	const char *MOBILE_DISPLAY_URL;
	const char *CREATIVE_DESTINATION_URL;
	const char *CREATIVE_DISPLAY_URL;
	const char *CREATIVE_IMG_ID;

	const char *CREATIVE_APPEND_ID;
	int APPEND_STATUS;
	const char *APPEND_CONTENT;
	int CREATIVE_APPEND_TYPE;

	const char *ID;
	int ID_TYPE;
	int BATCH_TYPE;

	const char *REPORT_ID;
	int REPORT_SPECIES;
	int REPORT_TYPE;
	const char *REPORT_NAME;
	const char *TIME_START;
	const char *TIME_END;
	int REPORT_BELONG;

	int MONEY_TYPE;
	const char *COMPANY_NAME;
	const char *COMPANY_SITE;
	const char *INDUSTRY;
	const char *PROVINCE_ID;
	const char *CITY_ID;
	const char *ADDRESS;
	const char *POSTCODE;
	const char *CONTACT;
	const char *TELEPHONE;
	const char *FAX;
	const char *MOBILE;
	const char *EMAIL;
	const char *WEBIM;
	const char *KEY;

	int FIND_NAME_COL;
	int BALANCE_TYPE;

	int CREATIVE_TYPE;
	const char *CREATIVE_REFUSE_REASON;
	const char *CREATIVE_ID_ARRAY;

	const char *MEDIA_ID;
	int MEDIA_TYPE;

	const char *GUAHAO_TOKEN;
	const char *HOSPITAL_IMG;
	const char *LOGIN_PASSWORD;
	const char *REGION_NAME_CN;

	int INSERT_TYPE;
	const char *LOGIN_NAME;
	const char *API_TOKEN;
	int ACCOUNT_ID_TYPE;
	const char *ACCOUNT_ID;

	const char *ANNOUNCEMENT_ID;
	const char *ANNOUNCEMENT_TITLE;
	const char *SEND_TIME;
	const char *CONTENT;

	const char *MONEY;
	int PAYMENT_TYPE;
	const char *ACCOUNTING_DOCUMENT_NUMBER;
	const char *ACCOUNTING_DOCUMENT_FILE;
	const char *ACCOUNTING_DOCUMENT_REMARK;

	const char *SALES;
	const char *CUSTOMER_SERVICE;

	int PLACE_REGION;
	int DSP_STATUS;

	int DEPT_TYPE;
	const char *DEPT_SERIALIZE;

	const char *SHOW_TIME_START;
	const char *SHOW_TIME_END;
	const char *CREATE_TIME_START;
	const char *CREATE_TIME_END;

	int BRAND_ID_TYPE;
	const char *BRAND_ID;
	const char *PRICE;
	const char *REFUSE_REASON;

	int BRAND_TYPE;
	int PAGE_TYPE;

	int SHOW_TIME_TYPE;
	int CREATE_TIME_TYPE;

	int DISCOUNT;
	const char *CPC_PRICE;
	const char *BUDGET_MORE;
	const char *BUDGET_LESS;

	const char *ACCOUNT_NAME;
	const char *UPLOAD_ID;
	const char *IMG_ID;

	int JOIN_DSP;

    } ad_system_packet_json_data_t;

    extern const char *ad_system_json_packet_head_parse(json_object *json_root);
    extern const char *ad_system_json_packet_value_parse(json_object *json_root, const char *key);
    extern json_object *ad_system_json_packet_data_parse(json_object *json_root, ad_system_packet_json_data_t *json);

#ifdef __cplusplus
}
#endif

#endif



