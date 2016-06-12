
#ifndef	__AD_SYSTEM_COMPUTING_H__
#define	__AD_SYSTEM_COMPUTING_H__

#include "lg_list.h"
#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#define	REPORT_FILE_PATH "/ad_system/images/html"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	int date_type;
	int media_type;
	char report_id[128];
	char report_path[1024];
	char domain_str[1024];
	char time_s[128];
	char time_e[128];

    } ad_system_report_thread_t;

    typedef struct {

	lg_list_t list;
	char date[128];
	int account_id;
	char account_name[2048];
	int impression;
	int click;
	double cost;
	double cpc;
	double cost_final;
	//double cost_discount;

    } ad_system_report_account_t;

    typedef struct {

	lg_list_t list;
	char date[128];
	char domain[2048];
	int account_id;
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;
	double cost_final;
	//double cost_discount;

    } ad_system_report_media_t;

    extern int ad_system_report_path_create(char *key, char *path);
    extern int ad_system_report_path_create_r(char *file_path, char *down_path);

    extern int ad_system_report_account_add(lg_list_t *head,
	    const char *date, 
	    int account_id,
	    int impression, 
	    int click, 
	    double cost, 
	    int mode);

    extern int ad_system_report_media_add(lg_list_t *head,
	    const char *date, 
	    const char *domain,
	    int account_id,
	    int impression, 
	    int click, 
	    double cost);

    extern void ad_system_report_free(lg_list_t *head);

    extern void *ad_system_report_media_create(void *arg);
    extern int ad_system_report_csv_media(lg_list_t *head, char *path);

    extern void *ad_system_report_account_create(void *arg);
    extern int ad_system_report_csv_account(lg_list_t *head, char *path, int mode, const char *time_s);

    extern int ad_system_hospital_image_copy(const char *account_id, const char *hospital_img);

    extern int ad_system_report_csv_money_operation(ad_system_sql_t *sql_conn, char *path);
    extern int ad_system_report_csv_money_list(ad_system_sql_t *sql_conn, char *path);

    double ad_system_report_cost_final(const char *date, double cost);
    extern double ad_system_report_cost_discount(ad_system_mongo_t *conn,
	    const char *date, //用于计算本时间之前的已消费总额
	    int account_id,	  //需要计算的账户ID
	    double cost_now);  //计算这次虚拟费用的实际消费

#ifdef __cplusplus
}
#endif

#endif




