
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
    extern char *ad_system_sql_string_escaped(char *dest, const  char *src, int mode);

    /************/
    /* 业务逻辑 */
    /************/

    //登录
    extern int ad_system_sql_login(const char *account_name, const char *password_md5, char *account_id);
    extern int ad_system_sql_get_get_title_info(char *account_id, char *account_name);

    extern int ad_system_sql_get_text_creative_count(int find_type, const char *find_name, int find_status, int balance_type, char *count);
    extern int ad_system_sql_get_append_creative_count(int find_type, char *find_name, int find_status, int balance_type, char *count);
    extern int ad_system_sql_get_img_creative_count(int find_type, char *find_name, int find_status, int balance_type, char *count);

    extern int ad_system_sql_get_append_creative_unique(ad_system_sql_t *p, const char *creative_id);
    extern int ad_system_sql_get_append_creative(ad_system_sql_t *p,
	    int find_type, 
	    const char *find_name, 
	    int find_status, 
	    int balance_type,
	    int sort_type, 
	    int sort_column, 
	    int index_s, 
	    int index_e);

    extern int ad_system_sql_get_manager_account_list(ad_system_sql_t *p,
	    int find_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    int status,
	    const char *account_id_str,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_text_creative(ad_system_sql_t *p,
	    int find_type, 
	    char *find_name, 
	    int find_status, 
	    int balance_type,
	    int sort_type, 
	    int sort_column, 
	    int index_s, 
	    int index_e);

    extern int ad_system_sql_get_img_creative(ad_system_sql_t *p,
	    int find_type, 
	    char *find_name,
	    int find_status, 
	    int dsp_status,
	    int balance_type,
	    int sort_type, 
	    int sort_column, 
	    int index_s, 
	    int index_e);

    extern int ad_system_sql_set_text_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason);
    extern int ad_system_sql_set_append_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason);
    extern int ad_system_sql_set_img_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason);

    extern int ad_system_sql_get_manager_account_list_count(char *count,
	    int find_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    int status,
	    const char *account_id);

    extern int ad_system_sql_set_manager_account_status(const char *account_id, int status);
    extern int ad_system_sql_set_manager_account_info(int update_type,
	    const char *account_id	      ,
	    const char *COMPANY_NAME          ,
	    const char *COMPANY_SITE          ,
	    const char *INDUSTRY              ,
	    const char *PROVINCE_ID           ,
	    const char *CITY_ID               ,
	    const char *ADDRESS               ,
	    const char *POSTCODE              ,
	    const char *CONTACT               ,
	    const char *TELEPHONE             ,
	    const char *FAX                   ,
	    const char *MOBILE                ,
	    const char *EMAIL                 ,
	    const char *WEBIM                 ,
	    const char *GUAHAO_TOKEN          ,
	    const char *HOSPITAL_IMG          ,
	    const char *LOGIN_PASSWORD	      ,
	    const char *REGION_NAME_CN        ,
	    int STATUS,
	    const char *SALES,
	    const char *CUSTOMER_SERVICE,
	    int PLACE_REGION,
	    int JOIN_DSP);

    extern int ad_system_sql_add_manager_account_name_check(const char *account_name);
    extern int ad_system_sql_manager_new_account_id(char *account_id);

    extern int ad_system_sql_add_manager_account(char *session_account_id, int insert_type,
	    char *account_id,
	    const char *LOGIN_NAME,
	    const char *LOGIN_PASSWORD,
	    const char *API_TOKEN,
	    const char *DOMAIN,
	    const char *COMPANY_NAME,
	    const char *COMPANY_SITE,
	    const char *INDUSTRY,
	    const char *PROVINCE_ID,
	    const char *CITY_ID,
	    const char *ADDRESS,
	    const char *POSTCODE,
	    const char *CONTACT,
	    const char *TELEPHONE,
	    const char *FAX,
	    const char *MOBILE,
	    const char *EMAIL,
	    const char *WEBIM,
	    const char *GUAHAO_TOKEN,
	    const char *REGION_NAME_CN,
	    const char *HOSPITAL_IMG,
	    const char *SALES,
	    const char *CUSTOMER_SERVICE,
	    int PLACE_REGION,
	    int JOIN_DSP);
    extern int ad_system_sql_del_manager_account(const char *account_id);

    extern int ad_system_sql_add_manager_announcement(const char *session_account_id, const char *send_time, const char *title, const char *content);

    int ad_system_sql_set_manager_announcement(int update_type, 
	    const char *session_account_id,
	    const char *announcement_id,
	    const char *send_time, 
	    const char *title, 
	    const char *content);

    extern int ad_system_sql_del_manager_announcement(const char *announcement_id);

    extern int ad_system_sql_get_manager_announcement_list(ad_system_sql_t *p,
	    int find_type,
	    const char *find_name,
	    int time_type,
	    const char *time_start,
	    const char *time_end,
	    int status,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_manager_announcement_list_count(char *count,
	    int find_type,
	    const char *find_name,
	    int time_type,
	    const char *time_start,
	    const char *time_end,
	    int status);

    extern int ad_system_sql_get_money_operation(ad_system_sql_t *p,
	    const char *account_id,
	    int find_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    int status,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_money_operation_count(char *count,
	    char *INVEST,
	    char *BALANCE,
	    char *CONSUMPTION,
	    int find_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    int status);

    extern int ad_system_sql_get_money_list_count(char *count,
	    int find_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    const char *account_id);

    extern int ad_system_sql_get_money_list(ad_system_sql_t *p,
	    int find_type,
	    int money_type,
	    int find_name_type,
	    const char *find_name,
	    const char *time_start,
	    const char *time_end,
	    const char *account_id,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_add_money_operation(
	    char *ACCOUNT_ID,
	    char *MONEY,
	    char *PAYMENT_TYPE,
	    char *ACCOUNTING_DOCUMENT_NUMBER,
	    char *ACCOUNTING_DOCUMENT_FILE,
	    char *ACCOUNTING_DOCUMENT_REMARK,
	    char *CREATE_BY,
	    char *LAST_UPDATE_BY,
	    char *BRAND_TYPE,
	    char *DISCOUNT);

    extern int campaign_region_check(const char *account_id);

    extern int ad_system_sql_get_brand(ad_system_sql_t *p,
	    int find_type,
	    const char *find_name,
	    int brand_status, 
	    int DEPT_TYPE, 
	    const char *DEPT_ARRAY, 
	    int REGION_TYPE, 
	    const char *REGION_ARRAY,
	    int SHOW_TIME_TYPE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    int CREATE_TIME_TYPE,
	    const char *CREATE_TIME_S,
	    const char *CREATE_TIME_E,
	    int SORT_TYPE,
	    int SORT_COLUMN,
	    int BALANCE_TYPE,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_brand_count(char *count,
	    int find_type,
	    const char *find_name,
	    int brand_status, 
	    int DEPT_TYPE, 
	    const char *DEPT_ARRAY, 
	    int REGION_TYPE, 
	    const char *REGION_ARRAY,
	    int SHOW_TIME_TYPE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    int CREATE_TIME_TYPE,
	    const char *CREATE_TIME_S,
	    const char *CREATE_TIME_E,
	    int BALANCE_TYPE);

    extern int ad_system_sql_find_brand_schedule(ad_system_sql_t *p,
	    int DEPT_TYPE,
	    const char *DEPT_ARRAY,
	    int REGION_TYPE,
	    const char *REGION_ARRAY,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    int page_index_s,
	    int page_index_e,
	    int page_type);

    extern int ad_system_sql_find_brand_schedule_count(char *count,
	    int DEPT_TYPE,
	    const char *DEPT_ARRAY,
	    int REGION_TYPE,
	    const char *REGION_ARRAY);

    extern int ad_system_sql_add_brand(
	    const char *ACCOUNT_ID,
	    int REGION_TYPE,
	    const char *REGION_SERIALIZE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    const char *DEPT_CODE,
	    const char *PRICE,
	    const char *PROPOSER_ID);

    extern int ad_system_sql_get_brand_creative(ad_system_sql_t *p,
	    int find_type, 
	    const char *find_name,
	    int brand_id_type,
	    const char *brand_id,
	    int status,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_brand_creative_count(char *count,
	    int find_type, 
	    const char *find_name,
	    int brand_id_type,
	    const char *brand_id,
	    int status,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    //推广->获取科室病种CODE
    extern int ad_system_sql_get_disease_code(ad_system_sql_t *p);

    extern int ad_system_sql_set_brand_creative(ad_system_sql_t *p, const char *brand_id, const char *auditor_id, int status, const char *REFUSE_REASON);
    extern int ad_system_sql_get_account_name(ad_system_sql_t *p, int find_type, const char *find_name);
    extern int ad_system_sql_brand_inspection(ad_system_sql_t *p,
	    const char *manager_id,
	    const char *brand_id_str, 
	    int status,
	    const char *REFUSE_REASON,
	    const char *ACCOUNT_ID,
	    int REGION_TYPE,
	    const char *REGION_SERIALIZE,
	    const char *DEPT_SERIALIZE,
	    const char *SHOW_TIME_S,
	    const char *SHOW_TIME_E,
	    const char *PRICE,
	    const char *PROPOSER_ID);

    extern int ad_system_sql_get_brand_unique(ad_system_sql_t *p, const char *brand_id);

    extern int ad_system_sql_brand_pric_check(ad_system_sql_t *p, const char *account_id, const char *PRICE);

    extern int ad_system_sql_get_manager_dsp_list(ad_system_sql_t *p,
	    const char *CPC_PRICE,
	    const char *BUDGET_MORE,
	    const char *BUDGET_LESS,
	    const char *ACCOUNT_NAME,
	    const char *REGION_SERIALIZE,
	    const char *DEPT_SERIALIZE,
	    int STATUS,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_get_manager_dsp_list_count(char *count,
	    const char *CPC_PRICE,
	    const char *BUDGET_MORE,
	    const char *BUDGET_LESS,
	    const char *ACCOUNT_NAME,
	    const char *REGION_SERIALIZE,
	    const char *DEPT_SERIALIZE,
	    int STATUS,
	    int sort_type,
	    int sort_column,
	    int page_index_s,
	    int page_index_e);

    extern int ad_system_sql_add_manager_dsp(ad_system_sql_t *p,
	    const char *TEXT_CREATIVE_ID,
	    const char *CREATIVE_NAME,
	    const char *IMG_SIZE,
	    const char *FILE_SIZE,
	    const char *FILE_PATH
	    );

    extern int ad_system_sql_get_manager_dsp(ad_system_sql_t *p, const char *TEXT_CREATIVE_ID);
    extern int ad_system_sql_set_manager_dsp(const char *TEXT_CREATIVE_ID, int status);
    extern int ad_system_sql_del_manager_dsp(const char *img_id);

#ifdef __cplusplus
}
#endif

#endif




