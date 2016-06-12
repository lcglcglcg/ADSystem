
#ifndef	__AD_SYSTEM_REDIS_H__
#define	__AD_SYSTEM_REDIS_H__

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_thread.h>
#include <hiredis/hiredis.h>
#include "lg_config_file.h"

#define REDIS_DB_VAR_CODE "1"
#define REDIS_DB_SESSION "2"
#define REDIS_DB_IMG_INFO "3"
#define	REDIS_DB_MANAGER_VAR_CODE "4"
#define	REDIS_DB_MANAGER_SESSION "5"
#define	REDIS_DB_MANAGER_REPORT "6"
#define	REDIS_DB_MANAGER_MONEY "7"
#define	REDIS_DB_DELIVERY_CENTER "8"
#define	REDIS_DB_MANAGER_CENTER "9"

#define	REDIS_DB_MATERIALS_CREATIVE	"10"
#define	REDIS_DB_MATERIALS_GROUP	"11"
#define	REDIS_DB_MATERIALS_CAMPAIGN	"12"
#define	REDIS_DB_MATERIALS_ACCOUNT	"13"

#ifdef __cplusplus
extern "C" { 
#endif

    extern int ad_system_redis_init(lg_list_t *config_file_head);
    extern redisContext *ad_system_redis_connect(char *dbs);

    extern int ad_system_redis_add_var_code(char *session_id, const char *value);
    extern int ad_system_redis_get_var_code(const char *session_id, char *value);
    extern int ad_system_redis_set_login_session(const char *session_id, const char *value);
    extern int ad_system_redis_get_login_session(const char *session_id, char *value);
    extern int ad_system_redis_del_login_session(const char *session_id);
    extern int ad_system_redis_get_delivery_center(const char *key, char *account_id);

    extern int ad_system_redis_get_account_name(const char *account_id, char *account_name);
    extern int ad_system_redis_get_campaign_name(int campaign_id, char *campaign_name);
    extern int ad_system_redis_get_group_name(int group_id, char *group_name);
    extern int ad_system_redis_get_creative_name(int display_style, int creative_id, char *creative_name);

    extern int ad_system_redis_get_img(const char *img_id, char *img_name, char *img_size, char *img_buffer_size, char *img_path);

    extern int ad_system_redis_get_manager_center(const char *key);

#ifdef __cplusplus
}
#endif

#endif

