
#ifndef	__AD_SYSTEM_MONGO_H__
#define	__AD_SYSTEM_MONGO_H__

#include "bson.h"
#include "mongoc.h"
#include "lg_list.h"
#include "lg_config_file.h"

#define	MONGO_DB_NAME "adx"
#define MONGO_REPORT_LIST "report_list"
#define MONGO_REPORT_STANDARD "report_standard"
#define MONGO_REPORT_REGION "report_region"
#define MONGO_DELIVERY_CENTER_LIST "delivery_center_list"

#define	ACCOUNT_DAY_TEMP "day_account_temp"
#define CAMPAIGN_DAY_TEMP "day_campaign_temp"
#define GROUP_DAY_TEMP "day_group_temp"
#define CREATIVE_DAY_TEMP "day_creative_temp"

#define	ACCOUNT_YESTERDAY_TEMP "yesterday_account"
#define CAMPAIGN_YESTERDAY_TEMP "yesterday_campaign"
#define GROUP_YESTERDAY_TEMP "yesterday_group"
#define CREATIVE_YESTERDAY_TEMP "yesterday_creative"

#define	ACCOUNT_SEVENDAY_TEMP "sevenday_account"
#define CAMPAIGN_SEVENDAY_TEMP "sevenday_campaign"
#define GROUP_SEVENDAY_TEMP "sevenday_group"
#define CREATIVE_SEVENDAY_TEMP "sevenday_creative"

#define	ACCOUNT_SEVENDAY_DAILY_TEMP "sevenday_daily"

#ifdef __cplusplus
extern "C" { 
#endif

    /* api */
    typedef struct {

	mongoc_client_t *client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_error_t error;

    } ad_system_mongo_t;

    extern int ad_system_mongo_init(lg_list_t *config_file_head);
    extern int ad_system_mongo_conn(ad_system_mongo_t *p);
    extern void ad_system_mongo_free(ad_system_mongo_t *p);
    extern void ad_system_mongo_list_free(lg_list_t *head);

    //标准报告载入
    extern int ad_system_mongo_report_standard_load(lg_list_t *head,
	    int display_style, int bidding_model, const char *account_id, const char *time_s, const char *time_e,int time_type);

    //地域报告载入
    extern int ad_system_mongo_report_region_load(lg_list_t *head, 
	    int display_style, int bidding_model, const char *account_id, const char *time_s, const char *time_e,int time_type);

    //后台管理登录
    extern int ad_system_mongo_delivery_center_find(const char *key, char *account_id_str);

    //统计页面
    typedef struct {

	lg_list_t next;
	char name[128];
	int impression;
	int click;
	double cost;

    } ad_system_mongo_stat_t;

    extern int ad_system_mongo_stat_load(ad_system_mongo_stat_t *p, const char *table, int display_style, const char *account_id);
    extern int ad_system_mongo_stat_list_load(lg_list_t *head, const char *table, int display_style,
	    const char *account_id, const char *name);
    extern ad_system_mongo_stat_t *ad_system_mongo_stat_list_find(lg_list_t *head, const char *name);

#ifdef __cplusplus
}
#endif

#endif




