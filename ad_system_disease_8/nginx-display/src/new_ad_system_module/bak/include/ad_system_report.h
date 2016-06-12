
#ifndef	__AD_SYSTEM_REPORT_H__
#define	__AD_SYSTEM_REPORT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lg_list.h"
#include "ad_system_sql.h"

#define	REPORT_FILE_PATH "/ad_system/images/html"
#define	REPORT_CPC 2
#define	REPORT_CPM 1

#ifdef __cplusplus
extern "C" { 
#endif

    extern int ad_system_report_path_create(char *key, char *path);
    extern int ad_system_report_path_create_r(char *file_path, char *down_path);

    /* report arg */

    typedef struct {

	int report_type;
	char report_key[128];
	char report_path[1024];

	int display_style;
	char account_id[128];

	int time_type;
	char time_s[128];
	char time_e[128];

    } ad_system_report_arg_t;

    /* standard report */

    typedef struct {

	lg_list_t next;
	int creative_id;
	char creative_name[1024];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_standard_creative_t;

    typedef struct {

	lg_list_t next;
	lg_list_t head;
	int group_id;
	char group_name[1024];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_standard_group_t;

    typedef struct {

	lg_list_t next;
	lg_list_t head;
	int campaign_id;
	char campaign_name[1024];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_standard_campaign_t;

    typedef struct {

	lg_list_t next;
	lg_list_t head;
	char date[128];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_standard_t;

    extern int ad_system_report_standard_add(lg_list_t *head, int display_style,
	    const char *date,
	    int campaign_id,
	    int group_id,
	    int creative_id,
	    int impression,
	    int click,
	    double cost);

    extern void *ad_system_report_standard_create(ad_system_report_arg_t *p);
    extern void ad_system_report_standard_free(lg_list_t *head);

    /* report complex */

    typedef struct {

	lg_list_t next;
	char region_child[256];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_complex_child_t;

    typedef struct {

	lg_list_t next;
	lg_list_t head;
	char region_father[256];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_complex_father_t;

    typedef struct {

	lg_list_t next;
	lg_list_t head;
	char date[128];
	int impression;
	int click;
	double cost;
	double ctr;
	double cpc;
	double cpm;

    } ad_system_report_complex_t;

    extern int ad_system_report_complex_add(lg_list_t *head,
	    const char *date,
	    int region_code,
	    int impression,
	    int click,
	    double cost);

    extern void *ad_system_report_complex_create(ad_system_report_arg_t *p);
    extern void ad_system_report_complex_free(lg_list_t *head);


    /* report list */
    typedef struct {

	lg_list_t list;
	char report_id[64];
	char report_name[256];
	char report_type[128];
	char range_time[128];
	char create_time[128];
	char status[128];
	char path[1024];

    } ad_system_mongo_report_list_t;

    extern int ad_system_mongo_report_list_add(const char *account_id,
	    const char *report_id,
	    const char *report_name,
	    int report_type,
	    int text_or_img,
	    const char *time_range,
	    const char *path);

    extern int ad_system_mongo_report_list_set(char *account_id, char *report_id, int status);
    extern int ad_system_mongo_report_list_find(char *account_id, int report_belong, lg_list_t *head);
    extern int ad_system_mongo_report_list_del(char *account_id, const char *report_id);


    extern int ad_system_report_csv_group_keyword(ad_system_sql_t *sql_conn, char *path);



#ifdef __cplusplus
}
#endif

#endif


