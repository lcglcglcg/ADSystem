
#ifndef	__AD_SYSTEM_JSON_H__
#define	__AD_SYSTEM_JSON_H__

#include "lg_json.h"

typedef struct {

    // type
    char *PACKET_TYPE;

    // LOGIN
    char *ACCOUNT;
    char *PASSWORD;
    char *VAR_CODE;

    // GET_TITLE_INFO
    char *USER_LOGIN_NAME;
    char *COMPANY_NAME;
    char *BALANCE;
    char *INVEST;
    char *CONSUMPTION;
    char *DOMAIN;
    char *GUAHAO_TOKEN;
    char *PLACE_REGION;









    char *PASS_OLD;
    char *PASS_NEW;

    int FIND_TYPE;
    char *FIND_NAME;

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
    char *BEGIN_TIME;
    char *END_TIME;

    char *CAMPAIGN_ID;
    char *CAMPAIGN_NAME;
    double CAMPAIGN_BUDGET;
    int CAMPAIGN_STATUS;

    int REGION_TYPE;
    char *REGION_SERIALIZE;
    int SCHEDULE_TYPE;
    char *SCHEDULE_SERIALIZE;

    char *GROUP_ID;
    int GROUP_NAME_TYPE;
    char *GROUP_NAME;
    int GROUP_PRICE_TYPE;
    char *GROUP_CPC_PRICE;
    char *GROUP_CPM_PRICE;
    int GROUP_TARGETING_TYPE;
    char *GROUP_DISEASE;
    char *GROUP_KEYWORD_ID;
    char *GROUP_KEYWORD;

    char *CREATIVE_ID;
    char *CREATIVE_NAME;
    int CREATIVE_STATUS;
    char *CREATIVE_DESCRIPTION;
    char *MOBILE_DESTINATION_URL;
    char *MOBILE_DISPLAY_URL;
    char *CREATIVE_DESTINATION_URL;
    char *CREATIVE_DISPLAY_URL;
    char *CREATIVE_IMG_ID;

    char *CREATIVE_APPEND_ID;
    int APPEND_STATUS;
    char *APPEND_CONTENT;
    int CREATIVE_APPEND_TYPE;

    char *ID;
    int ID_TYPE;
    int BATCH_TYPE;

    char *REPORT_ID;
    int REPORT_SPECIES;
    int REPORT_TYPE;
    char *REPORT_NAME;
    char *TIME_START;
    char *TIME_END;
    int REPORT_BELONG;

    int MONEY_TYPE;
    char *COMPANY_SITE;
    char *INDUSTRY;
    char *PROVINCE_ID;
    char *CITY_ID;
    char *ADDRESS;
    char *POSTCODE;
    char *CONTACT;
    char *TELEPHONE;
    char *FAX;
    char *MOBILE;
    char *EMAIL;
    char *WEBIM;
    char *KEY;

    char *ACCOUNT_ID;
    int CREATIVE_TYPE;

    int DEPT_TYPE;
    char *DEPT_SERIALIZE;
    char *SHOW_TIME_START;
    char *SHOW_TIME_END;
    char *PRICE;
    char *BRAND_ID;
    int MOD_STATUS;
    int INSERT_TYPE;

    char *TITLE;
    char *DESCRIPTION;
    char *DESTINATION_URL;
    char *DISPLAY_URL;
    char *WEB_SITE;
    char *WEB_IM;
    char *GUAHAO_URL;

    char *IMG0_ID;
    char *IMG0_URL;
    char *IMG1_ID;
    char *IMG1_URL;
    char *IMG2_ID;
    char *IMG2_URL;
    char *IMG3_ID;
    char *IMG3_URL;
    char *IMG4_ID;
    char *IMG4_URL;
    char *IMG5_ID;
    char *IMG5_URL;

    char *BRAND_CREATIVE_ID;
    int SHOW_TIME_TYPE;

} ad_system_json_t;

extern lg_json_t *ad_system_json_parse(ad_system_json_t *p, char *buffer);

#endif


