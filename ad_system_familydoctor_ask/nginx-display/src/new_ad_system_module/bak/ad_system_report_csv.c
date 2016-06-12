
#include <fcntl.h>
#include <libtar.h>

#include "lg_string.h"
#include "ad_system_sql.h"
#include "ad_system_report.h"

int ad_system_report_csv_comperssion(char *report_id, char *report_path)
{

    char cpc_path[256];
    sprintf(cpc_path, "%s%s/cpc.csv", REPORT_FILE_PATH, report_path);

    char cpm_path[256];
    sprintf(cpm_path, "%s%s/cpm.csv", REPORT_FILE_PATH, report_path);

    char file_path[256];
    sprintf(file_path, "%s%s/report.zip", REPORT_FILE_PATH, report_path);

    char cpc_name[128];
    sprintf(cpc_name, "CPC_%s.csv", report_id);

    char cpm_name[128];
    sprintf(cpm_name, "CPM_%s.csv", report_id);

    TAR *tar_handle = NULL;
    tar_open(&tar_handle, file_path, NULL,  O_WRONLY | O_CREAT,  0644,  TAR_GNU);
    tar_append_file(tar_handle, cpc_path, cpc_name);
    tar_append_file(tar_handle, cpm_path, cpm_name);
    tar_close(tar_handle);
    return 0;
}

/*******************/
/* standard report */
/*******************/

int ad_system_report_standard_title(FILE *fp, int report_type)
{
    size_t size = 0;
    char where[1024] = {0};
    if (report_type > 1)
	size += sprintf(&where[size], "推广计划ID\t推广计划\t");
    if (report_type > 2)
	size += sprintf(&where[size], "推广组ID\t推广组\t");
    if (report_type > 3)
	size += sprintf(&where[size], "创意ID\t创意\t");

    char buffer[4096];
    size = sprintf(buffer, "%s\t%s%s\t%s\t%s\t%s\t%s\t%s\r\n",
	    "时间",
	    where,
	    "展现次数",
	    "点击次数",
	    "总消费",
	    "点击率",
	    "平均点击价格",
	    "千次展现成本");

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_standard_csv_write(FILE *fp, int report_type,
	char *date,
	int campaign_id,
	char *campaign_name,
	int group_id,
	char *group_name,
	int creative_id,
	char *creative_name,
	int impression,
	int click,
	double cost,
	double ctr,
	double cpc,
	double cpm)
{

    size_t size = 0;
    char where[1024] = {0};
    if (report_type > 1)
	size += sprintf(&where[size], "%u\t%s\t", campaign_id, campaign_name);
    if (report_type > 2)
	size += sprintf(&where[size], "%u\t%s\t", group_id, group_name);
    if (report_type > 3)
	size += sprintf(&where[size], "%u\t%s\t", creative_id, creative_name);

    char buffer[4096];
    size = sprintf(buffer, "%s\t%s%u\t%u\t%f\t%f\t%f\t%f\r\n",
	    date,
	    where,
	    impression,
	    click,
	    cost,
	    ctr,
	    cpc,
	    cpm);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

void ad_system_report_standard_parse_creative(FILE *fp, int report_type,
	char *date, 
	int campaign_id, 
	char *campaign_name, 
	int group_id, 
	char *group_name, 
	lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_creative_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_standard_creative_t *)p;
	ad_system_report_standard_csv_write(fp, report_type,
		date,
		campaign_id,
		campaign_name,
		group_id,
		group_name,
		node->creative_id,
		node->creative_name,
		node->impression,
		node->click,
		node->cost,
		node->ctr,
		node->cpc,
		node->cpm);
    }
}

void ad_system_report_standard_parse_group(FILE *fp, int report_type,
	char *date,
	int campaign_id,
	char *campaign_name,
	lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_group_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_standard_group_t *)p;
	if (report_type == 3) {

	    ad_system_report_standard_csv_write(fp, report_type,
		    date,
		    campaign_id,
		    campaign_name,
		    node->group_id,
		    node->group_name,
		    0,
		    NULL,
		    node->impression,
		    node->click,
		    node->cost,
		    node->ctr,
		    node->cpc,
		    node->cpm);

	} else {

	    ad_system_report_standard_parse_creative(fp, report_type,
		    date,
		    campaign_id,
		    campaign_name,
		    node->group_id,
		    node->group_name,
		    &node->head);
	}
    }
}

void ad_system_report_standard_parse_campaign(FILE *fp, int report_type,
	char *date, 
	lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_campaign_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_standard_campaign_t *)p;
	if (report_type == 2) {

	    ad_system_report_standard_csv_write(fp, report_type,
		    date,
		    node->campaign_id,
		    node->campaign_name,
		    0,
		    NULL,
		    0,
		    NULL,
		    node->impression,
		    node->click,
		    node->cost,
		    node->ctr,
		    node->cpc,
		    node->cpm);

	} else {

	    ad_system_report_standard_parse_group(fp, report_type,
		    date,
		    node->campaign_id,
		    node->campaign_name,
		    &node->head);
	}
    }
}

void ad_system_report_standard_parse(FILE *fp, int report_type, lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_standard_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_standard_t *)p;
	if (report_type == 1) {

	    ad_system_report_standard_csv_write(fp, report_type,
		    node->date,
		    0,
		    NULL,
		    0,
		    NULL,
		    0,
		    NULL,
		    node->impression,
		    node->click,
		    node->cost,
		    node->ctr,
		    node->cpc,
		    node->cpm);
	} else {

	    ad_system_report_standard_parse_campaign(fp, report_type,
		    node->date,
		    &node->head);
	}
    }
}

int ad_system_report_standard_csv(int report_type, char *report_path, int bidding_model, lg_list_t *head)
{

    char path[256];
    if (bidding_model == REPORT_CPC)
	sprintf(path, "%s%s/cpc.csv", REPORT_FILE_PATH, report_path);
    else if (bidding_model == REPORT_CPM)
	sprintf(path, "%s%s/cpm.csv", REPORT_FILE_PATH, report_path);
    else return -1;

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_standard_title(fp, report_type);
    ad_system_report_standard_parse(fp, report_type, head);

    fclose(fp);
    return 0;
}

/******************/
/* complex report */
/******************/

int ad_system_report_complex_title(FILE *fp, int report_type)
{

    size_t size = 0;
    char where[1024] = {0};
    if (report_type > 1)
	size += sprintf(&where[size], "%s\t", "城市");

    char buffer[4096];
    size = sprintf(buffer, "%s\t%s\t%s%s\t%s\t%s\t%s\t%s\t%s\r\n",
	    "时间",
	    "省份",
	    where,
	    "展现次数",
	    "点击次数",
	    "总消费",
	    "点击率",
	    "平均点击价格",
	    "千次展现成本");

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_complex_csv_write(FILE *fp, int report_type,
	char *date,
	char *region_father,
	char *region_child,
	int impression,
	int click,
	double cost,
	double ctr,
	double cpc,
	double cpm)
{

    size_t size = 0;
    char where[1024] = {0};
    if (report_type > 1)
	size += sprintf(&where[size], "%s\t", region_child);

    char buffer[4096];
    size = sprintf(buffer, "%s\t%s\t%s%u\t%u\t%f\t%f\t%f\t%f\r\n",
	    date,
	    region_father,
	    where,
	    impression,
	    click,		
	    cost,
	    ctr,
	    cpc,
	    cpm);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

void ad_system_report_complex_parse_child(FILE *fp, int report_type, char *date, char *region_father, lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_complex_child_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_complex_child_t *)p;
	ad_system_report_complex_csv_write(fp, report_type,
		date, 
		region_father,
		node->region_child,
		node->impression,
		node->click,
		node->cost,
		node->ctr,
		node->cpc,
		node->cpm);
    }
}

void ad_system_report_complex_parse_father(FILE *fp, int report_type, char *date, lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_complex_father_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_complex_father_t *)p;
	if (report_type == 1) {

	    ad_system_report_complex_csv_write(fp, report_type,
		    date,
		    node->region_father,
		    NULL,
		    node->impression,
		    node->click,
		    node->cost,
		    node->ctr,
		    node->cpc,
		    node->cpm);

	} else {

	    ad_system_report_complex_parse_child(fp, report_type, date, node->region_father, &node->head);
	}
    }
}

void ad_system_report_complex_parse(FILE *fp, int report_type, lg_list_t *head)
{

    lg_list_t *p = NULL;
    ad_system_report_complex_t *node = NULL;
    lg_list_for_each(p, head){

	node = (ad_system_report_complex_t *)p;
	ad_system_report_complex_parse_father(fp, report_type, node->date, &node->head);
    }
}

int ad_system_report_complex_csv(int report_type, char *report_path, int bidding_model, lg_list_t *head)
{

    char path[256];
    if (bidding_model == REPORT_CPC)
	sprintf(path, "%s%s/cpc.csv", REPORT_FILE_PATH, report_path);
    else if (bidding_model == REPORT_CPM)
	sprintf(path, "%s%s/cpm.csv", REPORT_FILE_PATH, report_path);
    else return -1;

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_complex_title(fp, report_type);
    ad_system_report_complex_parse(fp, report_type, head);

    fclose(fp);
    return 0;
}

int ad_system_report_csv_group_keyword_title(FILE *fp)
{ 
    char buffer[1024] = 
	"关键词ID\t"
	"关键词\t"
	"推广计划ID\t"
	"推广计划\t"
	"推广组ID\t"
	"推广组\t"
	"CPC出价\t"
	"CPM出价\r\n";
    size_t size = strlen(buffer);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_group_keyword_write(FILE *fp,
	const char *group_keyword_id,
	const char *group_keyword,
	const char *campaign_id,
	const char *campaign_name,
	const char *group_id,
	const char *group_name,
	double group_cpc_price,
	double group_cmp_price)
{
    char buffer[4096] = {0};
    size_t size = sprintf(buffer, "%s\t%s\t%s\t%s\t%s\t%s\t%f\t%f\r\n",
	    group_keyword_id,
	    group_keyword,
	    campaign_id,
	    campaign_name,
	    group_id,
	    group_name,
	    group_cpc_price,
	    group_cmp_price);

    char wbuffer[4096] = {0};
    if (lg_string_utf8_to_unicode(buffer, size, wbuffer, 4096))
	return -1;

    size = lg_string_unicode_len(wbuffer);
    return fwrite(wbuffer, 1, size, fp);
}

int ad_system_report_csv_group_keyword(ad_system_sql_t *sql_conn, char *path)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "w")))
	return -1;

    ad_system_report_csv_group_keyword_title(fp);

    while(ad_system_sql_fetch_next(sql_conn)) {

	ad_system_report_csv_group_keyword_write(fp,
		ad_system_sql_get_string(sql_conn, NULL, 1),
		ad_system_sql_get_string(sql_conn, NULL, 2),
		ad_system_sql_get_string(sql_conn, NULL, 3),
		ad_system_sql_get_string(sql_conn, NULL, 4),
		ad_system_sql_get_string(sql_conn, NULL, 5),
		ad_system_sql_get_string(sql_conn, NULL, 6),
		ad_system_sql_get_double(sql_conn, NULL, 7),
		ad_system_sql_get_double(sql_conn, NULL, 8));
    }

    fclose(fp);
    return 0;
}


