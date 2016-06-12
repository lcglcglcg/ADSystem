
#ifndef	__AD_SYSTEM_REDIS_H__
#define	__AD_SYSTEM_REDIS_H__

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_thread.h>
#include <hiredis/hiredis.h>
#include "lg_config_file.h"
#include "lg_list.h"

#define REDIS_DB_VAR_CODE "1"
#define REDIS_DB_SESSION "2"
#define REDIS_DB_IMG_INFO "3"
#define	REDIS_DB_MANAGER_VAR_CODE "4"
#define	REDIS_DB_MANAGER_SESSION "5"
#define	REDIS_DB_MANAGER_REPORT "6"
#define	REDIS_DB_MANAGER_MONEY "7"
#define	REDIS_DB_DELIVERY_CENTER "8"

#define	REDIS_DB_MATERIALS_CREATIVE	"10"
#define	REDIS_DB_MATERIALS_GROUP	"11"
#define	REDIS_DB_MATERIALS_CAMPAIGN	"12"
#define	REDIS_DB_MATERIALS_ACCOUNT	"13"
#define	REDIS_DB_REPORT_MONEY 		"14"

extern int ad_system_redis_init(lg_list_t *config_file_head);
extern redisContext *ad_system_redis_connect(char *dbs);

extern int ad_system_redis_add_var_code(char *session_id, const char *value);
extern int ad_system_redis_get_var_code(const char *session_id, char *value);

extern int ad_system_redis_set_login_session(const char *session_id, const char *value);
extern int ad_system_redis_get_login_session(const char *session_id, char *value);
extern int ad_system_redis_del_login_session(const char *session_id);

extern int ad_system_redis_get_creative_info(
	int display_style, 
	char *creative,
	char *campaign_name, 
	char *group_name, 
	char *creative_name);

extern int ad_system_redis_add_report(char *report_key, char *report_path);
extern int ad_system_redis_get_report(const char *report_key, char *report_path);

extern int ad_system_redis_add_money_info(char *key,
	const char *account_id,
	const char *money,
	int payment_type,
	const char *accounting_document_number,
	const char *accounting_document_file,
	const char *accounting_document_remark,
	int BRAND_TYPE,
	int DISCOUNT);
extern int ad_system_redis_get_money_info(const char *key,
	char *account_id,
	char *money,
	char *payment_type,
	char *accounting_document_number,
	char *accounting_document_file,
	char *accounting_document_remark,
	char *BRAND_TYPE,
	char *DISCOUNT);

extern int ad_system_redis_add_delivery_center(const char *key, const char *account_id);
extern int ad_system_redis_get_delivery_center(const char *key, char *account_id);

extern int ad_system_redis_get_account_name(int account_id, char *account_name);
extern int ad_system_redis_get_campaign_name(const char *campaign_id, char *campaign_name);
extern int ad_system_redis_get_group_name(const char *group_id, char *group_name);
extern int ad_system_redis_get_creative_name(int display_style, const char *creative_id, char *creative_name);

typedef struct {

    lg_list_t next;

    char key[128];
    double money;
    double discount;
    double multiple;

} ad_system_redis_recharge_t;

extern int ad_system_redis_recharge_get_list(int account_id, lg_list_t *head);
extern int ad_system_redis_get_img(const char *img_id, char *img_name, char *img_size, char *img_buffer_size, char *img_path);

#endif


