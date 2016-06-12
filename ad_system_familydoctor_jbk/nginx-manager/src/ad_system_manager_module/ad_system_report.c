
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <uuid/uuid.h>

#include "ad_system_mongo.h"
#include "ad_system_redis.h"
#include "ad_system_report.h"

// 计算实际消费
extern char *strptime(const char *s, const char *format, struct tm *tm);
double ad_system_report_cost_final(const char *date, double cost)
{
    if (!cost) return 0;

    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    strptime(date, "%Y-%m-%d", &tm);
    time_t t = mktime(&tm) + 28800;

    double cost_final = 0;
    if (t <= 1412870400)
	cost_final = 2;
    else if (t >= 1412870400 && t <= 1444406400)
	cost_final = 1.85;
    else
	cost_final = 1.7;

    return cost / cost_final;
}

// 计算咋扣消费
double ad_system_report_cost_discount(ad_system_mongo_t *conn,
	const char *date, //用于计算本时间之前的已消费总额
	int account_id,	  //需要计算的账户ID
	double cost_now)  //计算这次虚拟费用的实际消费
{

    lg_list_t head;
    if (ad_system_redis_recharge_get_list(account_id, &head)) //载入充值记录(redis)
	return -1;

    double cost_total = ad_system_mongo_get_account_cost_total(conn, date, account_id); //已消费总额(mongo聚合)
    if (cost_total == -1)
	return -1;

    double cost_recharge = 0;  //累计充值金额
    double cost_Virtual = 0;   //累计虚拟金额
    double cost_final = 0;     //累计实际金额

    lg_list_t *p = NULL;
    lg_list_for_each(p, &head) { //遍历充值记录

	ad_system_redis_recharge_t *node = (ad_system_redis_recharge_t *)p;

	if (!cost_Virtual) {

	    cost_recharge += node->money;      //充值金额累计
	    if (cost_total < cost_recharge) {  //已消费总额 < 当前充值累计(开始计算实际费用)

		// 充值累计 - 已消费总额 - 当前消费额度 >=0 本条充值额度已满足(返回本次折扣)
		if ((cost_recharge - cost_total - cost_now) >= 0) {

		    if (cost_now && node->multiple)
			return cost_now / node->multiple;
		    return cost_now;

		} else {

		    // 充值累计 - 已消费总额 - 当前消费额度 < 0(累计本次虚拟消费与实际消费)
		    cost_Virtual += cost_recharge - cost_total;
		    if (cost_Virtual && node->multiple)
			cost_final = cost_Virtual / node->multiple;
		    else
			cost_final = cost_Virtual;
		}
	    }

	} else {

	    // 累计虚拟消费 + 本次充值金额 >= 本次消费 (本次充值累计满足本次消费)
	    if ((cost_Virtual + node->money) >= cost_now) {

		if (cost_now && node->multiple)
		    cost_final += (cost_now - cost_Virtual) / node->multiple;
		else
		    cost_final += (cost_now - cost_Virtual);
		return cost_final;
	    }

	    // 累计本次虚拟消费与实际消费
	    cost_Virtual += node->money;
	    if (node->multiple)
		cost_final += node->money / node->multiple;
	    else
		cost_final += node->money;
	}
    }

    return cost_final;
}

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

int ad_system_report_account_add(lg_list_t *head, 
	const char *date, 
	int account_id, 
	int impression, 
	int click, 
	double cost, 
	int mode)
{

    if (!date || !account_id)
	return -1;

    lg_list_t *p = NULL;
    ad_system_report_account_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_account_t *)p;

	if (mode == 1 && strcmp(node->date, date) == 0 && node->account_id == account_id) {

	    node->impression += impression;
	    node->click += click;
	    node->cost += cost;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;

	    node->cost_final += ad_system_report_cost_final(date, cost);
	    return 0;

	} else if (mode == 2 && node->account_id == account_id) {

	    node->impression += impression;
	    node->click += click;
	    node->cost += cost;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;

	    node->cost_final += ad_system_report_cost_final(date, cost);
	    return 0;
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_account_t))))
	return -1;

    memset(node, 0, sizeof(ad_system_report_account_t));

    strcpy(node->date, date);
    node->account_id = account_id;
    ad_system_redis_get_account_name(account_id, node->account_name);
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;

    node->cost_final = ad_system_report_cost_final(date, cost);
    lg_list_add(head, &node->list);
    return 0;
}

int ad_system_report_media_add(lg_list_t *head, 
	const char *date, 
	const char *domain, 
	int account_id,
	int impression,
	int click,
	double cost)
{

    lg_list_t *p = NULL;
    ad_system_report_media_t *node = NULL;
    lg_list_for_each(p, head) {

	node = (ad_system_report_media_t *)p;
	if (strcmp(node->date, date) == 0 && strcmp(node->domain, domain) == 0) {

	    node->impression += impression;
	    node->click += click;
	    node->cost += cost;
	    if (node->click && node->impression)
		node->ctr = (double)node->click / (double)node->impression;
	    if (node->cost && node->click)
		node->cpc = node->cost / (double)node->click;
	    if (node->cost && node->impression)
		node->cpm = node->cost / (double)node->impression * (double)1000;

	    node->cost_final += ad_system_report_cost_final(date, cost);
	    return 0;
	}
    }

    if (!(node = malloc(sizeof(ad_system_report_media_t))))
	return -1;

    memset(node, 0, sizeof(ad_system_report_media_t));

    strcpy(node->date, date);
    strcpy(node->domain, domain);
    node->account_id = account_id;
    node->impression = impression;
    node->click = click;
    node->cost = cost;
    if (node->click && node->impression)
	node->ctr = (double)node->click / (double)node->impression;
    if (node->cost && node->click)
	node->cpc = node->cost / (double)node->click;
    if (node->cost && node->impression)
	node->cpm = node->cost / (double)node->impression * (double)1000;

    node->cost_final = ad_system_report_cost_final(date, cost);

    lg_list_add(head, &node->list);
    return 0;
}

void ad_system_report_free(lg_list_t *head)
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

void *ad_system_report_media_create(void *arg)
{

    pthread_detach(pthread_self());
    ad_system_report_thread_t *p = arg;

    char *visit_domain = NULL;
    if (p->media_type == 1)
	visit_domain = p->domain_str;

    lg_list_t head;
    if (ad_system_mongo_report_media_load(&head,
		visit_domain,
		p->time_s,
		p->time_e)) {

	ad_system_redis_add_report(p->report_id, "null");
	ad_system_report_free(&head);
	free(p);
	return NULL;
    }

    char path[1024] = {0};
    sprintf(path, "%s%s/report.csv", REPORT_FILE_PATH, p->report_path);
    if (ad_system_report_csv_media(&head, path)) {

	ad_system_redis_add_report(p->report_id, "null");
	ad_system_report_free(&head);
	free(p);
	return NULL;
    }

    sprintf(path, "%s/report.csv", p->report_path);
    ad_system_redis_add_report(p->report_id, path);
    ad_system_report_free(&head);
    free(p);
    return NULL;
}

void *ad_system_report_account_create(void *arg)
{

    pthread_detach(pthread_self());
    ad_system_report_thread_t *p = arg;

    char *visit_domain = NULL;
    if (p->media_type == 1)
	visit_domain = p->domain_str;

    lg_list_t head;
    if (ad_system_mongo_report_account_load(&head,
		visit_domain,
		p->time_s,
		p->time_e,
		p->date_type)) {

	ad_system_redis_add_report(p->report_id, "null");
	ad_system_report_free(&head);
	free(p);
	return NULL;
    }

    char path[1024];
    sprintf(path, "%s%s/report.csv", REPORT_FILE_PATH, p->report_path);

    if (ad_system_report_csv_account(&head, path, p->date_type, p->time_s) != 0) {

	ad_system_redis_add_report(p->report_id, "null");
	ad_system_report_free(&head);
	free(p);
	return NULL;
    }

    sprintf(path, "%s/report.csv", p->report_path);
    ad_system_redis_add_report(p->report_id, path);
    ad_system_report_free(&head);
    free(p);
    return NULL;
}


