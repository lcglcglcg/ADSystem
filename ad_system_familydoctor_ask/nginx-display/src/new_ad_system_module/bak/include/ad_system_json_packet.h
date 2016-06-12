
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

	const char *ACCOUNT_ID;
	int CREATIVE_TYPE;

	int DEPT_TYPE;
	const char *DEPT_SERIALIZE;
	const char *SHOW_TIME_START;
	const char *SHOW_TIME_END;
	const char *PRICE;
	const char *BRAND_ID;
	int MOD_STATUS;
	int INSERT_TYPE;

	const char *TITLE;
	const char *DESCRIPTION;
	const char *DESTINATION_URL;
	const char *DISPLAY_URL;
	const char *WEB_SITE;
	const char *WEB_IM;
	const char *GUAHAO_URL;

	const char *IMG0_ID;
	const char *IMG0_URL;
	const char *IMG1_ID;
	const char *IMG1_URL;
	const char *IMG2_ID;
	const char *IMG2_URL;
	const char *IMG3_ID;
	const char *IMG3_URL;
	const char *IMG4_ID;
	const char *IMG4_URL;
	const char *IMG5_ID;
	const char *IMG5_URL;
	
	const char *BRAND_CREATIVE_ID;
	int SHOW_TIME_TYPE;
    
    } ad_system_packet_json_data_t;

    extern const char *ad_system_json_packet_head_parse(json_object *json_root);
    extern const char *ad_system_json_packet_value_parse(json_object *json_root, const char *key);
    extern json_object *ad_system_json_packet_data_parse(json_object *json_root, ad_system_packet_json_data_t *json);

#ifdef __cplusplus

#endif

#endif



