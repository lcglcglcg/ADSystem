
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lg_string.h"
#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_report.h"
#include "ad_system_merger.h"
#include "ad_system_json_packet.h"

int ad_system_merger_campaign_cmp(void *priv, lg_list_t *a, lg_list_t *b)
{

    ad_system_merger_campaign_t *campaign_a = NULL, *campaign_b = NULL;
    campaign_a = lg_list_entry(a, ad_system_merger_campaign_t, list);
    campaign_b = lg_list_entry(b, ad_system_merger_campaign_t, list);

    switch(*(int *)priv) {

	case 1101:
	    return strcmp(campaign_a->CAMPAIGN_NAME, campaign_b->CAMPAIGN_NAME);
	case 1201:
	    return strcmp(campaign_b->CAMPAIGN_NAME, campaign_a->CAMPAIGN_NAME);

	case 1102:
	    return campaign_a->CAMPAIGN_STATE - campaign_b->CAMPAIGN_STATE;
	case 1202:
	    return campaign_b->CAMPAIGN_STATE - campaign_a->CAMPAIGN_STATE;

	case 1103:
	    return campaign_a->CAMPAIGN_BUDGET - campaign_b->CAMPAIGN_BUDGET;
	case 1203:
	    return campaign_b->CAMPAIGN_BUDGET - campaign_a->CAMPAIGN_BUDGET;

	case 1104:
	    return campaign_a->CAMPAIGN_IMPRESSION - campaign_b->CAMPAIGN_IMPRESSION;
	case 1204:
	    return campaign_b->CAMPAIGN_IMPRESSION - campaign_a->CAMPAIGN_IMPRESSION;

	case 1105:
	    return campaign_a->CAMPAIGN_CLICK - campaign_b->CAMPAIGN_CLICK;
	case 1205:
	    return campaign_b->CAMPAIGN_CLICK - campaign_a->CAMPAIGN_CLICK;

	case 1106:
	    return campaign_a->CAMPAIGN_CTR*1000 - campaign_b->CAMPAIGN_CTR*1000;
	case 1206:
	    return campaign_b->CAMPAIGN_CTR*1000 - campaign_a->CAMPAIGN_CTR*1000;

	case 1107:
	    return campaign_a->CAMPAIGN_CPC*1000 - campaign_b->CAMPAIGN_CPC*1000;
	case 1207:
	    return campaign_b->CAMPAIGN_CPC*1000 - campaign_a->CAMPAIGN_CPC*1000;

	case 1108:
	    return campaign_a->CAMPAIGN_COST*1000 - campaign_b->CAMPAIGN_COST*1000;
	case 1208:
	    return campaign_b->CAMPAIGN_COST*1000 - campaign_a->CAMPAIGN_COST*1000;

	case 1109:
	    return campaign_a->CAMPAIGN_CPM - campaign_b->CAMPAIGN_CPM;
	case 1209:
	    return campaign_b->CAMPAIGN_CPM - campaign_a->CAMPAIGN_CPM;

	default:
	    return campaign_b->CAMPAIGN_CLICK - campaign_a->CAMPAIGN_CLICK;
    }

    return 0;
}

int ad_system_merger_group_cmp(void *priv, lg_list_t *a, lg_list_t *b)
{

    //fprintf(stdout, "[ad_system_merger_cmp][%d]\n", *(int *)priv);
    ad_system_merger_group_t *group_a = NULL, *group_b = NULL;
    group_a = lg_list_entry(a, ad_system_merger_group_t, list);
    group_b = lg_list_entry(b, ad_system_merger_group_t, list);

    switch(*(int *)priv) {

	case 2101:
	    return strcmp(group_a->GROUP_NAME, group_b->GROUP_NAME);
	case 2201:
	    return strcmp(group_b->GROUP_NAME, group_a->GROUP_NAME);

	case 2102:
	    return strcmp(group_a->CAMPAIGN_NAME, group_b->CAMPAIGN_NAME);
	case 2202:
	    return strcmp(group_b->CAMPAIGN_NAME, group_a->CAMPAIGN_NAME);

	case 2103:
	    return group_a->GROUP_STATE - group_b->GROUP_STATE;
	case 2203:
	    return group_b->GROUP_STATE - group_a->GROUP_STATE;

	case 2104:
	    return group_a->GROUP_CPC_PRICE*1000 - group_b->GROUP_CPC_PRICE*1000;
	case 2204:
	    return group_b->GROUP_CPC_PRICE*1000 - group_a->GROUP_CPC_PRICE*1000;

	case 2105:
	    return group_a->GROUP_IMPRESSION - group_b->GROUP_IMPRESSION;
	case 2205:
	    return group_b->GROUP_IMPRESSION - group_a->GROUP_IMPRESSION;

	case 2106:
	    return group_a->GROUP_CLICK - group_b->GROUP_CLICK;
	case 2206:
	    return group_b->GROUP_CLICK - group_a->GROUP_CLICK;

	case 2107:
	    return group_a->GROUP_CTR*1000  - group_b->GROUP_CTR*1000;
	case 2207:
	    return group_b->GROUP_CTR*1000 - group_a->GROUP_CTR*1000;

	case 2108:
	    return group_a->GROUP_CPC*1000 - group_b->GROUP_CPC*1000;
	case 2208:
	    return group_b->GROUP_CPC*1000 - group_a->GROUP_CPC*1000;

	case 2109:
	    return group_a->GROUP_COST*1000 - group_b->GROUP_COST*1000;
	case 2209:
	    return group_b->GROUP_COST*1000 - group_a->GROUP_COST*1000;

	case 2110:
	    return group_a->GROUP_CPM - group_b->GROUP_CPM;
	case 2210:
	    return group_b->GROUP_CPM - group_a->GROUP_CPM;

	case 2111:
	    return group_a->GROUP_CPM_PRICE*1000 - group_b->GROUP_CPM_PRICE*1000;
	case 2211:
	    return group_b->GROUP_CPM_PRICE*1000 - group_a->GROUP_CPM_PRICE*1000;

	default:
	    return group_b->GROUP_CLICK - group_a->GROUP_CLICK;
    }

    return 0;
}

int ad_system_merger_creative_text_cmp(void *priv, lg_list_t *a, lg_list_t *b)
{

    ad_system_merger_creative_text_t *creative_text_a = NULL;
    creative_text_a = lg_list_entry(a, ad_system_merger_creative_text_t, list);

    ad_system_merger_creative_text_t *creative_text_b = NULL;
    creative_text_b = lg_list_entry(b, ad_system_merger_creative_text_t, list);

    switch(*(int *)priv) {

	case 3101:
	    return strcmp(creative_text_a->CREATIVE_NAME, creative_text_b->CREATIVE_NAME);
	case 3201:
	    return strcmp(creative_text_b->CREATIVE_NAME, creative_text_a->CREATIVE_NAME);

	case 3102:
	    return strcmp(creative_text_a->CAMPAIGN_NAME, creative_text_b->CAMPAIGN_NAME);
	case 3202:
	    return strcmp(creative_text_b->CAMPAIGN_NAME, creative_text_a->CAMPAIGN_NAME);

	case 3103:
	    return strcmp(creative_text_a->GROUP_NAME, creative_text_b->GROUP_NAME);
	case 3203:
	    return strcmp(creative_text_b->GROUP_NAME, creative_text_a->GROUP_NAME);

	case 3104:
	    return creative_text_a->CREATIVE_STATE - creative_text_b->CREATIVE_STATE;
	case 3204:
	    return creative_text_b->CREATIVE_STATE - creative_text_a->CREATIVE_STATE;

	    //case 3105:

	case 3106:
	    return creative_text_a->CREATIVE_IMPRESSION - creative_text_b->CREATIVE_IMPRESSION;
	case 3206:
	    return creative_text_b->CREATIVE_IMPRESSION - creative_text_a->CREATIVE_IMPRESSION;

	case 3107:
	    return creative_text_a->CREATIVE_CLICK - creative_text_b->CREATIVE_CLICK;
	case 3207:
	    return creative_text_b->CREATIVE_CLICK - creative_text_a->CREATIVE_CLICK;

	case 3108:
	    return creative_text_a->CREATIVE_CTR*1000 - creative_text_b->CREATIVE_CTR*1000;
	case 3208:
	    return creative_text_b->CREATIVE_CTR*1000 - creative_text_a->CREATIVE_CTR*1000;

	case 3109:
	    return creative_text_a->CREATIVE_CPC*1000 - creative_text_b->CREATIVE_CPC*1000;
	case 3209:
	    return creative_text_b->CREATIVE_CPC*1000 - creative_text_a->CREATIVE_CPC*1000;

	case 3110:
	    return creative_text_a->CREATIVE_COST*1000 - creative_text_b->CREATIVE_COST*1000;
	case 3210:
	    return creative_text_b->CREATIVE_COST*1000 - creative_text_a->CREATIVE_COST*1000;

	case 3111:
	    return creative_text_a->CREATIVE_CPM  - creative_text_b->CREATIVE_CPM;
	case 3211:
	    return creative_text_b->CREATIVE_CPM  - creative_text_a->CREATIVE_CPM;

	default:
	    return creative_text_b->CREATIVE_CLICK - creative_text_a->CREATIVE_CLICK;
    }

    return 0;
}

int ad_system_merger_creative_img_cmp(void *priv, lg_list_t *a, lg_list_t *b)
{

    ad_system_merger_creative_img_t *creative_img_a = NULL;
    creative_img_a = lg_list_entry(a, ad_system_merger_creative_img_t, list);

    ad_system_merger_creative_img_t *creative_img_b = NULL;
    creative_img_b = lg_list_entry(b, ad_system_merger_creative_img_t, list);

    switch(*(int *)priv) {

	case 4101:
	    return strcmp(creative_img_a->CREATIVE_NAME, creative_img_b->CREATIVE_NAME);
	case 4201:
	    return strcmp(creative_img_b->CREATIVE_NAME, creative_img_a->CREATIVE_NAME);

	case 4102:
	    return strcmp(creative_img_a->CAMPAIGN_NAME, creative_img_b->CAMPAIGN_NAME);
	case 4202:
	    return strcmp(creative_img_b->CAMPAIGN_NAME, creative_img_a->CAMPAIGN_NAME);

	case 4103:
	    return strcmp(creative_img_a->GROUP_NAME, creative_img_b->GROUP_NAME);
	case 4203:
	    return strcmp(creative_img_b->GROUP_NAME, creative_img_a->GROUP_NAME);

	case 4104:
	    return creative_img_a->CREATIVE_STATE - creative_img_b->CREATIVE_STATE;
	case 4204:
	    return creative_img_b->CREATIVE_STATE - creative_img_a->CREATIVE_STATE;

	    //case 4105:

	case 4106:
	    return creative_img_a->CREATIVE_IMPRESSION - creative_img_b->CREATIVE_IMPRESSION;
	case 4206:
	    return creative_img_b->CREATIVE_IMPRESSION - creative_img_a->CREATIVE_IMPRESSION;

	case 4107:
	    return creative_img_a->CREATIVE_CLICK - creative_img_b->CREATIVE_CLICK;
	case 4207:
	    return creative_img_b->CREATIVE_CLICK - creative_img_a->CREATIVE_CLICK;

	case 4108:
	    return creative_img_a->CREATIVE_CTR*1000 - creative_img_b->CREATIVE_CTR*1000;
	case 4208:
	    return creative_img_b->CREATIVE_CTR*1000 - creative_img_a->CREATIVE_CTR*1000;

	case 4109:
	    return creative_img_a->CREATIVE_CPC*1000 - creative_img_b->CREATIVE_CPC*1000;
	case 4209:
	    return creative_img_b->CREATIVE_CPC*1000 - creative_img_a->CREATIVE_CPC*1000;

	case 4110:
	    return creative_img_a->CREATIVE_COST*1000 - creative_img_b->CREATIVE_COST*1000;
	case 4210:
	    return creative_img_b->CREATIVE_COST*1000 - creative_img_a->CREATIVE_COST*1000;

	case 4111:
	    return creative_img_a->CREATIVE_CPM  - creative_img_b->CREATIVE_CPM;
	case 4211:
	    return creative_img_b->CREATIVE_CPM  - creative_img_a->CREATIVE_CPM;

	default:
	    return creative_img_b->CREATIVE_CLICK - creative_img_a->CREATIVE_CLICK;
    }

    return 0;
}

int ad_system_merger_schedule(const char *sysdate, const char *schedule)
{

    int week = 0;
    if (strncmp(sysdate, "MON", 3) == 0) {
	week = 0;
    } else if (strncmp(sysdate, "TUE", 3) == 0) {
	week = 1;
    } else if (strncmp(sysdate, "WED", 3) == 0) {
	week = 2;
    } else if (strncmp(sysdate, "THU", 3) == 0) {
	week = 3;
    } else if (strncmp(sysdate, "FRI", 3) == 0) {
	week = 4;
    } else if (strncmp(sysdate, "SAT", 3) == 0) {
	week = 5;
    } else if (strncmp(sysdate, "SUN", 3) == 0) {
	week = 6;
    } else {
	return -1;
    }

    int hour = atoi(sysdate + 3);
    if (hour < 0 && hour > 23)
	return -1;

    if (schedule[week * 24 + hour] == '1')
	return 1;
    else if (schedule[week * 24 + hour] == '0')
	return 0;

    return -1;
}

int ad_system_merger_campaign(
	ad_system_sql_t *sql_conn, 
	lg_list_t *mongo_head, 
	ad_system_packet_json_data_t *json,
	lg_list_t *merger_head,
	ad_system_merger_campaign_info_t *info)
{

    lg_list_init(merger_head);
    memset(info, 0, sizeof(ad_system_merger_campaign_info_t));

    ad_system_merger_campaign_t temp;
    while(ad_system_sql_fetch_next(sql_conn)) {

	memset(&temp, 0, sizeof(ad_system_merger_campaign_t));
	sprintf(temp.CAMPAIGN_ID, "%s",  ad_system_sql_get_string(sql_conn, NULL, 1));
	sprintf(temp.CAMPAIGN_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 2));
	int CAMPAIGN_STATE = ad_system_sql_get_number(sql_conn, NULL, 3);
	int PAUSE = ad_system_sql_get_number(sql_conn, NULL, 4);
	temp.CAMPAIGN_BUDGET = ad_system_sql_get_double(sql_conn, NULL, 5);
	temp.SCHEDULE_TYPE = ad_system_sql_get_number(sql_conn, NULL, 6);
	sprintf(temp.SCHEDULE_SERIALIZE, "%s", ad_system_sql_get_string(sql_conn, NULL, 7));
	const char *SCHEDULE_SYS_TIME = ad_system_sql_get_string(sql_conn, NULL, 8);
	temp.REGION_TYPE = ad_system_sql_get_number(sql_conn, NULL, 9);
	sprintf(temp.REGION_SERIALIZE, "%s", ad_system_sql_get_string(sql_conn, NULL, 10));
	sprintf(temp.BEGIN_TIME, "%s", ad_system_sql_get_string(sql_conn, NULL, 11));
	sprintf(temp.END_TIME, "%s", ad_system_sql_get_string(sql_conn, NULL, 12));
	int DATE_SUB_BEGIN_TIME = ad_system_sql_get_number(sql_conn, NULL, 13);
	int DATE_SUB_END_TIME = ad_system_sql_get_number(sql_conn, NULL, 14);

	if (PAUSE == 2) {
	    temp.CAMPAIGN_STATE = 5;//暂停

	} else if (DATE_SUB_BEGIN_TIME == 2) {
	    temp.CAMPAIGN_STATE = 4;//未开始

	} else if (DATE_SUB_END_TIME == 2) {
	    temp.CAMPAIGN_STATE = 6;//下线

	} else if (CAMPAIGN_STATE == 13) {
	    temp.CAMPAIGN_STATE = 3;//预算不足

	} else if (temp.SCHEDULE_TYPE == 2 
		&& ad_system_merger_schedule(SCHEDULE_SYS_TIME, temp.SCHEDULE_SERIALIZE) == 0) {
	    temp.CAMPAIGN_STATE = 2;//暂停时段

	} else {
	    temp.CAMPAIGN_STATE = 1;//有效
	}

	//结束时间类型
	if (*temp.END_TIME) temp.TIME_TYPE = 2;
	else temp.TIME_TYPE = 1;

	if (json->FIND_STATUS == 100 || json->FIND_STATUS == temp.CAMPAIGN_STATE) {

	    ad_system_mongo_stat_t *mongo_node = NULL;
	    if ((mongo_node = ad_system_mongo_stat_list_find(mongo_head, temp.CAMPAIGN_ID))) {

		double ctr = 0;
		if (mongo_node->click && mongo_node->impression)
		    ctr = (double)mongo_node->click / (double)mongo_node->impression;
		double cpc = 0;
		if (mongo_node->cost && mongo_node->click)
		    cpc = mongo_node->cost / (double)mongo_node->click;
		double cpm = 0;
		if (mongo_node->cost && mongo_node->impression)
		    cpm = mongo_node->cost / (double)mongo_node->impression * (double)1000;

		temp.CAMPAIGN_IMPRESSION = mongo_node->impression;
		temp.CAMPAIGN_CLICK = mongo_node->click;
		temp.CAMPAIGN_COST = mongo_node->cost;
		temp.CAMPAIGN_CTR = ctr;
		temp.CAMPAIGN_CPC = cpc;
		temp.CAMPAIGN_CPM = cpm;
	    }

	    ad_system_merger_campaign_t *node = NULL;
	    if ((node = malloc(sizeof(ad_system_merger_campaign_t)))) {

		memcpy(node, &temp, sizeof(ad_system_merger_campaign_t));
		lg_list_add(merger_head, &node->list);

		if (temp.CAMPAIGN_STATE == 1) 
		    info->CAMPAIGN_ENABLED++;

		info->CAMPAIGN_COUNT++;
		info->CAMPAIGN_BUDGET += temp.CAMPAIGN_BUDGET;
		info->CAMPAIGN_IMPRESSION += temp.CAMPAIGN_IMPRESSION;
		info->CAMPAIGN_CLICK += temp.CAMPAIGN_CLICK;
		info->CAMPAIGN_COST += temp.CAMPAIGN_COST;
	    }
	}
    }

    if (info->CAMPAIGN_CLICK && info->CAMPAIGN_IMPRESSION)
	info->CAMPAIGN_CTR = (double)info->CAMPAIGN_CLICK / (double)info->CAMPAIGN_IMPRESSION;
    if (info->CAMPAIGN_COST && info->CAMPAIGN_CLICK)
	info->CAMPAIGN_CPC = (double)info->CAMPAIGN_COST / (double)info->CAMPAIGN_CLICK;
    if (info->CAMPAIGN_COST && info->CAMPAIGN_IMPRESSION)
	info->CAMPAIGN_CPM = (double)info->CAMPAIGN_COST / (double)info->CAMPAIGN_IMPRESSION * 1000;

    int ad_system_merger_cmp_code = 1000 + json->SORT_TYPE * 100 + json->SORT_COLUMN;
    lg_list_sort(&ad_system_merger_cmp_code, merger_head, 4096, ad_system_merger_campaign_cmp);
    return 0;
}

int ad_system_merger_group(
	ad_system_sql_t *sql_conn, 
	lg_list_t *mongo_head,
	ad_system_packet_json_data_t *json,
	lg_list_t *merger_head,
	ad_system_merger_group_info_t *info)
{

    lg_list_init(merger_head);
    memset(info, 0, sizeof(ad_system_merger_group_info_t));

    ad_system_merger_group_t temp;
    while(ad_system_sql_fetch_next(sql_conn)) {

	memset(&temp, 0, sizeof(ad_system_merger_group_t));
	sprintf(temp.CAMPAIGN_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 1));
	sprintf(temp.CAMPAIGN_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 2));
	sprintf(temp.GROUP_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 3));
	sprintf(temp.GROUP_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 4));
	temp.GROUP_CPC_PRICE = ad_system_sql_get_double(sql_conn, NULL, 5);
	temp.GROUP_CPM_PRICE = ad_system_sql_get_double(sql_conn, NULL, 6);
	int GROUP_STATE = ad_system_sql_get_number(sql_conn, NULL, 7);
	int CAMPAIGN_PAUSE = ad_system_sql_get_number(sql_conn, NULL, 8);
	int CAMPAIGN_STATE = ad_system_sql_get_number(sql_conn, NULL, 9);
	int SCHEDULE_TYPE = ad_system_sql_get_number(sql_conn, NULL, 10);
	const char *SCHEDULE_SYS_TIME = ad_system_sql_get_string(sql_conn, NULL, 11);
	const char *SCHEDULE_SERIALIZE = ad_system_sql_get_string(sql_conn, NULL, 12);
	int DATE_SUB_BEGIN_TIME = ad_system_sql_get_number(sql_conn, NULL, 13);
	int DATE_SUB_END_TIME = ad_system_sql_get_number(sql_conn, NULL, 14);
	temp.GROUP_TARGETING_TYPE = ad_system_sql_get_number(sql_conn, NULL, 15);
	sprintf(temp.GROUP_DISEASE, "%s", ad_system_sql_get_string(sql_conn, NULL, 16));

	if (GROUP_STATE == 2) {
	    temp.GROUP_STATE = 8;//组暂停

	} else if (CAMPAIGN_PAUSE == 2) {
	    temp.GROUP_STATE = 5;//计划暂停

	} else if (DATE_SUB_BEGIN_TIME == 2) {
	    temp.GROUP_STATE = 4;//未开始

	} else if (DATE_SUB_END_TIME == 2) {
	    temp.GROUP_STATE = 6;//下线

	} else if (CAMPAIGN_STATE == 13) {
	    temp.GROUP_STATE = 3;//预算不足

	} else if (SCHEDULE_TYPE == 2 
		&& ad_system_merger_schedule(SCHEDULE_SYS_TIME, SCHEDULE_SERIALIZE) == 0) {
	    temp.GROUP_STATE = 2;//暂停时段

#if 0
	} else if (temp.GROUP_CPC_PRICE <= 0.6) {
	    temp.GROUP_STATE = 7;//CPC出价低

	} else if (temp.GROUP_CPC_PRICE <= 0.6) {
	    temp.GROUP_STATE = 7;//CPC出价低
#endif

	} else {
	    temp.GROUP_STATE = 1;//有效
	}


		temp.GROUP_IMPRESSION = 0;
		temp.GROUP_CLICK = 0;
		temp.GROUP_COST = 0;
		temp.GROUP_CTR = 0;
		temp.GROUP_CPC = 0;
		temp.GROUP_CPM = 0;
	    
	// fprintf(stdout, "%s::%s::%d\n", temp.GROUP_ID, temp.GROUP_NAME, temp.GROUP_IMPRESSION);

	if (json->FIND_STATUS == 100 || json->FIND_STATUS == temp.GROUP_STATE) {

	    ad_system_mongo_stat_t *mongo_node = NULL;
	    if ((mongo_node = ad_system_mongo_stat_list_find(mongo_head, temp.GROUP_ID))) {

		double ctr = 0;
		if (mongo_node->click && mongo_node->impression)
		    ctr = (double)mongo_node->click / (double)mongo_node->impression;
		double cpc = 0;
		if (mongo_node->cost && mongo_node->click)
		    cpc = mongo_node->cost / (double)mongo_node->click;
		double cpm = 0;
		if (mongo_node->cost && mongo_node->impression)
		    cpm = mongo_node->cost / (double)mongo_node->impression * (double)1000;

		temp.GROUP_IMPRESSION = mongo_node->impression;
		temp.GROUP_CLICK = mongo_node->click;
		temp.GROUP_COST = mongo_node->cost;
		temp.GROUP_CTR = ctr;
		temp.GROUP_CPC = cpc;
		temp.GROUP_CPM = cpm;
	    
		// fprintf(stdout, "%s::%d\n", temp.GROUP_ID, mongo_node->impression);
	    }
	
	    // fprintf(stdout, "%s::%s::%d\n", temp.GROUP_ID, temp.GROUP_NAME, temp.GROUP_IMPRESSION);
	    
	    ad_system_merger_group_t *node = NULL;
	    if ((node = malloc(sizeof(ad_system_merger_group_t)))) {

		memcpy(node, &temp, sizeof(ad_system_merger_group_t));
		lg_list_add(merger_head, &node->list);

		if (node->GROUP_STATE == 1)
		    info->GROUP_ENABLED++;

		info->GROUP_COUNT++;
		info->GROUP_IMPRESSION += node->GROUP_IMPRESSION;
		info->GROUP_CLICK += node->GROUP_CLICK;
		info->GROUP_COST += node->GROUP_COST;
	    }
	}

	    // fprintf(stdout, "%s::%s::%d\n", temp.GROUP_ID, temp.GROUP_NAME, temp.GROUP_IMPRESSION);
    }

    if (info->GROUP_CLICK && info->GROUP_IMPRESSION)
	info->GROUP_CTR = (double)info->GROUP_CLICK / (double)info->GROUP_IMPRESSION;
    if (info->GROUP_COST && info->GROUP_CLICK)
	info->GROUP_CPC = (double)info->GROUP_COST / (double)info->GROUP_CLICK;
    if (info->GROUP_COST && info->GROUP_IMPRESSION)
	info->GROUP_CPM = (double)info->GROUP_COST / (double)info->GROUP_IMPRESSION * 1000;

    int ad_system_merger_cmp_code = 2000 + json->SORT_TYPE * 100 + json->SORT_COLUMN;
    lg_list_sort(&ad_system_merger_cmp_code, merger_head, 4096, ad_system_merger_group_cmp);
    return 0;

}

int ad_system_merger_creative_text(
	ad_system_sql_t *sql_conn, 
	lg_list_t *mongo_head, 
	ad_system_packet_json_data_t *json,
	lg_list_t *merger_head,
	ad_system_merger_creative_info_t *info)
{

    lg_list_init(merger_head);
    memset(info, 0, sizeof(ad_system_merger_creative_info_t));

    ad_system_merger_creative_text_t temp;
    while(ad_system_sql_fetch_next(sql_conn)) {

	memset(&temp, 0, sizeof(ad_system_merger_creative_text_t));
	sprintf(temp.CAMPAIGN_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 1));
	sprintf(temp.CAMPAIGN_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 2));
	sprintf(temp.GROUP_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 3));
	sprintf(temp.GROUP_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 4));
	sprintf(temp.CREATIVE_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 5));
	sprintf(temp.CREATIVE_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 6));
	int CREATIVE_PAUSE = ad_system_sql_get_number(sql_conn, NULL, 7);
	temp.CREATIVE_STATE = ad_system_sql_get_number(sql_conn, NULL, 8);
	sprintf(temp.CREATIVE_REFUSE_REASON, "%s", ad_system_sql_get_string(sql_conn, NULL, 9));
	sprintf(temp.CREATIVE_DESCRIPTION, "%s", ad_system_sql_get_string(sql_conn, NULL, 10));
	sprintf(temp.CREATIVE_DESTINATION_URL, "%s", ad_system_sql_get_string(sql_conn, NULL, 11));
	sprintf(temp.CREATIVE_DISPLAY_URL, "%s", ad_system_sql_get_string(sql_conn, NULL, 12));
	sprintf(temp.CREATIVE_APPEND_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 13));
	sprintf(temp.MOBILE_DESTINATION_URL, "%s", ad_system_sql_get_string(sql_conn, NULL, 14));
	sprintf(temp.MOBILE_DISPLAY_URL, "%s", ad_system_sql_get_string(sql_conn, NULL, 15));

	if (CREATIVE_PAUSE == 2)
	    temp.CREATIVE_STATE = 3;

	temp.CREATIVE_APPEND_TYPE = 1;
	if (temp.CREATIVE_APPEND_ID)
	    temp.CREATIVE_APPEND_TYPE = 2;

	if (json->FIND_STATUS == 100 || json->FIND_STATUS == temp.CREATIVE_STATE || json->FIND_STATUS == 6) {

	    ad_system_mongo_stat_t *mongo_node = NULL;
	    if ((mongo_node = ad_system_mongo_stat_list_find(mongo_head, temp.CREATIVE_ID))) {

		double ctr = 0;
		if (mongo_node->click && mongo_node->impression)
		    ctr = (double)mongo_node->click / (double)mongo_node->impression;
		double cpc = 0;
		if (mongo_node->cost && mongo_node->click)
		    cpc = mongo_node->cost / (double)mongo_node->click;
		double cpm = 0;
		if (mongo_node->cost && mongo_node->impression)
		    cpm = mongo_node->cost / (double)mongo_node->impression * (double)1000;

		temp.CREATIVE_IMPRESSION = mongo_node->impression;
		temp.CREATIVE_CLICK = mongo_node->click;
		temp.CREATIVE_COST = mongo_node->cost;
		temp.CREATIVE_CTR = ctr;
		temp.CREATIVE_CPC = cpc;
		temp.CREATIVE_CPM = cpm;
	    }

	    ad_system_merger_creative_text_t *node = NULL;
	    if ((node = malloc(sizeof(ad_system_merger_creative_text_t)))) {

		memcpy(node, &temp, sizeof(ad_system_merger_creative_text_t));
		lg_list_add(merger_head, &node->list);

		if (node->CREATIVE_STATE == 1)
		    info->CREATIVE_ENABLED++;

		info->CREATIVE_COUNT++;
		info->CREATIVE_IMPRESSION += node->CREATIVE_IMPRESSION;
		info->CREATIVE_CLICK += node->CREATIVE_CLICK;
		info->CREATIVE_COST += node->CREATIVE_COST;
	    }
	}
    }

    if (info->CREATIVE_CLICK && info->CREATIVE_IMPRESSION)
	info->CREATIVE_CTR = (double)info->CREATIVE_CLICK / (double)info->CREATIVE_IMPRESSION;
    if (info->CREATIVE_COST && info->CREATIVE_CLICK)
	info->CREATIVE_CPC = (double)info->CREATIVE_COST / (double)info->CREATIVE_CLICK;
    if (info->CREATIVE_COST && info->CREATIVE_IMPRESSION)
	info->CREATIVE_CPM = (double)info->CREATIVE_COST / (double)info->CREATIVE_IMPRESSION * 1000;

    int ad_system_merger_cmp_code = 3000 + json->SORT_TYPE * 100 + json->SORT_COLUMN;
    lg_list_sort(&ad_system_merger_cmp_code, merger_head, 4096, ad_system_merger_creative_text_cmp);
    return 0;

}

int ad_system_merger_creative_img(
	ad_system_sql_t *sql_conn, 
	lg_list_t *mongo_head, 
	ad_system_packet_json_data_t *json,
	lg_list_t *merger_head,
	ad_system_merger_creative_info_t *info)
{

    lg_list_init(merger_head);
    memset(info, 0, sizeof(ad_system_merger_creative_info_t));

    ad_system_merger_creative_img_t temp;
    while(ad_system_sql_fetch_next(sql_conn)) {

	memset(&temp, 0, sizeof(ad_system_merger_creative_img_t));
	sprintf(temp.CAMPAIGN_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 1));
	sprintf(temp.CAMPAIGN_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 2));
	sprintf(temp.GROUP_ID, "%s",  ad_system_sql_get_string(sql_conn, NULL, 3));
	sprintf(temp.GROUP_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 4));
	sprintf(temp.CREATIVE_ID, "%s", ad_system_sql_get_string(sql_conn, NULL, 5));
	sprintf(temp.CREATIVE_NAME, "%s", ad_system_sql_get_string(sql_conn, NULL, 6));
	int CREATIVE_PAUSE = ad_system_sql_get_number(sql_conn, NULL, 7);
	temp.CREATIVE_STATE = ad_system_sql_get_number(sql_conn, NULL, 8);
	sprintf(temp.CREATIVE_REFUSE_REASON, "%s", ad_system_sql_get_string(sql_conn, NULL, 9));
	sprintf(temp.CREATIVE_IMG_SIZE, "%s",  ad_system_sql_get_string(sql_conn, NULL, 10));
	sprintf(temp.CREATIVE_IMG_SIZE_IMGLENGTH, "%s", ad_system_sql_get_string(sql_conn, NULL, 11));
	sprintf(temp.CREATIVE_FILE_PATH  , "%s" ,ad_system_sql_get_string(sql_conn, NULL, 12));
	sprintf(temp.CREATIVE_DESTINATION_URL ,"%s", ad_system_sql_get_string(sql_conn, NULL, 13));
	sprintf(temp.CREATIVE_DISPLAY_URL      ,"%s", ad_system_sql_get_string(sql_conn, NULL, 14));

	temp.CREATIVE_DSP_STATE = ad_system_sql_get_number(sql_conn, NULL, 15);
	sprintf(temp.CREATIVE_DSP_REFUSE_REASON,"%s", ad_system_sql_get_string(sql_conn, NULL, 16));

	if (CREATIVE_PAUSE == 2)
	    temp.CREATIVE_STATE = 3;

	//CREATIVE_STATE: 1有效 2不宜推广 3暂停 4审核中 5审核拒绝
	//if (temp.CREATIVE_STATE == 2 || temp.CREATIVE_STATE == 4)
	//temp.CREATIVE_DSP_STATE = 1;

	if (json->FIND_STATUS == 100 || json->FIND_STATUS == temp.CREATIVE_STATE || json->FIND_STATUS == 6 || json->FIND_STATUS == 10 || json->FIND_STATUS == 12) {

	    ad_system_mongo_stat_t *mongo_node = NULL;
	    if ((mongo_node = ad_system_mongo_stat_list_find(mongo_head, temp.CREATIVE_ID))) {

		double ctr = 0;
		if (mongo_node->click && mongo_node->impression)
		    ctr = (double)mongo_node->click / (double)mongo_node->impression;
		double cpc = 0;
		if (mongo_node->cost && mongo_node->click)
		    cpc = mongo_node->cost / (double)mongo_node->click;
		double cpm = 0;
		if (mongo_node->cost && mongo_node->impression)
		    cpm = mongo_node->cost / (double)mongo_node->impression * (double)1000;

		temp.CREATIVE_IMPRESSION = mongo_node->impression;
		temp.CREATIVE_CLICK = mongo_node->click;
		temp.CREATIVE_COST = mongo_node->cost;
		temp.CREATIVE_CTR = ctr;
		temp.CREATIVE_CPC = cpc;
		temp.CREATIVE_CPM = cpm;
	    }

	    ad_system_merger_creative_img_t *node = NULL;
	    if ((node = malloc(sizeof(ad_system_merger_creative_img_t)))) {

		memcpy(node, &temp, sizeof(ad_system_merger_creative_img_t));
		lg_list_add(merger_head, &node->list);

		if (node->CREATIVE_STATE == 1)
		    info->CREATIVE_ENABLED++;

		info->CREATIVE_COUNT++;
		info->CREATIVE_IMPRESSION += node->CREATIVE_IMPRESSION;
		info->CREATIVE_CLICK += node->CREATIVE_CLICK;
		info->CREATIVE_COST += node->CREATIVE_COST;
	    }
	}
    }

    if (info->CREATIVE_CLICK && info->CREATIVE_IMPRESSION)
	info->CREATIVE_CTR = (double)info->CREATIVE_CLICK / (double)info->CREATIVE_IMPRESSION;
    if (info->CREATIVE_COST && info->CREATIVE_CLICK)
	info->CREATIVE_CPC = (double)info->CREATIVE_COST / (double)info->CREATIVE_CLICK;
    if (info->CREATIVE_COST && info->CREATIVE_IMPRESSION)
	info->CREATIVE_CPM = (double)info->CREATIVE_COST / (double)info->CREATIVE_IMPRESSION * 1000;

    int ad_system_merger_cmp_code = 4000 + json->SORT_TYPE * 100 + json->SORT_COLUMN;
    lg_list_sort(&ad_system_merger_cmp_code, merger_head, 4096, ad_system_merger_creative_img_cmp);
    return 0;
}

void ad_system_merger_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    lg_list_t *node = NULL;

    while(p != head) {

	node = p;
	p = p->next;
	free(node);
    }

    lg_list_init(head);
}


