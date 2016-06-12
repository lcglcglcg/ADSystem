
#include "ad_system_redis.h"

struct timeval timeout = { 1, 500000 }; // 1.5 seconds

static char *REDIS_HOST = NULL;
static int REDIS_PORT = 0;
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
    redisContext *c = redisConnectWithTimeout(REDIS_HOST, REDIS_PORT, timeout);
    if (c == NULL || c->err) {

	if (c) redisFree(c);
	return NULL;
    }

    if (dbs) {

	redisReply *reply = redisCommand(c, "select %s", dbs);
	if (reply->type != REDIS_REPLY_STATUS) {

	    freeReplyObject(reply);
	    redisFree(c);
	    return NULL;
	}

	freeReplyObject(reply);
    }

    return c;
}

int ad_system_redis_add_var_code(char *session_id, const char *value)
{
    if (!session_id || !value)
	return -1;

    redisContext *c = ad_system_redis_connect(REDIS_DB_VAR_CODE);
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

    redisContext *c = ad_system_redis_connect(REDIS_DB_VAR_CODE);
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

    redisContext *c = ad_system_redis_connect(REDIS_DB_SESSION);
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

    redisContext *c = ad_system_redis_connect(REDIS_DB_SESSION);
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

    redisContext *c = ad_system_redis_connect(REDIS_DB_SESSION);
    if (!c) return -1;

    redisReply *reply = redisCommand(c, "DEL %s", session_id);
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

int ad_system_redis_get_creative_name(int display_style, int creative_id, char *creative_name)
{
    redisContext *conn = ad_system_redis_connect(REDIS_DB_MATERIALS_CREATIVE);
    if (!conn) return -1;

    char buffer[256] = {0};
    if (display_style == 1) sprintf(buffer, "LRANGE get_img:%u 0 0", creative_id);
    else sprintf(buffer, "LRANGE get_text:%u 0 0", creative_id);

    redisReply *reply = redisCommand(conn, buffer);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    strcpy(creative_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(conn);
    return 0;
}

int ad_system_redis_get_group_name(int group_id, char *group_name)
{
    redisContext *conn = ad_system_redis_connect(REDIS_DB_MATERIALS_GROUP);
    if (!conn) return -1;

    redisReply *reply = redisCommand(conn, "LRANGE %u 0 0", group_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    strcpy(group_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(conn);
    return 0;
}

int ad_system_redis_get_campaign_name(int campaign_id, char *campaign_name)
{
    redisContext *conn = ad_system_redis_connect(REDIS_DB_MATERIALS_CAMPAIGN);
    if (!conn) return -1;

    redisReply *reply = redisCommand(conn, "LRANGE %u 0 0", campaign_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    strcpy(campaign_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(conn);
    return 0;
}

int ad_system_redis_get_account_name(const char *account_id, char *account_name)
{
    redisContext *conn = ad_system_redis_connect(REDIS_DB_MATERIALS_ACCOUNT);
    if (!conn) return -1;

    redisReply *reply = redisCommand(conn, "LRANGE %s 0 0", account_id);
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 1) {

	freeReplyObject(reply);
	redisFree(conn);
	return -1;
    }

    strcpy(account_name, reply->element[0]->str);
    freeReplyObject(reply);
    redisFree(conn);
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

int ad_system_redis_get_manager_center(const char *key)
{

    redisContext *c = ad_system_redis_connect(REDIS_DB_MANAGER_CENTER);
    if (!c) return -1; 

    redisReply *reply = redisCommand(c, "GET %s", key);
    if (!reply) {

	freeReplyObject(reply);
	redisFree(c);
	return -1; 
    }   

    if (reply->type != REDIS_REPLY_STRING) {

	freeReplyObject(reply);
	redisFree(c);
	return -1; 
    }   

    freeReplyObject(reply);
    //reply = redisCommand(c, "DEL %s", key);
    //if (reply)freeReplyObject(reply);

    redisFree(c);
    return 0;
}


