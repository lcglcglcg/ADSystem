
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lg_list.h"
#include "lg_string.h"
#include "lg_config_file.h"
#include "ad_system_init.h"

#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_redis.h"
#include "ad_system_region_code.h"

static char *ad_system_region_code_buffer = NULL;
static size_t ad_system_region_code_size = 0;
static char *delivery_center_domain = NULL;
extern int lg_region_code_load(const char *path);

char *get_ad_system_region_code_buffer()
{
    return ad_system_region_code_buffer;
}

size_t get_ad_system_region_code_size()
{
    return ad_system_region_code_size;
}

char *get_delivery_center_domain()
{
    return delivery_center_domain;
}

int ad_system_init()
{
    srand(time(NULL));

    lg_list_t config_file_head;
    if (lg_config_file_load(&config_file_head, "/ad_system/ad_system_58.com/conf/ad_system.conf")) {
	fprintf(stdout, "lg_config_file_load err\n");
	return -1;
    }

    if (ad_system_sql_init(&config_file_head)) {
	fprintf(stdout, "ad_system_sql_init err\n");
	return -1;
    }

#if 0
    if (ad_system_mongo_init(&config_file_head)) {
	fprintf(stdout, "ad_system_mongo_init err\n");
	return -1;
    }
#endif

    if (ad_system_redis_init(&config_file_head)) {
	fprintf(stdout, "ad_system_redis_init err\n");
	return -1;
    }

    //report find 使用
    if (ad_system_region_code_load("/etc/ad_system_region_code.csv")) {
	fprintf(stdout, "region_code.csv err\n");
	return -1;
    }

    //发送前台使用
    if (lg_string_file_to_buffer("/etc/ad_system_region_code.json", &ad_system_region_code_buffer, &ad_system_region_code_size)) {
	fprintf(stdout, "region_code.json err\n");
	return -1;
    }

    if (!(delivery_center_domain = lg_config_file_find(&config_file_head, "delivery_center_domain"))) {
	fprintf(stdout, "delivery_center_domain err\n");
	return -1;;
    }

    if (lg_region_code_load("/etc/ad_system_region_code.csv")) {
	fprintf(stdout, "region_code.csv err\n");
	return -1;
    }

    return 0;
}


