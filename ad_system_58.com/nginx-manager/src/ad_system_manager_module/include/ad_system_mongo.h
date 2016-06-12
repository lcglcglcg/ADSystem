
#ifndef	__AD_SYSTEM_MONGO_H__
#define	__AD_SYSTEM_MONGO_H__

#include "bson.h"
#include "mongoc.h"
#include "lg_list.h"
#include "lg_config_file.h"

#define	MONGO_DB_NAME "adx"
#define MONGO_REPORT_LIST "report_list"
#define MONGO_DELIVERY_CENTER_LIST "delivery_center_list"

#ifdef __cplusplus
extern "C" { 
#endif

    extern int ad_system_mongo_init(lg_list_t *config_file_head);

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

    typedef struct {

	lg_list_t next;
	char account_id[128];
	char domain[128];
	int impression;
	int click;
	double cost;

    } ad_system_mongo_list_t;

    typedef struct {

	lg_list_t next;
	char domain_id[128];
	char domain_str[1024];

    } ad_system_mongo_domain_list_t;

    //媒体列表
    extern int ad_system_mongo_get_media_domain_list(lg_list_t *head);

    //查询媒体
    extern int ad_system_mongo_get_media_domain_unique(const char *domain_id, char *domain_str);

    //财务->媒体报告载入
    extern int ad_system_mongo_report_media_load(lg_list_t *head,
	    const char *visit_domain,
	    const char *time_s,
	    const char *time_e);

    //财务->用户报告载入
    extern int ad_system_mongo_report_account_load(lg_list_t *head,
	    const char *visit_domain,
	    const char *time_s,
	    const char *time_e,
	    int mode);

    extern int ad_system_report_xls_file_media(lg_list_t *head, char *path);
    extern int ad_system_report_xls_file_account(lg_list_t *head, char *path, int mode);

    //账户管理->去投放中心
    extern int ad_system_mongo_delivery_center_add(char *key, char *account_id);

    extern double ad_system_mongo_get_account_cost_total(ad_system_mongo_t *conn, const char *date_str, int account_id);

#ifdef __cplusplus
}
#endif



#endif




