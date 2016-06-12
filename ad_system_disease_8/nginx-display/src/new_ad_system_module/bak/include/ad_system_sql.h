
#ifndef	__AD_SYSTEM_SQL_H__
#define	__AD_SYSTEM_SQL_H__

#include "lg_list.h"
#include "lg_config_file.h"
#include "ad_system_packet.h"

#ifdef __cplusplus
extern "C" { 
#endif

    /**************/
    /* oracle API */
    /**************/

    typedef struct {

	void *cn;
	void *st;
	void *rs;

    } ad_system_sql_t;

    extern int ad_system_sql_init(lg_list_t *config_file_head);
    extern int ad_system_sql_connect(ad_system_sql_t *p);

    extern int ad_system_sql_fetch_next(ad_system_sql_t *p);
    extern int ad_system_sql_get_number(ad_system_sql_t *p, char *name, int index);
    extern double ad_system_sql_get_double(ad_system_sql_t *p, char *name, int index);
    extern const char *ad_system_sql_get_string(ad_system_sql_t *p, char *name, int index);

    extern int ad_system_sql_execute(ad_system_sql_t *p, char *sql_str);
    extern void ad_system_sql_commit(ad_system_sql_t *p);
    extern void ad_system_sql_rollback(ad_system_sql_t *p);
    extern void ad_system_sql_free(ad_system_sql_t *p);

    /************/
    /* 业务逻辑 */
    /************/

    //登录验证
    extern int ad_system_sql_login(ad_system_sql_t *p, const char *account, const char *password);

    //更新登录地址
    extern int ad_system_sql_set_login(const char *account_id, char *addr);

    //获取标题信息
    extern int ad_system_sql_get_title_info(ad_system_sql_t *p, char *account_id);

    //首页->获取公告
    extern int ad_system_sql_get_announcement(ad_system_sql_t *p, int page_index_s, int page_index_e, const char *title);

    //首页->获取公告->数量
    extern int ad_system_sql_get_announcement_count(ad_system_sql_t *p, const char *title);

    //首页->曲线图->出价
    extern int ad_system_sql_get_budget(ad_system_sql_t *p, char *account_id);

    //推广->获取科室病种CODE
    extern int ad_system_sql_get_disease_code(ad_system_sql_t *p);

    //推广->获取计划
    extern int ad_system_sql_get_campaign(ad_system_sql_t *p,
	    char *account_id, 
	    int display_status,
	    int find_type,
	    int find_name_type,
	    const char *find_name);

    //推广->获取组出价
    extern int ad_system_sql_get_group_price(const char *campaign_id, char *cpc, char *cpm);

    //推广->设置计划
    extern int ad_system_sql_set_campaign(char *account_id,
	    const char *campaign_id,
	    int update_type,
	    int status,
	    const char *dudget,
	    int region_type,
	    const char *region_serialize,
	    int schedule_type,
	    const char *schedule_serialize,
	    int time_type,
	    const char *begin_time,
	    const char *end_time,
	    const char *campign_name);

    //推广->获取组
    int ad_system_sql_get_group(ad_system_sql_t *p,
	    const char *account_id, 
	    int display_status, 
	    int find_type,
	    const char *campaign_id, 
	    int group_name_type, 
	    const char *group_name);

    //推广->设置组
    extern int ad_system_sql_set_group(
	    const char *account_id,
	    const char *group_id,
	    int update_type,
	    int status,
	    const char *cpc_price,
	    const char *cpm_price,
	    int bidding_model,
	    int targeting_type,
	    const char *group_name);

    //推广->设置组附加属性
    extern int ad_system_sql_set_group_targeting(
	    const char *account_id,
	    const char *group_id,
	    int update_type,
	    int targeting_type,
	    const char *disease,
	    const char *keyword);


    //推广->获取文字创意
    extern int ad_system_sql_get_creative_text(ad_system_sql_t *p,
	    const char *account_id, 
	    int find_type,
	    const char *campaign_id, 
	    const char *group_id, 
	    int creative_name_type, 
	    const char *creative_name);

    //推广->获取文字创意已删除
    extern int ad_system_sql_get_creative_del_text(ad_system_sql_t *p,
	    const char *account_id, 
	    int find_type,
	    const char *campaign_id, 
	    const char *group_id, 
	    int find_name_type, 
	    const char *find_name);

    //推广->修改文字创意
    extern int ad_system_sql_set_creative_text(const char *creative_id, int status);
#if 0
    extern int ad_system_sql_set_creative_text(
	    const char *account_id,
	    const char *creative_id,
	    int update_type,
	    int status,
	    const char *creative_name,
	    const char *descriptionpriec,
	    const char *destination_url,
	    const char *creative_display_url,
	    int creative_append_type,
	    const char *creative_append_id,
	    const char *mobile_destination_url,
	    const char *mobile_display_url);
#endif

    //推广->复制文字创意
    extern int ad_system_sql_copy_creative_text(
	    int update_update,
	    const char *creative_id,
	    const char *creative_name,
	    const char *descriptionpriec,
	    const char *destination_url,
	    const char *creative_display_url,
	    int creative_append_type,
	    const char *creative_append_id,
	    const char *mobile_destination_url,
	    const char *mobile_display_url);

    //推广->获取图片创意
    extern int ad_system_sql_get_img_creative(ad_system_sql_t *p,
	    const char *account_id, 
	    int find_type,
	    int find_status,
	    const char *campaign_id, 
	    const char *group_id, 
	    int creative_name_type, 
	    const char *creative_name);

    //推广->获取图片创意(已删除)
    extern int ad_system_sql_get_creative_del_img(ad_system_sql_t *p,
	    const char *account_id,
	    int find_type,
	    const char *campaign_id, 
	    const char *group_id, 
	    int find_name_type,
	    const char *find_name);

    //推广->修改图片创意
    extern int ad_system_sql_set_creative_img(
	    const char *account_id,
	    const char *creative_id,
	    int creative_status);

    //推广->复制图片创意
    extern int ad_system_sql_copy_creative_img(
	    int update_type,
	    const char *creative_id,
	    const char *creative_name,
	    const char *creative_img_size,
	    const char *creative_img_buffer_size,
	    const char *creative_img_path,
	    const char *creative_destination_url,
	    const char *creative_display_url);

    //推广->创建图片创意
    extern int ad_system_sql_add_creative_img(
	    const char *account_id,
	    const char *campaign_id,
	    const char *group_id,
	    const char *creative_name,
	    const char *img_size,
	    const char *img_buffer_size,
	    const char *img_path,
	    const char *creative_destination_url,
	    const char *creative_display_url);

    //推广->附加创意
    extern int ad_system_sql_get_creative_append(ad_system_sql_t *p,
	    const char *account_id, 
	    int find_type,
	    int sort_type,
	    int sort_column,
	    const char *find_name, 
	    int status);

    //推广->修改附加创意
    extern int ad_system_sql_set_creative_append(
	    const char *account_id,
	    const char *creative_append_id,
	    int update_type,
	    int status,
	    const char *contend);

    //推广->添加文字创意
    extern int ad_system_sql_add_creative_text(const char *account_id,
	    const char *campaign_id,
	    const char *group_id,
	    const char *creative_name,
	    const char *descriptionpriec,
	    const char *destination_url,
	    const char *creative_display_url,
	    int creative_append_type,
	    const char *creative_append_id,
	    const char *mobile_destination_url,
	    const char *mobile_display_url);

    //推广->添加附加创意
    extern int ad_system_sql_add_creative_append(const char *account_id, const char *append_contend);

    //推广->验证组重命名
    extern int ad_system_sql_group_name_check(const char *campaign_id, const int display_status, const char *group_name);

    //推广->获取新的组ID
    extern int ad_system_sql_get_group_id(char *group_id);

    //推广->添加组
    extern int ad_system_sql_add_group(
	    const char *account_id,
	    int display_status,
	    const char *campaign_id,
	    const char *group_id,
	    const char *group_name,
	    int group_price_type,
	    const char *group_cpc_price,
	    const char *group_cpm_price,
	    int targeting_type);

    //推广->添加组附属属性
    extern int ad_system_sql_add_group_targeting(
	    const char *account_id,
	    const char *campaign_id,
	    const char *group_id,
	    int targeting_type,
	    const char *disease,
	    const char *keyword);

    //推广->获取新的计划ID
    extern int ad_system_sql_get_campaign_id(char *campaign_id);

    //推广->验证计划重命名
    extern int ad_system_sql_campaign_name_check(const char *account_id, const char *campaign_name, int display_style);

    //推广->添加计划
    extern int ad_system_sql_add_campaign(
	    const char *account_id,
	    int display_status,
	    const char *campaign_id,
	    const char *campaign_name,
	    const char *dudget,
	    int region_type,
	    const char *region_serialize,
	    int schedule_type,
	    const char *schedule_serialize,
	    int time_type,
	    const char *begin_time,
	    const char *end_time);

    //删除文字创意
    extern int ad_system_sql_del_creative_text(const char *creative_id);

    //删除图片创意
    extern int ad_system_sql_del_creative_img(const char *creative_id);

    //删除组
    extern int ad_system_sql_del_group(const char *group_id);

    //删除计划
    extern int ad_system_sql_del_campaign(const char *campaign_id);

    //删除附加创意
    extern int ad_system_sql_del_creative_append(const char *append_id);

    //财务->充值记录
    extern int ad_system_sql_get_money_recharge(ad_system_sql_t *p,
	    const char *account_id,
	    int find_type,
	    int money_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    //财务->充值记录数量
    extern int ad_system_sql_get_money_recharge_count(ad_system_sql_t *p,
	    const char *account_id,
	    int find_type,
	    int money_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end);

    //账户信息->修改信息
    extern int ad_system_sql_set_user_info(ad_system_sql_t *p, char *account_id,
	    const char *company_name,
	    const char *company_site,
	    const char *industry,
	    const char *province_id,
	    const char *city_id,
	    const char *address,
	    const char *postcode,
	    const char *contact,
	    const char *telephone,
	    const char *fax,
	    const char *mobile,
	    const char *email,
	    const char *webim);

    //推广->获取预算与出价
    extern int ad_system_sql_get_budget_and_bidding(ad_system_sql_t *p, char *account_id, const char *group_id);

    //推广->计划(单个)
    extern int ad_system_sql_get_campaign_unique(ad_system_sql_t *p, const char *campaign_id);

    //推广->附加创意获取(单个)
    extern int ad_system_sql_get_creative_append_unique(ad_system_sql_t *p, const char *creative_append_id);

    //推广->获取组(单个)
    extern int ad_system_sql_get_group_unique(ad_system_sql_t *p,	const char *group_id);


    //账户信息->基本信息
    extern int ad_system_sql_get_user_info(ad_system_sql_t *p, char *account_id);

    //账户信息->获取密码
    extern int ad_system_sql_get_user_pass(ad_system_sql_t *p, char *account_id);

    //账户信息修改改改密码
    extern int ad_system_sql_set_user_pass(char *account_id, const char *pass_new);

    extern int ad_system_sql_get_group_count(ad_system_sql_t *p,
	    const char *account_id, 
	    int display_status, 
	    int find_type,
	    const char *campaign_id, 
	    int group_name_type, 
	    const char *group_name,
	    char *count);

    extern int ad_system_sql_get_group_set_keywoed(const char *group_id, const char *keyword);

    //推广->组->关键词
    extern int ad_system_sql_add_group_keyword(ad_system_sql_t *sql_conn, const char *account_id, const char *group_id, const char  *keyword);
    extern int ad_system_sql_get_group_keyword(ad_system_sql_t *p, const char *account_id, 
	    int page_index_s,
	    int page_index_e,
	    int display_status,
	    const char *campaign_id,
	    const char *group_id,
	    int find_type, 
	    int find_name_type,
	    const char *find_name);
    extern int ad_system_sql_get_group_keyword_report(ad_system_sql_t *p, const char *account_id, 
	    int display_status,
	    const char *campaign_id,
	    const char *group_id,
	    int find_type, 
	    int find_name_type,
	    const char *find_name);
    extern int ad_system_sql_get_group_keyword_count(ad_system_sql_t *p, const char *account_id, 
	    int display_status,
	    const char *campaign_id,
	    const char *group_id,
	    int find_type, 
	    int find_name_type,
	    const char *find_name);
    extern int ad_system_sql_del_group_keyword(const char  *keyword_id);

    //更新组状态
    extern int ad_system_sql_group_synch_state(const char *account_id, const char *group_id, const char *keyword_id);

    //推广->左侧列表
    extern int ad_system_oci_get_sperad_list(ad_system_sql_t *p, char *account_id, int display_status);

    //获取计划ID
    extern int ad_system_sql_get_campaign_id_from_group_or_creative(char *campaign_id, char *group_id, char *creative_id);

    //推广->获取创意预算
    extern int ad_system_sql_get_campaign_budget(const char *account_id, const char *campaign_id, const char *group_id, char *budget, char *bidding_model);

    //批量修改组出价
    extern int ad_system_sql_ad_system_batch_operation_group_price(const char *group_id, int price_type, const char *cpc_price, const char *cpm_price);

    //复制创意检测
    extern int ad_system_sql_copy_creative_check(
	    const char *account_id,
	    const char *campaign_id,
	    const char *group_id);

    //复制创意文字
    extern int ad_system_sql_copy_creative_text_r(
	    const char *creative_id,
	    const char *account_id,
	    const char *campaign_id,
	    const char *group_id);

    //复制创意图片
    extern int ad_system_sql_copy_creative_img_r(
	    const char *creative_id,
	    const char *account_id,
	    const char *campaign_id,
	    const char *group_id);

    extern int ad_system_sql_get_brand(ad_system_sql_t *p,
	    const char *account_id,
	    int brand_status,
	    int CREATIVE_STATUS,
	    int DEPT_TYPE, 
	    const char *DEPT_ARRAY, 
	    int REGION_TYPE, 
	    const char *REGION_ARRAY,
	    int SHOW_TIME_TYPE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    int SORT_TYPE,
	    int SORT_COLUMN,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_brand_count(char *count,
	    const char *account_id,
	    int brand_status,
	    int CREATIVE_STATUS,
	    int DEPT_TYPE, 
	    const char *DEPT_ARRAY, 
	    int REGION_TYPE, 
	    const char *REGION_ARRAY,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    int SORT_TYPE,
	    int SORT_COLUMN,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_add_brand(
	    const char *ACCOUNT_ID,
	    int REGION_TYPE,
	    const char *REGION_SERIALIZE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    const char *DEPT_CODE,
	    const char *PRICE,
	    const char *PROPOSER_ID);

    extern int ad_system_sql_get_brand_creative(ad_system_sql_t *p, int MOD_STATUS, const char *brand_id);
    extern int ad_system_sql_add_brand_creative(ad_system_sql_t *p,
	    int insert_type,
	    const char *BRAND_ID,
	    const char *ACCOUNT_ID,
	    const char *TITLE,
	    const char *DESCRIPTION,
	    const char *DESTINATION_URL,
	    const char *DISPLAY_URL,
	    const char *TELEPHONE,
	    const char *WEB_SITE,
	    const char *WEB_IM,
	    const char *GUAHAO_URL);

    extern int ad_system_sql_set_brand_creative(int update_type,
	    const char *BRAND_CREATIVE_ID,
	    const char *ACCOUNT_ID,
	    const char *TITLE,
	    const char *DESCRIPTION,
	    const char *DESTINATION_URL,
	    const char *DISPLAY_URL,
	    const char *TELEPHONE,
	    const char *WEB_SITE,
	    const char *WEB_IM,
	    const char *GUAHAO_URL);

    extern int ad_system_sql_add_brand_creative_append(ad_system_sql_t *p,
	    const char *brand_creative_id,
	    const char *img_path, 
	    const char *img_size, 
	    const char *img_url, 
	    int img_sequence);

    extern int ad_system_sql_set_brand_creative_append(int status, const char *brand_creative_id,
	    const char *img_path, const char *img_size, const char *img_url, int img_sequence);

    extern int ad_system_sql_del_brand(const char *brand_id_list);

    extern int ad_system_sql_get_account_domain(const char *account_id, char *domain);

    
    extern int ad_system_sql_get_brand_creative_id(char *brand_creative_id);

    extern int ad_system_sql_get_disease_code_type(char *account_id, int *disease_code_type);


#ifdef __cplusplus
}
#endif

#endif


