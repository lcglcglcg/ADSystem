
#ifndef	__AD_SYSTEM_SQL_H__
#define	__AD_SYSTEM_SQL_H__

#include "lg_ocilib.h"

//登录验证
extern int ad_system_sql_login(lg_ocilib_t *p,  char *account,  char *password);

//获取标题信息
extern int ad_system_sql_get_title_info(lg_ocilib_t *p, char *account_id);

//推广->获取地域CODE
extern int ad_system_sql_get_region_code(lg_ocilib_t *p);

//推广->获取科室病种CODE
extern int ad_system_sql_get_disease_code(lg_ocilib_t *p);

//推广->左侧列表
extern int ad_system_sql_get_sperad_list(lg_ocilib_t *p, char *account_id, int display_status);

//推广->计划重命名验证
extern int ad_system_sql_campaign_name_check(lg_ocilib_t *p,  char *account_id,  char *campaign_name, int display_style);

//推广->计划创建ID
extern int ad_system_sql_create_campaign_id(lg_ocilib_t *sql_conn);

//推广->计划添加
extern int ad_system_sql_add_campaign(lg_ocilib_t *sql_conn,
	char *account_id,
	char *campaign_id,
	char *campaign_name,
	double campaign_dudget,
	int region_type,
	char *region_serialize,
	int schedule_type,
	char *schedule_serialize,
	char *begin_time,
	char *end_time,
	int display_status);

//推广->计划获取
extern int ad_system_sql_get_campaign(lg_ocilib_t *p,
	char *account_id, 
	int display_status,
	int find_name_type,
	char *find_name);

//推广->计划删除
extern int ad_system_sql_del_campaign(lg_ocilib_t *p, char *campaign_id);

//推广->组重名验证
extern int ad_system_sql_group_name_check(lg_ocilib_t *p, char *campaign_id,  int display_status,  char *group_name);

//推广->组->创建新的ID
extern int ad_system_sql_create_group_id(lg_ocilib_t *sql_conn);

//推广->组->添加
extern int ad_system_sql_add_group(lg_ocilib_t *p,
	char *account_id,
	int display_status,
	char *campaign_id,
	char *group_id,
	char *group_name,
	int group_price_type,
	char *group_cpc_price,
	char *group_cpm_price,
	int targeting_type,
	char *group_keyword,
	char *group_disease);

//推广->组->获取
extern int ad_system_sql_get_group(lg_ocilib_t *p,
	char *account_id, 
	int display_status, 
	int find_type,
	char *campaign_id, 
	int group_name_type, 
	char *group_name);

//推广->组->关键词
extern int ad_system_sql_get_group_keyword(lg_ocilib_t *p,  char *account_id,
	int page_index_s,
	int page_index_e,
	int display_status,
	char *campaign_id,
	char *group_id,
	int find_type, 
	int find_name_type,
	char *find_name);

#endif




