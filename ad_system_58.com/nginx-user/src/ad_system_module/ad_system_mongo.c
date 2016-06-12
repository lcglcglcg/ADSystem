
#include <stdio.h>
#include <stdlib.h>

#include <bson.h>
#include <mongoc.h>

#include "lg_list.h"
#include "lg_string.h"

#include "ad_system_mongo.h"
#include "ad_system_report.h"
#include "ad_system_region_code.h"

int ad_system_mongo_report_standard_load(lg_list_t *head, 
	int display_style, 
	int bidding_model, 
	const char *account_id, 
	const char *time_s, 
	const char *time_e,
	int time_type)
{

    return 0;
}

//地域报告
int ad_system_mongo_report_region_load(lg_list_t *head, 
	int display_style, int bidding_model, const char *account_id, const char *time_s, const char *time_e,int time_type)
{

    return 0;
}

int ad_system_mongo_report_list_add(
	const char *account_id,
	const char *report_id,
	const char *report_name,
	int report_type,
	int report_belong,
	const char *time_range,
	const char *path)
{

    return 0;
}

int ad_system_mongo_report_list_set(char *account_id, char *report_id, int status)
{

    return 0;
}

int ad_system_mongo_report_list_element(const bson_t *doc, lg_list_t *head)
{

    return 0;
}

int ad_system_mongo_report_list_find(char *account_id, int report_belong, lg_list_t *head)
{

    return 0;
}

int ad_system_mongo_report_list_del(char *account_id, const char *report_id)
{

    return 0;
}

//后台管理登录
int ad_system_mongo_delivery_center_find(const char *key, char *account_id_str)
{

    return 0;
}

int ad_system_mongo_stat_load(ad_system_mongo_stat_t *p, const char *table, int display_style, const char *account_id)
{

    return 0;
}

int ad_system_mongo_stat_list_load(lg_list_t *head, const char *table, int display_style, const char *account_id, const char *name)
{

    return 0;
}

ad_system_mongo_stat_t *ad_system_mongo_stat_list_find(lg_list_t *head, const char *name)
{

    return NULL;
}




