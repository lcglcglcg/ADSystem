
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lg_string.h"
#include "lg_config_file.h"

#include "ad_system_init.h"
#include "ad_system_region_code.h"
#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_redis.h"

int ad_system_init()
{
    srand(time(NULL));

    lg_list_t config_file_head;
    if (lg_config_file_load(&config_file_head, "/etc/ad_system.conf")) {
	fprintf(stdout, "lg_config_file_load err\n");
	return -1;
    }

    if (ad_system_sql_init(&config_file_head)) {
	fprintf(stdout, "ad_system_sql_init err\n");
	return -1;
    }

    if (ad_system_mongo_init(&config_file_head)) {
	fprintf(stdout, "ad_system_mongo_init err\n");
	return -1;
    }

    if (ad_system_redis_init(&config_file_head)) {
	fprintf(stdout, "ad_system_redis_init err\n");
	return -1;
    }

    //report find 使用
    if (ad_system_region_code_load("/etc/region_code.csv")) {
	fprintf(stdout, "region_code.csv err\n");
	return -1;
    }

    //发送前台使用
    if (lg_string_file_to_buffer("/ad_system/nginx-service/region_code.json", &ad_system_global_region_code.buffer, &ad_system_global_region_code.size)) {
	fprintf(stdout, "region_code.json err\n");
	return -1;
    }

    return 0;
}


