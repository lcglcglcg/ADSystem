
#include "ad_system_redis.h"

static char *REDIS_HOST = NULL;
static int REDIS_PORT = 0;
static struct timeval redis_timeout = { 1, 500000 }; // 1.5 seconds
int ad_system_redis_init(lg_list_t *config_file_head)
{

    if (!(REDIS_HOST = lg_config_file_find(config_file_head, "redis_host")))
	return -1;

    char *redis_port_str = NULL;
    if (!(redis_port_str = lg_config_file_find(config_file_head, "redis_port")))
	return -1;

    if (!(REDIS_PORT = atoi(redis_port_str)))
	return -1;;

    fprintf(stdout, "REDIS_HOST=%s\n", REDIS_HOST);
    fprintf(stdout, "REDIS_PORT=%d\n", REDIS_PORT);
    return 0;
}

redisContext *ad_system_redis_connect(char *dbs)
{
    redisContext *c = redisConnectWithTimeout(REDIS_HOST, REDIS_PORT, redis_timeout);
    if (c == NULL || c->err) {

	if (c) redisFree(c);
	return NULL;
    }

    if (dbs) {

	redisReply *r = redisCommand(c, "select %s", dbs);
	if (r->type == REDIS_REPLY_STATUS && strcasecmp(r->str, "OK") == 0) {

	    freeReplyObject(r);
	    return c;
	}

	freeReplyObject(r);
	redisFree(c);
	return NULL;
    }

    return c;
}

int ad_system_redis_add_var_code(char *session_id, const char *value)
{
    if (!session_id || !value)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_VAR_CODE);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "SET %s %s", session_id, value);
    if (reply->type != REDIS_REPLY_STATUS) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    redisReply *time_out = redisCommand(c, "EXPIRE %s 60", session_id);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_var_code(const char *session_id, char *value)
{

    if (!session_id)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_VAR_CODE);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "GET %s", session_id);
    if (reply->type != REDIS_REPLY_STRING) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    if (value) strcpy(value, reply->str);

    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_set_login_session(const char *session_id, const char *value)
{
    if (!session_id || !value)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_SESSION);
    redisReply *reply = redisCommand(c, "SET %s %s", session_id, value);
    if (reply->type != REDIS_REPLY_STATUS) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    redisReply *time_out = redisCommand(c, "EXPIRE %s 1800", session_id);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_login_session(const char *session_id, char *value)
{

    if (!session_id)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_SESSION);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "GET %s", session_id);
    if (reply->type != REDIS_REPLY_STRING) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    if (value) strcpy(value, reply->str);

    redisReply *time_out = redisCommand(c, "EXPIRE %s 1800", session_id);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_del_login_session(const char *session_id)
{
    if (!session_id)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_SESSION);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "DEL %s", session_id);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_creative_info(
	int display_style, 
	char *creative_id,
	char *campaign_name, 
	char *group_name, 
	char *creative_name)
{

    redisContext *conn = ad_system_redis_connect(NULL);
    if (!conn) return -1;

    char buffer[256];
    if (display_style == 1)
	sprintf(buffer, "LRANGE get_img:%s 0 -1", creative_id);
    else
	sprintf(buffer, "LRANGE get_text:%s 0 -1", creative_id);

    redisReply *reply = redisCommand(conn, buffer);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 7) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    strcpy(campaign_name, reply->element[3]->str);
    strcpy(group_name, reply->element[5]->str);
    strcpy(creative_name, reply->element[6]->str);

    freeReplyObject(reply);
    redisFree(conn);
    return 0;
}

int ad_system_redis_add_report(char *report_key, char *report_path)
{

    if (!report_key || !report_path)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_REPORT);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "SET %s %s", report_key, report_path);
    if (reply->type != REDIS_REPLY_STATUS) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    redisReply *time_out = redisCommand(c, "EXPIRE %s 10000", report_key);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_report(const char *report_key, char *report_path)
{

    if (!report_key || !report_path)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_REPORT);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "GET %s", report_key);
    if (reply->type != REDIS_REPLY_STRING) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(report_path, reply->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_add_delivery_center(const char *key, const char *account_id)
{

    redisContext *c = ad_system_redis_connect(REDIS_DB_DELIVERY_CENTER);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "SET %s %s", key, account_id);
    if (reply->type != REDIS_REPLY_STATUS) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    redisReply *time_out = redisCommand(c, "EXPIRE %s 30", key);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_delivery_center(const char *key, char *account_id)
{

    redisContext *c = ad_system_redis_connect(REDIS_DB_DELIVERY_CENTER);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "GET %s", key);
    if (reply->type != REDIS_REPLY_STRING) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    if (account_id) strcpy(account_id, reply->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_creative_name(int display_style, const char *creative_id, char *creative_name)
{
    redisContext *c = ad_system_redis_connect(REDIS_DB_MATERIALS_CREATIVE);
    if (!c) return -1;

    char buffer[256] = {0};
    if (display_style == 1) sprintf(buffer, "LRANGE get_img:%s 0 0", creative_id);
    else sprintf(buffer, "LRANGE get_text:%s 0 0", creative_id);

    redisReply *reply = redisCommand(c, buffer);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(creative_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_group_name(const char *group_id, char *group_name)
{
    redisContext *c = ad_system_redis_connect(REDIS_DB_MATERIALS_GROUP);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "LRANGE %s 0 0", group_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(group_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_campaign_name(const char *campaign_id, char *campaign_name)
{
    redisContext *c = ad_system_redis_connect(REDIS_DB_MATERIALS_CAMPAIGN);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "LRANGE %s 0 0", campaign_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(campaign_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_account_name(int account_id, char *account_name)
{
    redisContext *c = ad_system_redis_connect(REDIS_DB_MATERIALS_ACCOUNT);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "LRANGE %d 0 0", account_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(account_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(c);
    return 0;
}


int ad_system_redis_get_report_money_call(void *priv, lg_list_t *a, lg_list_t *b)
{

    ad_system_redis_recharge_t *node_a = (ad_system_redis_recharge_t *)a;
    ad_system_redis_recharge_t *node_b = (ad_system_redis_recharge_t *)b;
    return strcmp(node_a->key, node_b->key);
}

int ad_system_redis_get_report_money_r(redisContext *conn, const char *key, lg_list_t *head)
{

    redisReply *reply = redisCommand(conn, "LRANGE %s 0 -1", key);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	freeReplyObject(reply);
	return -1;
    }

    ad_system_redis_recharge_t *p = malloc(sizeof(ad_system_redis_recharge_t));
    if (!p) return -1;

    strcpy(p->key, key);
    p->money = atof(reply->element[0]->str);
    p->discount = atof(reply->element[1]->str);
    p->multiple = atof(reply->element[2]->str);
    lg_list_add(head, &p->next);

    freeReplyObject(reply);
    return 0;
}

int ad_system_redis_recharge_get_list(int account_id, lg_list_t *head)
{

    redisContext *conn = ad_system_redis_connect(REDIS_DB_REPORT_MONEY);
    if (!conn) return -1;

    if (!head) return -1;
    lg_list_init(head);

    redisReply *reply = redisCommand(conn, "KEYS %d:*", account_id);
    if (reply->type != REDIS_REPLY_ARRAY) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    size_t i = 0;
    for (i = 0; i < reply->elements; i++) {

	if (ad_system_redis_get_report_money_r(conn, reply->element[i]->str, head)) {

	    freeReplyObject(reply);
	    redisFree(conn);
	    return -1;
	}
    }

    freeReplyObject(reply);
    redisFree(conn);

    lg_list_sort(NULL, head, 4096, ad_system_redis_get_report_money_call);
    return 0;
}


int ad_system_redis_add_money_info(char *key,
	const char *account_id,
	const char *money,
	int payment_type,
	const char *accounting_document_number,
	const char *accounting_document_file,
	const char *accounting_document_remark,
	int BRAND_TYPE,
	int DISCOUNT)
{

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_MONEY);
    if (!c) return -1;

    char buffer[1024] = {0};
    sprintf(buffer, "LPUSH %s %s %s %d %s %s %s %d %d",
	    key,
	    account_id,
	    money,
	    payment_type,
	    *accounting_document_number ? accounting_document_number : "null",
	    *accounting_document_file ? accounting_document_file : "null",
	    *accounting_document_remark ? accounting_document_remark : "null",
	    BRAND_TYPE,
	    DISCOUNT);

    fprintf(stderr, "ad_system_redis_add_money_info:==>[%s]\n", buffer);

    redisReply *reply = redisCommand(c, buffer);
    if (reply && reply->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    redisReply *time_out = redisCommand(c, "EXPIRE %s 10000", key);
    if (time_out->type != REDIS_REPLY_INTEGER) {

	freeReplyObject(reply);
	freeReplyObject(time_out);
	redisFree(c);
	return -1;
    }

    freeReplyObject(reply);
    freeReplyObject(time_out);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_money_info(const char *key,
	char *account_id,
	char *money,
	char *payment_type,
	char *accounting_document_number,
	char *accounting_document_file,
	char *accounting_document_remark,
	char *BRAND_TYPE,
	char *DISCOUNT)
{

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_MONEY);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "LRANGE %s 0 -1", key);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 8) {

	freeReplyObject(reply);
	redisFree(c);
	return -1;
    }

    strcpy(account_id,			reply->element[7]->str);
    strcpy(money,                       reply->element[6]->str);
    strcpy(payment_type,                reply->element[5]->str);
    strcpy(accounting_document_number,  reply->element[4]->str);
    strcpy(accounting_document_file,    reply->element[3]->str);
    strcpy(accounting_document_remark,  reply->element[2]->str);
    strcpy(BRAND_TYPE,  		reply->element[1]->str);
    strcpy(DISCOUNT,  			reply->element[0]->str);

    freeReplyObject(reply);
    redisFree(c);
    return 0;
}

int ad_system_redis_get_img(const char *img_id, char *img_name, char *img_size, char *img_buffer_size, char *img_path)
{

    redisContext *conn = ad_system_redis_connect(REDIS_DB_IMG_INFO);
    if (!conn) return -1;

    redisReply *reply = redisCommand(conn, "ZRANGE %s 0 -1", img_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 4) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    if (img_name)
	strcpy(img_name, reply->element[0]->str);//name           	

    if (img_size)
	strcpy(img_size, reply->element[1]->str);//img_size

    if (img_buffer_size)
	strcpy(img_buffer_size, reply->element[2]->str);//img_buffer_size
    
    if (img_path)
	strcpy(img_path, reply->element[3]->str);//img_buffer_size

    freeReplyObject(reply);
    redisFree(conn);
    return 0;
}




