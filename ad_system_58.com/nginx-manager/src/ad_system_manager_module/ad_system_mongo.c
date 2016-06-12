
#include <stdio.h>
#include <stdlib.h>

#include "ad_system_mongo.h"
#include "ad_system_report.h"
#include "ad_system_region_code.h"

//账户管理->去投放中心
int ad_system_mongo_delivery_center_add(char *key, char *account_id)
{

    return 0;
}

//财务->媒体报告载入
int ad_system_mongo_report_media_load(lg_list_t *head,
	const char *visit_domain,
	const char *time_s,
	const char *time_e)
{

    return 0;
}

int ad_system_mongo_report_account_load(lg_list_t *head,
	const char *visit_domain,
	const char *time_s,
	const char *time_e,
	int mode)
{

    return 0;
}

int ad_system_mongo_get_media_domain_list(lg_list_t *head)
{

    return 0;
}

int ad_system_mongo_get_media_domain_unique(const char *domain_id, char *domain_str)
{

    return -1;
}

double ad_system_mongo_get_account_cost_total(ad_system_mongo_t *conn, const char *date_str, int account_id)
{

    return 0;
}


