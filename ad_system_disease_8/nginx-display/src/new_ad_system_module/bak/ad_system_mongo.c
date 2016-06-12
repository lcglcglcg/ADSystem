
#include <stdio.h>
#include <stdlib.h>

#include <bson.h>
#include <mongoc.h>

#include "lg_list.h"
#include "lg_string.h"

#include "ad_system_mongo.h"
#include "ad_system_report.h"
#include "ad_system_region_code.h"

#define	IF_ITER_FIND_TIME(doc, iter, name) if (!bson_iter_init_find(iter, doc, name) || !BSON_ITER_HOLDS_DATE_TIME(iter))
#define	IF_ITER_FIND_STRING(doc, iter, name) if (!bson_iter_init_find(iter, doc, name) || !BSON_ITER_HOLDS_UTF8(iter))

static char MONGODB_HOST[128] = {0};
int ad_system_mongo_init(lg_list_t *config_file_head)
{

    char *mongodb_host_str = NULL;
    if (!(mongodb_host_str = lg_config_file_find(config_file_head, "mongodb_host")))
	return -1;

    char *mongodb_port_str = NULL;
    if (!(mongodb_port_str = lg_config_file_find(config_file_head, "mongodb_port")))
	return -1;

    fprintf(stdout, "MONGODB_HOST=%s\n", mongodb_host_str);
    fprintf(stdout, "MONGODB_PORT=%s\n", mongodb_port_str);
    sprintf(MONGODB_HOST, "mongodb://%s:%s", mongodb_host_str, mongodb_port_str);
    return 0;
}

int ad_system_mongo_conn(ad_system_mongo_t *p)
{
    memset(p, 0, sizeof(ad_system_mongo_t));
    if (!(p->client = mongoc_client_new (MONGODB_HOST)))
	return -1;

    return 0;
}

int ad_system_mongo_insert(ad_system_mongo_t *p, const char *table, bson_t *query)
{

    if (!(p->collection = mongoc_client_get_collection (p->client, MONGO_DB_NAME, table)))
	return -1;

    return mongoc_collection_insert (p->collection, MONGOC_INSERT_NONE, query, NULL, &p->error);
}

int ad_system_mongo_update(ad_system_mongo_t *p, const char *table, bson_t *query, bson_t *u)
{

    if (!(p->collection = mongoc_client_get_collection (p->client, MONGO_DB_NAME, table)))
	return -1;

    return mongoc_collection_update(p->collection, MONGOC_INSERT_NONE, query, u, NULL, &p->error);
}

int ad_system_mongo_delete(ad_system_mongo_t *p, const char *table, bson_t *query)
{

    if (!(p->collection = mongoc_client_get_collection (p->client, MONGO_DB_NAME, table)))
	return -1;

    mongoc_collection_delete(p->collection, MONGOC_INSERT_NONE, query, NULL, &p->error);
    return 0;
}

int ad_system_mongo_find(ad_system_mongo_t *p, const char *table, bson_t *query)
{

    if (!(p->collection = mongoc_client_get_collection (p->client, MONGO_DB_NAME, table)))
	return -1;

    if (!(p->cursor = mongoc_collection_find (
		    p->collection,
		    MONGOC_QUERY_NONE,
		    0,
		    0,
		    0,
		    query,
		    NULL,  /* Fields, NULL for all. */
		    NULL))) /* Read Prefs, NULL for default */
	return -1;

    if (mongoc_cursor_error (p->cursor, &p->error))
	return -1;

    return 0;
}

void ad_system_mongo_free(ad_system_mongo_t *p)
{

    if (p->cursor)
	mongoc_cursor_destroy (p->cursor);
    mongoc_collection_destroy (p->collection);
    mongoc_client_destroy (p->client);
}

void ad_system_mongo_list_free(lg_list_t *head)
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

char *ad_system_mongo_time_to_string_week(time_t date_time, char *buffer)
{

    struct tm date_tm;
    gmtime_r(&date_time, &date_tm);

    time_t date_time_s = 0;
    if (date_tm.tm_wday == 0)
	date_time_s = date_time - 86400 * 6;
    else
	date_time_s = date_time - 86400 * (date_tm.tm_wday - 1);

    time_t date_time_e = date_time_s + 86400 * 6;

    struct tm date_tm_s;
    struct tm date_tm_e;
    gmtime_r(&date_time_s, &date_tm_s);
    gmtime_r(&date_time_e, &date_tm_e);

    sprintf(buffer, "%04d-%02d-%02d至%04d-%02d-%02d",
	    date_tm_s.tm_year + 1900,
	    date_tm_s.tm_mon + 1,
	    date_tm_s.tm_mday,
	    date_tm_e.tm_year + 1900,
	    date_tm_e.tm_mon + 1,
	    date_tm_e.tm_mday);

    return buffer;
}

char *ad_system_mongo_time_to_string(time_t date_time, char *buffer, int time_type)
{

    struct tm tm;
    gmtime_r(&date_time, &tm);

    switch(time_type) {

	case 1://日
	    sprintf(buffer, "%04d-%02d-%02d",
		    tm.tm_year + 1900,
		    tm.tm_mon + 1,
		    tm.tm_mday);
	    return buffer;

	case 2://周
	    return ad_system_mongo_time_to_string_week(date_time, buffer);

	case 3://月
	    sprintf(buffer, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);
	    return buffer;

	case 4://年
	    sprintf(buffer, "%04d", tm.tm_year + 1900);
	    return buffer;

	default:
	    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
		    tm.tm_year + 1900,
		    tm.tm_mon + 1,
		    tm.tm_mday,
		    tm.tm_hour,
		    tm.tm_min,
		    tm.tm_sec);
	    return buffer;
    }

    return NULL;
}

time_t ad_system_mongo_string_to_time(const char *time_str)
{

    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(time_str, "%Y-%m-%d", &tm);
    return mktime(&tm) + 28800;
}

int ad_system_mongo_to_number(bson_iter_t *iter)
{

    int n = 0;
    int type = (bson_iter_type(iter));
    switch(type){

	case BSON_TYPE_DOUBLE:
	    n = bson_iter_double(iter);
	    break;

	case BSON_TYPE_INT32:
	    n = bson_iter_int32(iter);
	    break;

	case BSON_TYPE_INT64:
	    n = bson_iter_int64(iter);
	    break;

	case BSON_TYPE_UTF8:
	    n = atoi(bson_iter_utf8(iter, NULL));
	    break;
    }   

    return n;
}

double ad_system_mongo_to_double(bson_iter_t *iter)
{

    double n = 0;
    int type = (bson_iter_type(iter));
    switch(type){

	case BSON_TYPE_DOUBLE:
	    n = bson_iter_double(iter);
	    break;

	case BSON_TYPE_INT32:
	    n = bson_iter_int32(iter);
	    break;

	case BSON_TYPE_INT64:
	    n = bson_iter_int64(iter);
	    break;

	case BSON_TYPE_UTF8:
	    n = atof(bson_iter_utf8(iter, NULL));
	    break;
    }   

    return n;
}

//标准报告
int ad_system_mongo_report_standard_element(lg_list_t *head, const bson_t *doc, int display_style, int time_type)
{

    bson_iter_t iter_campaign_id;
    if (!bson_iter_init_find(&iter_campaign_id, doc, "campaign_id"))
	return -1;

    bson_iter_t iter_group_id;
    if (!bson_iter_init_find(&iter_group_id, doc, "group_id"))
	return -1;

    bson_iter_t iter_creative_id;
    if (!bson_iter_init_find(&iter_creative_id, doc, "creative_id"))
	return -1;

    bson_iter_t iter_time;
    if (!bson_iter_init_find(&iter_time, doc, "time"))
	return -1;

    bson_iter_t iter_impression;
    if (!bson_iter_init_find(&iter_impression, doc, "impression"))
	return -1;

    bson_iter_t iter_click;
    if (!bson_iter_init_find(&iter_click, doc, "click"))
	return -1;

    bson_iter_t iter_cost;
    if (!bson_iter_init_find(&iter_cost, doc, "cost"))
	return -1;

    char time_buffer[128];
    return ad_system_report_standard_add(head, display_style,
	    ad_system_mongo_time_to_string(bson_iter_time_t(&iter_time), time_buffer, time_type),
	    ad_system_mongo_to_number(&iter_campaign_id),
	    ad_system_mongo_to_number(&iter_group_id),
	    ad_system_mongo_to_number(&iter_creative_id),
	    ad_system_mongo_to_number(&iter_impression),
	    ad_system_mongo_to_number(&iter_click),
	    ad_system_mongo_to_double(&iter_cost));
}

int ad_system_mongo_report_standard_load(lg_list_t *head, 
	int display_style, 
	int bidding_model, 
	const char *account_id, 
	const char *time_s, 
	const char *time_e,
	int time_type)
{

    lg_list_init(head);
    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query_time;
    bson_init (&query_time);
    bson_append_time_t(&query_time, "$gte", -1, ad_system_mongo_string_to_time(time_s));
    bson_append_time_t(&query_time, "$lte", -1, ad_system_mongo_string_to_time(time_e));

    bson_t query;
    bson_init (&query);
    bson_append_int32(&query, "account_id", -1, atoi(account_id));
    bson_append_int32(&query, "display_style", -1, display_style);
    bson_append_int32(&query, "bidding_model", -1, bidding_model);
    bson_append_document(&query, "time", -1, &query_time);

    //fprintf(stdout, "%s\n", bson_as_json(&query, NULL));

    if (ad_system_mongo_find(&conn, MONGO_REPORT_STANDARD, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    while (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    ad_system_mongo_report_standard_element(head, conn.doc, display_style, time_type);
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

//地域报告
int ad_system_mongo_report_region_element(lg_list_t *head, const bson_t *doc, int time_type)
{

    bson_iter_t iter_time;
    if (!bson_iter_init_find(&iter_time, doc, "time"))
	return -1;

    bson_iter_t iter_region_code;
    if (!bson_iter_init_find(&iter_region_code, doc, "region_code"))
	return -1;

    bson_iter_t iter_impression;
    if (!bson_iter_init_find(&iter_impression, doc, "impression"))
	return -1;

    bson_iter_t iter_click;
    if (!bson_iter_init_find(&iter_click, doc, "click"))
	return -1;

    bson_iter_t iter_cost;
    if (!bson_iter_init_find(&iter_cost, doc, "cost"))
	return -1;

    int impression = ad_system_mongo_to_number(&iter_impression);
    int click = ad_system_mongo_to_number(&iter_click);
    double cost = ad_system_mongo_to_double(&iter_cost);
    char time_buffer[128];
    return ad_system_report_complex_add(head,
	    ad_system_mongo_time_to_string(bson_iter_time_t(&iter_time), time_buffer, time_type),
	    ad_system_mongo_to_number(&iter_region_code),
	    impression,
	    click,
	    cost);
    return 0;
}

//地域报告
int ad_system_mongo_report_region_load(lg_list_t *head, 
	int display_style, int bidding_model, const char *account_id, const char *time_s, const char *time_e,int time_type)
{

    lg_list_init(head);
    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query_time;
    bson_init (&query_time);
    bson_append_time_t(&query_time, "$gte", -1, ad_system_mongo_string_to_time(time_s));
    bson_append_time_t(&query_time, "$lte", -1, ad_system_mongo_string_to_time(time_e));

    bson_t query;
    bson_init (&query);
    bson_append_int32(&query, "account_id", -1, atoi(account_id));
    bson_append_int32(&query, "display_style", -1, display_style);
    bson_append_int32(&query, "bidding_model", -1, bidding_model);
    bson_append_document(&query, "time", -1, &query_time);

    if (ad_system_mongo_find(&conn, MONGO_REPORT_REGION, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    while (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    ad_system_mongo_report_region_element(head, conn.doc, time_type);
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
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

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    char report_type_str[128];
    sprintf(report_type_str, "%u", report_type);

    char report_belong_str[128];
    sprintf(report_belong_str, "%u", report_belong);

    bson_t query;
    bson_init (&query);
    bson_append_utf8(&query, "account_id", -1, account_id, -1);
    bson_append_utf8(&query, "id", -1, report_id, -1);
    bson_append_utf8(&query, "name", -1, report_name, -1);
    bson_append_utf8(&query, "type", -1, report_type_str, -1);
    bson_append_utf8(&query, "belong", -1, report_belong_str, -1);
    bson_append_utf8(&query, "time_range", -1, time_range, -1);
    bson_append_time_t(&query, "create_time", -1, time(NULL) + 28800);
    bson_append_utf8(&query, "status", -1, "1", -1);
    bson_append_utf8(&query, "path", -1, path, -1);

    ad_system_mongo_insert(&conn, MONGO_REPORT_LIST, &query);

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

int ad_system_mongo_report_list_set(char *account_id, char *report_id, int status)
{

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    char status_str[128];
    sprintf(status_str, "%u", status);

    bson_t query;
    bson_init (&query);
    bson_append_utf8(&query, "account_id", -1, account_id, -1);
    bson_append_utf8(&query, "id", -1, report_id, -1);

    bson_t b_status;
    bson_init (&b_status);
    bson_append_utf8(&b_status, "status", -1, status_str, -1);

    bson_t b_update;
    bson_init (&b_update);
    bson_append_document(&b_update, "$set", -1, &b_status);

    ad_system_mongo_update(&conn, MONGO_REPORT_LIST, &query, &b_update);

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

int ad_system_mongo_report_list_element(const bson_t *doc, lg_list_t *head)
{

    bson_iter_t iter_id;
    IF_ITER_FIND_STRING(doc, &iter_id, "id")
	return -1;

    bson_iter_t iter_name;
    IF_ITER_FIND_STRING(doc, &iter_name, "name")
	return -1;

    bson_iter_t iter_type;
    IF_ITER_FIND_STRING(doc, &iter_type, "type")
	return -1;

    bson_iter_t iter_range_time;
    IF_ITER_FIND_STRING(doc, &iter_range_time, "time_range")
	return -1;

    bson_iter_t iter_create_time;
    IF_ITER_FIND_TIME(doc, &iter_create_time, "create_time")
	return -1;

    bson_iter_t iter_status;
    IF_ITER_FIND_STRING(doc, &iter_status, "status")
	return -1;

    bson_iter_t iter_path;
    IF_ITER_FIND_STRING(doc, &iter_path, "path")
	return -1;

    ad_system_mongo_report_list_t *node = NULL;
    if (!(node = (ad_system_mongo_report_list_t *)malloc(sizeof(ad_system_mongo_report_list_t))))
	return -1;

    char create_time[128] = {0};
    ad_system_mongo_time_to_string(bson_iter_time_t(&iter_create_time), create_time, 0);

    sprintf(node->report_id, "%s", bson_iter_utf8(&iter_id, NULL));
    sprintf(node->report_name, "%s", bson_iter_utf8(&iter_name, NULL));
    sprintf(node->report_type, "%s", bson_iter_utf8(&iter_type, NULL));
    sprintf(node->range_time, "%s", bson_iter_utf8(&iter_range_time, NULL));
    sprintf(node->create_time, "%s", create_time);
    sprintf(node->status, "%s", bson_iter_utf8(&iter_status, NULL));
    sprintf(node->path, "%s", bson_iter_utf8(&iter_path, NULL));

    lg_list_add(head, &node->list);
    return 0;
}

int ad_system_mongo_report_list_find(char *account_id, int report_belong, lg_list_t *head)
{

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    lg_list_init(head);
    char report_belong_str[128];
    sprintf(report_belong_str, "%u", report_belong);

    bson_t query;
    bson_init (&query);
    bson_append_utf8(&query, "account_id", -1, account_id, -1);
    bson_append_utf8(&query, "belong", -1, report_belong_str, -1);

    if (ad_system_mongo_find(&conn, MONGO_REPORT_LIST, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    while (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    ad_system_mongo_report_list_element(conn.doc, head);
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

int ad_system_mongo_report_list_del(char *account_id, const char *report_id)
{

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query;
    bson_init (&query);
    bson_append_utf8(&query, "account_id", -1, account_id, -1);
    bson_append_utf8(&query, "id", -1, report_id, -1);

    if (ad_system_mongo_delete(&conn, MONGO_REPORT_LIST, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

//后台管理登录
int ad_system_mongo_delivery_center_find(const char *key, char *account_id_str)
{

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query;
    bson_init (&query);
    bson_append_utf8(&query, "_id", -1, key, -1);

    if (ad_system_mongo_find(&conn, MONGO_DELIVERY_CENTER_LIST, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    if (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    bson_iter_t iter_account_id;
	    IF_ITER_FIND_STRING(conn.doc, &iter_account_id, "account_id")
		return -1;

	    sprintf(account_id_str, "%s", bson_iter_utf8(&iter_account_id, NULL));
	    ad_system_mongo_delete(&conn, MONGO_DELIVERY_CENTER_LIST, &query);

	    bson_destroy (&query);
	    ad_system_mongo_free(&conn);
	    return 0;
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

int ad_system_mongo_stat_load(ad_system_mongo_stat_t *p, const char *table, int display_style, const char *account_id)
{

    memset(p, 0, sizeof(ad_system_mongo_stat_t));

    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query;
    bson_init (&query);
    bson_append_int32(&query, "account_id", -1, atoi(account_id));
    bson_append_int32(&query, "display_style", -1, display_style);

    if (ad_system_mongo_find(&conn, table, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1; 
    }

    if (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    bson_iter_t iter_impression;
	    if (!bson_iter_init_find(&iter_impression, conn.doc, "impression"))
		return -1;

	    bson_iter_t iter_click;
	    if (!bson_iter_init_find(&iter_click, conn.doc, "click"))
		return -1;

	    bson_iter_t iter_cost;
	    if (!bson_iter_init_find(&iter_cost, conn.doc, "cost"))
		return -1;

	    p->impression = ad_system_mongo_to_number(&iter_impression);
	    p->click = ad_system_mongo_to_number(&iter_click);
	    p->cost = ad_system_mongo_to_double(&iter_cost);
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

int ad_system_mongo_stat_list_element(lg_list_t *head, const bson_t *doc, const char *name)
{

    bson_iter_t iter_name;
    if (!bson_iter_init_find(&iter_name, doc, name))
	return -1;

    bson_iter_t iter_impression;
    if (!bson_iter_init_find(&iter_impression, doc, "impression"))
	return -1;

    bson_iter_t iter_click;
    if (!bson_iter_init_find(&iter_click, doc, "click"))
	return -1;

    bson_iter_t iter_cost;
    if (!bson_iter_init_find(&iter_cost, doc, "cost"))
	return -1;

    ad_system_mongo_stat_t *node = NULL;
    if (!(node = malloc(sizeof(ad_system_mongo_stat_t))))
	return -1;

    if (strcmp(name, "time") == 0)
	ad_system_mongo_time_to_string(bson_iter_time_t(&iter_name), node->name, 1);
    else if (bson_iter_type(&iter_name) == BSON_TYPE_UTF8)
	strcpy(node->name, bson_iter_utf8(&iter_name, NULL));
    else
	sprintf(node->name, "%u", ad_system_mongo_to_number(&iter_name));

    node->impression = ad_system_mongo_to_number(&iter_impression);
    node->click = ad_system_mongo_to_number(&iter_click);
    node->cost = ad_system_mongo_to_double(&iter_cost);
    lg_list_add(head, &node->next);
    return 0;
}

int ad_system_mongo_stat_list_load(lg_list_t *head, const char *table, int display_style, const char *account_id, const char *name)
{

    lg_list_init(head);
    ad_system_mongo_t conn;
    if (ad_system_mongo_conn(&conn))
	return -1;

    bson_t query;
    bson_init (&query);
    bson_append_int32(&query, "account_id", -1, atoi(account_id));
    bson_append_int32(&query, "display_style", -1, display_style);
    fprintf(stdout, "%s\n", bson_as_json(&query, NULL));

    if (ad_system_mongo_find(&conn, table, &query)) {

	bson_destroy (&query);
	ad_system_mongo_free(&conn);
	return -1;
    }

    while (!mongoc_cursor_error (conn.cursor, &conn.error) &&
	    mongoc_cursor_more (conn.cursor)) {

	if (mongoc_cursor_next(conn.cursor, &conn.doc)) {

	    ad_system_mongo_stat_list_element(head, conn.doc, name);
	}
    }

    bson_destroy (&query);
    ad_system_mongo_free(&conn);
    return 0;
}

ad_system_mongo_stat_t *ad_system_mongo_stat_list_find(lg_list_t *head, const char *name)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, head){

	ad_system_mongo_stat_t *node = (ad_system_mongo_stat_t *)p;
	if (strcmp(node->name, name) == 0)
	    return node;
    }

    return NULL;
}




