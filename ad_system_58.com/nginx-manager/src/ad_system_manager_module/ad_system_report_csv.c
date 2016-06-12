
#include <stdio.h>
#include <string.h>
#include "lg_string.h"
#include "ad_system_redis.h"
#include "ad_system_mongo.h"
#include "ad_system_report.h"

int ad_system_report_csv_account_title(FILE *fp, int mode)
{

    size_t size = 0;
    char buffer[2048] = {0};
    if (mode == 1) size = sprintf(buffer,
	    "日期\t"
	    "用户ID\t"
	    "用户名称\t"
	    "展现量\t"
	    "点击量\t"
	    "虚拟款金额(￥)\t"
	    //"实际款消费金额(￥)\t"
	    "平均点击价格(￥)\t"
	    "实际消费(￥)\r\n");
    else size = sprintf(buffer,
	    "用户ID\t"
	    "用户名称\t"
	    "展现量\t"
	    "点击量\t"
	    "虚拟款金额(￥)\t"
	    "平均点击价格(￥)\t"
	    "实际消费(￥)\r\n");

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_account_write(FILE *fp, int mode, const char *time_s, ad_system_mongo_t *conn,
	char *date,
	int account_id,
	char *account_name,
	int impression,
	int click,
	double cost,
	double cost_final,
	//double cost_discount,
	double cpc)
{

    size_t size = 0;
    char buffer[4096] = {0};
    char wbuffer[4096] = {0};

    if (mode == 1) {

	size = sprintf(buffer, "%s\t%d\t%s\t%u\t%u\t%f\t%f\t%f\r\n",
		date,
		account_id,
		account_name,
		impression,
		click,
		cost,
		//cost_discount,
		cpc,
		cost_final);

    } else {

	size = sprintf(buffer, "%d\t%s\t%u\t%u\t%f\t%f\t%f\r\n",
		account_id,
		account_name,
		impression,
		click,
		cost,
		//cost_discount,
		cpc,
		cost_final);
    }

    //fprintf(stderr, buffer);
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_account(lg_list_t *head, char *path, int mode, const char *time_s)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_csv_account_title(fp, mode);

    lg_list_t *p = NULL;
    lg_list_for_each(p, head){

	ad_system_report_account_t *node = NULL;
	node = (ad_system_report_account_t *)p;
	ad_system_report_csv_account_write(fp, mode, time_s, NULL,
		node->date,
		node->account_id,
		node->account_name,
		node->impression,
		node->click,
		node->cost,
		node->cost_final,
		//node->cost_discount,
		node->cpc);
    }

    fclose(fp);
    return 0;
}

int ad_system_report_csv_media_title(FILE *fp)
{
    char buffer[] = 
	"日期\t"
	"媒体\t"
	"展现量\t"
	"点击量\t"
	"虚拟款金额(￥)\t"
	//"实际款消费金额(￥)\t"
	"点击率\t"
	"平均点击价格(￥)\t"
	"千次展现成本(￥)\t"
	"实际消费(￥)\r\n";
    size_t size = strlen(buffer);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_media_write(FILE *fp,
	char *date,
	char *domain,
	int impression,
	int click,
	double cost,
	double ctr,
	double cpc,
	double cpm,
	double cost_final)
{

    //fprintf(stderr, "[2][%f][%f]\n", cost, cost_final);

    char buffer[4096] = {0};
    size_t size = sprintf(buffer, "%s\t%s\t%u\t%u\t%f\t%0.2f%%\t%f\t%f\t%f\r\n",
	    date,
	    domain,
	    impression,
	    click,
	    cost,
	    //cost_discount,
	    ctr * 100,
	    cpc,
	    cpm,
	    cost_final);

    //fprintf(stderr, buffer);
    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_media(lg_list_t *head, char *path)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_csv_media_title(fp);

    lg_list_t *p = NULL;
    lg_list_for_each(p, head){

	ad_system_report_media_t *node = NULL;
	node = (ad_system_report_media_t *)p;
	//fprintf(stderr, "[1][%f][%f]\n", node->cost, node->cost_final);
	ad_system_report_csv_media_write(fp,
		node->date,
		node->domain,
		node->impression,
		node->click,
		node->cost,
		node->ctr,
		node->cpc,
		node->cpm,
		node->cost_final);
    }

    fclose(fp);
    return 0;
}

int ad_system_report_csv_money_operation_title(FILE *fp)
{
    char buffer[] = {
	"用户名称\t"
	    "公司名称\t"
	    "开户时间\t"
	    "投资总额\t"
	    "账面余额\r\n"};
    size_t size = strlen(buffer);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_money_operation_write(FILE *fp,
	const char *login_name,
	const char *company_name,
	const char *create_time,
	const char *invest,
	const char *balence)
{

    char buffer[4096] = {0};
    size_t size = sprintf(buffer, "%s\t%s\t%s\t%s\t%s\r\n",
	    login_name,
	    company_name,
	    create_time,
	    balence,
	    invest);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_money_operation(ad_system_sql_t *sql_conn, char *path)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_csv_money_operation_title(fp);

    while(ad_system_sql_fetch_next(sql_conn)) {

	ad_system_report_csv_money_operation_write(fp, 
		ad_system_sql_get_string(sql_conn, NULL, 2),
		ad_system_sql_get_string(sql_conn, NULL, 3),
		ad_system_sql_get_string(sql_conn, NULL, 7),
		ad_system_sql_get_string(sql_conn, NULL, 5),
		ad_system_sql_get_string(sql_conn, NULL, 6));
    }

    fclose(fp);
    return 0;
}

int ad_system_report_csv_money_list_title(FILE *fp)
{

    char *buffer = 
	"登录名\t"
	"公司名称\t"
	"流水号\t"
	"充值金额\t"
	"投资总额\t"
	"账面余额\t"
	"充值时间\r\n";
    size_t size = strlen(buffer);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_money_list_write(FILE *fp,
	const char *account_name,
	const char *company_name,
	const char *swift_number,
	const char *money,
	const char *invest,
	const char *current_money_balance,
	const char *create_time)
{
    char buffer[4096] = {0};
    size_t size = sprintf(buffer, "%s\t%s\t%s\t%s\t%s\t%s\t%s\r\n",
	    account_name,
	    company_name,
	    swift_number,
	    money,
	    invest,
	    current_money_balance,
	    create_time);


    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_money_list(ad_system_sql_t *sql_conn, char *path)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_csv_money_list_title(fp);

    while(ad_system_sql_fetch_next(sql_conn)) {

	ad_system_report_csv_money_list_write(fp, 
		ad_system_sql_get_string(sql_conn, NULL, 2),
		ad_system_sql_get_string(sql_conn, NULL, 3),
		ad_system_sql_get_string(sql_conn, NULL, 5),
		ad_system_sql_get_string(sql_conn, NULL, 6),
		ad_system_sql_get_string(sql_conn, NULL, 8),
		ad_system_sql_get_string(sql_conn, NULL, 7),
		ad_system_sql_get_string(sql_conn, NULL, 13));
    }

    fclose(fp);
    return 0;
}


