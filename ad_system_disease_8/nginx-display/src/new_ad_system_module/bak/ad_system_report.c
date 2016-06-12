
#include <pthread.h>
#include <uuid/uuid.h>

#include "ad_system_sql.h"
#include "ad_system_mongo.h"
#include "ad_system_redis.h"
#include "ad_system_region_code.h"
#include "ad_system_report.h"
#include "ad_system_report_csv.h"

int ad_system_report_path_create(char *key, char *path)
{

    int i, j;
    char dir_path[128] = {0};
    memset(path, 0, 128);

    uuid_t uuid;
    uuid_generate(uuid);

    for (i = 0, j = 0; i < 16; i++, j++, j++)
	sprintf(&key[j], "%02x", uuid[i]);

    for (i = 0, j = 0; i < 32; i++) {

	path[j++] = '/';
	path[j++] = key[i];

	sprintf(dir_path, "%s%s", REPORT_FILE_PATH, path);
	if (access(dir_path, 0))
	    if (mkdir(dir_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		return -1;
    }

    return 0;
}

int ad_system_report_path_create_r(char *file_path, char *down_path)
{

    char key[128] = {0}, _path[128] = {0};
    if (ad_system_report_path_create(key, _path))
	return -1;

    sprintf(file_path, "%s%s/report.csv", REPORT_FILE_PATH, _path);
    sprintf(down_path, "%s/report.csv", _path);
    return 0;
}

/*******************/
/* report standard */
/*******************/

int ad_system_report_standard_creative_add(lg_list_t *head, int display_style,
	int creative_id,
	int impression,
	int click, 
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_creative_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_standard_creative_t *)p;
	if (node->creative_id == creative_id) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;
	    return 0;
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_standard_creative_t))))
	return -1;

    memset(node, 0, sizeof(ad_system_report_standard_creative_t));

    node->creative_id = creative_id;
    ad_system_redis_get_creative_name(display_style, creative_id, node->creative_name);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;
    lg_list_add(head, &node->next);
    return 0;
}

int ad_system_report_standard_group_add(lg_list_t *head, int display_style,
	int group_id, 
	int creative_id, 
	int impression,
	int click, 
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_group_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_standard_group_t *)p;
	if (node->group_id == group_id) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    return ad_system_report_standard_creative_add(&node->head, display_style,
		    creative_id,
		    impression,
		    click,
		    cost);
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_standard_group_t))))
	return -1;

    memset(node, 0, sizeof(ad_system_report_standard_group_t));

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);
    node->group_id = group_id;
    ad_system_redis_get_group_name(group_id, node->group_name);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return ad_system_report_standard_creative_add(&node->head, display_style,
	    creative_id,
	    impression,
	    click,
	    cost);
}

int ad_system_report_standard_campaign_add(lg_list_t *head, int display_style,
	int campaign_id,
	int group_id,
	int creative_id,
	int impression,
	int click,
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_campaign_t*node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_standard_campaign_t *)p;
	if (node->campaign_id == campaign_id) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;
	    return ad_system_report_standard_group_add(&node->head, display_style,
		    group_id,
		    creative_id,
		    impression,
		    click,
		    cost);
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_standard_campaign_t))))
	return -1;

    memset(node, 0, sizeof(ad_system_report_standard_campaign_t));

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);
    node->campaign_id = campaign_id;
    ad_system_redis_get_campaign_name(campaign_id, node->campaign_name);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return ad_system_report_standard_group_add(&node->head, display_style,
	    group_id,
	    creative_id,
	    impression,
	    click,
	    cost);
}

int ad_system_report_standard_add(lg_list_t *head, int display_style,
	const char *date,
	int campaign_id,
	int group_id,
	int creative_id,
	int impression,
	int click,
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_standard_t *)p;
	if (strcmp(node->date, date) == 0) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    return ad_system_report_standard_campaign_add(&node->head, display_style,
		    campaign_id,
		    group_id,
		    creative_id,
		    impression,
		    click,
		    cost);
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_standard_t))))
	return -1;

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);
    strcpy(node->date, date);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return ad_system_report_standard_campaign_add(&node->head, display_style,
	    campaign_id,
	    group_id,
	    creative_id,
	    impression,
	    click,
	    cost);
}

void ad_system_report_standard_creative_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_standard_creative_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_standard_creative_t *)p;
	p = p->next;
	free(node);
    }
}

void ad_system_report_standard_group_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_standard_group_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_standard_group_t *)p;
	ad_system_report_standard_creative_free(&node->head);
	p = p->next;
	free(node);
    }
}

void ad_system_report_standard_campaign_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_standard_campaign_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_standard_campaign_t *)p;
	ad_system_report_standard_group_free(&node->head);
	p = p->next;
	free(node);
    }
}

void ad_system_report_standard_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_standard_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_standard_t *)p;
	ad_system_report_standard_campaign_free(&node->head);
	p = p->next;
	free(node);
    }
}

void *ad_system_report_standard_create(ad_system_report_arg_t *p)
{

    //cpc
    lg_list_t cpc_head;
    if (ad_system_mongo_report_standard_load(&cpc_head,
		p->display_style,
		REPORT_CPC,
		p->account_id,
		p->time_s, 
		p->time_e, 
		p->time_type)) {

	ad_system_mongo_report_list_set(p->account_id, p->report_key, 3);
	return NULL;
    }

    ad_system_report_standard_csv(p->report_type, p->report_path, REPORT_CPC, &cpc_head);
    ad_system_report_standard_free(&cpc_head);

    //cpm
    lg_list_t cpm_head;
    if (ad_system_mongo_report_standard_load(&cpm_head,
		p->display_style,
		REPORT_CPM,
		p->account_id,
		p->time_s, 
		p->time_e, 
		p->time_type)) {

	ad_system_mongo_report_list_set(p->account_id, p->report_key, 3);
	return NULL;
    }

    ad_system_report_standard_csv(p->report_type, p->report_path, REPORT_CPM, &cpm_head);
    ad_system_report_standard_free(&cpm_head);

    //压缩
    ad_system_report_csv_comperssion(p->report_key, p->report_path);
    ad_system_mongo_report_list_set(p->account_id, p->report_key, 2);
    return NULL;
}

/******************/
/* report complex */
/******************/

int ad_system_report_complex_child_add(lg_list_t *head,
	const char *region_child,
	int impression,
	int click,
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_complex_child_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_complex_child_t *)p;
	if (strcmp(node->region_child, region_child) == 0) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    return 0;
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_complex_child_t))))
	return -1;

    lg_list_add(head, &node->next);
    strcpy(node->region_child, region_child);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return 0;
}

int ad_system_report_complex_father_add(lg_list_t *head,
	const char *region_father,
	const char *region_child,
	int impression,
	int click,
	double cost)

{

    lg_list_t *p = NULL;
    ad_system_report_complex_father_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_complex_father_t *)p;
	if (strcmp(node->region_father, region_father) == 0) {

	    node->impression += impression;;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    return ad_system_report_complex_child_add(
		    &node->head,
		    region_child,
		    impression,
		    click,
		    cost);
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_complex_father_t))))
	return -1;

    lg_list_add(head, &node->next);
    lg_list_init(&node->head);
    strcpy(node->region_father, region_father);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return ad_system_report_complex_child_add(
	    &node->head,
	    region_child,
	    impression,
	    click,
	    cost);
}

int ad_system_report_complex_add(lg_list_t *head,
	const char *date,
	int region_code,
	int impression,
	int click,
	double cost)
{

    ad_system_region_code_t *region = ad_system_region_code_find(region_code);

    lg_list_t *p = NULL;
    ad_system_report_complex_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_complex_t *)p;
	if (strcmp(node->date, date) == 0) {

	    node->impression += impression;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    return ad_system_report_complex_father_add(
		    &node->head,
		    region?region->father:"其他",
		    region?region->child:"其他",
		    impression,
		    click,
		    cost);
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_complex_t))))
	return -1;

    lg_list_init(&node->head);
    lg_list_add(head, &node->next);
    strcpy(node->date, date);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    node->ctr = 0;
    node->cpc = 0;
    node->cpm = 0;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    return ad_system_report_complex_father_add(
	    &node->head,
	    region?region->father:"其他",
	    region?region->child:"其他",
	    impression,
	    click,
	    cost);
}

void ad_system_report_complex_child_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_complex_child_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_complex_child_t *)p;
	p = p->next;
	free(node);
    }
}

void ad_system_report_complex_father_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_complex_father_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_complex_father_t *)p;
	ad_system_report_complex_child_free(&node->head);
	p = p->next;
	free(node);
    }
}

void ad_system_report_complex_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    ad_system_report_complex_t *node = NULL;

    while(p != head) {

	node = (ad_system_report_complex_t *)p;
	ad_system_report_complex_father_free(&node->head);
	p = p->next;
	free(node);
    }
}

void *ad_system_report_complex_create(ad_system_report_arg_t *p)
{

    lg_list_t cpc_head;
    if (ad_system_mongo_report_region_load(&cpc_head,
		p->display_style,//display_style,
		REPORT_CPC,
		p->account_id,//account_id, 
		p->time_s,
		p->time_e,
		p->time_type)) {

	ad_system_mongo_report_list_set(p->account_id, p->report_key, 3);
	return NULL;
    }

    ad_system_report_complex_csv(p->report_type, p->report_path, REPORT_CPC, &cpc_head);
    ad_system_report_complex_free(&cpc_head);

    lg_list_t cpm_head;
    if (ad_system_mongo_report_region_load(&cpm_head,
		p->display_style,//display_style,
		REPORT_CPM,
		p->account_id,//account_id, 
		p->time_s,
		p->time_e,
		p->time_type)) {

	ad_system_mongo_report_list_set(p->account_id, p->report_key, 3);
	return NULL;
    }

    ad_system_report_complex_csv(p->report_type, p->report_path, REPORT_CPM, &cpm_head);
    ad_system_report_complex_free(&cpm_head);

    ad_system_report_csv_comperssion(p->report_key, p->report_path);
    ad_system_mongo_report_list_set(p->account_id, p->report_key, 2);
    return NULL;
}

