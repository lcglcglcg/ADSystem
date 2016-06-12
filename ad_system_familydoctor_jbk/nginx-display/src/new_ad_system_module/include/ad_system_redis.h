
#ifndef	__AD_SYSTEM_REDIS_H__
#define	__AD_SYSTEM_REDIS_H__

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_thread.h>
#include <hiredis/hiredis.h>

#define REDIS_DB_SESSION 		"0"
#define REDIS_DB_VAR_CODE 		"1"
#define REDIS_DB_IMG_INFO 		"2"
#define	REDIS_DB_MONEY			"3"
#define	REDIS_DB_DELIVERY_CENTER 	"4"


// var code
extern int ad_system_redis_add_var_code(char *session_id, char *value);
extern int ad_system_redis_get_var_code(char *session_id, char *value);

// session
extern int ad_system_redis_set_login_session(char *session_id, char *value);
extern int ad_system_redis_get_login_session(char *session_id, char *value);
extern int ad_system_redis_del_login_session(char *session_id);



extern int ad_system_redis_get_delivery_center(char *key, char *account_id);
extern int ad_system_redis_get_account_name(char *account_id, char *account_name);
extern int ad_system_redis_get_campaign_name(int campaign_id, char *campaign_name);
extern int ad_system_redis_get_group_name(int group_id, char *group_name);
extern int ad_system_redis_get_creative_name(int display_style, int creative_id, char *creative_name);

extern int ad_system_redis_get_img(char *img_id, char *img_name, char *img_size, char *img_buffer_size, char *img_path);

extern int ad_system_redis_get_manager_center(char *key);

#ifdef __cplusplus
}
#endif

#endif

