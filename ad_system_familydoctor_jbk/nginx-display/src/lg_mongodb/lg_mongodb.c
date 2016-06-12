
#include "lg_mongodb.h"

void *__lg_mongodb_malloc(void *pool, size_t size)
{
    return malloc(size);
}

void __lg_mongodb_free(void *pool, void *p)
{
    free(p);
}

int lg_mongodb_pool_init(lg_mongodb_pool_t *mongodb_pool, char *host, int port)
{

    mongoc_uri_t *uri = mongoc_uri_new_for_host_port(host, port);
    if (!uri) return -1;

    mongoc_client_pool_t *pool = mongoc_client_pool_new(uri);
    if (!pool) return -1;

    memset(mongodb_pool, 0, sizeof(lg_mongodb_pool_t));
    mongodb_pool->pool = pool;
    mongodb_pool->conn_count = 0;

    mongodb_pool->memory_pool = NULL;
    mongodb_pool->malloc = __lg_mongodb_malloc;
    mongodb_pool->free = __lg_mongodb_free;
    return 0;
}

int lg_mongodb_pool_conn(lg_mongodb_pool_t *mongodb_pool, lg_mongodb_t *mongodb, const char *db_name, const char *table_name)
{

    if (!mongodb_pool || !mongodb_pool->pool)
	return -1;

    mongoc_client_t *client = mongoc_client_pool_pop(mongodb_pool->pool);
    if (!client) return -1;

    mongoc_collection_t *collection = NULL;
    if (!(collection = mongoc_client_get_collection (client, db_name, table_name))) {

	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    mongodb->client = client;
    mongodb->conn_pool = mongodb_pool;
    mongodb_pool->conn_count++;

    mongodb->collection = collection;
    lg_list_init(&mongodb->collection_head);

    mongodb->cursor = NULL;
    lg_list_init(&mongodb->cursor_head);

    mongodb->doc = NULL;

    mongodb->memory_pool = mongodb_pool->memory_pool;
    mongodb->malloc = mongodb_pool->malloc;
    mongodb->free = mongodb_pool->free;
    return 0;
}

int lg_mongodb_pool_ping(lg_mongodb_pool_t *mongodb_pool)
{

    if (!mongodb_pool || !mongodb_pool->pool)
	return -1;

    mongoc_client_t *client = mongoc_client_pool_pop(mongodb_pool->pool);
    if (!client) return -1;

    mongoc_database_t *database = mongoc_client_get_database(client, "local");
    if (!database) {

	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    bson_t ping;
    bson_init(&ping);
    bson_append_int32(&ping, "ping", -1, 1);
    mongoc_cursor_t *cursor = mongoc_database_command(database, 0, 0, 1, 0, &ping, NULL, NULL);
    if (!cursor) {

	bson_destroy(&ping);
	mongoc_database_destroy(database);
	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    const bson_t *doc = NULL;
    if (!mongoc_cursor_next(cursor, &doc)) {

	bson_destroy(&ping);
	mongoc_cursor_destroy(cursor);
	mongoc_database_destroy(database);
	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    // fprintf(stdout, "%s\n", bson_as_json(doc, NULL));

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, doc, "ok")) {

	bson_destroy(&ping);
	mongoc_cursor_destroy(cursor);
	mongoc_database_destroy(database);
	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    int ret = 0;
    switch (bson_iter_type(&iter)) {

	case BSON_TYPE_INT32:
	    ret = bson_iter_int32(&iter);
	    break;

	case BSON_TYPE_INT64:
	    ret = bson_iter_int64(&iter);
	    break;

	case BSON_TYPE_DOUBLE:
	    ret = bson_iter_double(&iter);
	    break;

	default:
	    bson_destroy(&ping);
	    mongoc_cursor_destroy(cursor);
	    mongoc_database_destroy(database);
	    mongoc_client_pool_push(mongodb_pool->pool, client);
	    return -1;
    }

    if (ret != 1) {

	bson_destroy(&ping);
	mongoc_cursor_destroy(cursor);
	mongoc_database_destroy(database);
	mongoc_client_pool_push(mongodb_pool->pool, client);
	return -1;
    }

    bson_destroy(&ping);
    mongoc_cursor_destroy(cursor);
    mongoc_database_destroy(database);
    mongoc_client_pool_push(mongodb_pool->pool, client);
    return 0;
}

int lg_mongodb_conn(lg_mongodb_t *mongodb, char *host, int port, const char *db_name, const char *table_name)
{

    char uri_host[1024] = {0};
    sprintf(uri_host, "mongodb://%s:%d",  host, port);
    memset(mongodb, 0, sizeof(lg_mongodb_t));
    if (!(mongodb->client = mongoc_client_new (uri_host)))
	return -1;

    if (!(mongodb->collection = mongoc_client_get_collection (mongodb->client, db_name, table_name))) {

	mongoc_client_destroy (mongodb->client);
	return -1;
    }

    mongodb->conn_pool = NULL;
    lg_list_init(&mongodb->collection_head);

    mongodb->cursor = NULL;
    lg_list_init(&mongodb->cursor_head);

    mongodb->doc = NULL;

    mongodb->memory_pool = NULL;
    mongodb->malloc = __lg_mongodb_malloc;
    mongodb->free = __lg_mongodb_free;
    return 0;
}

void lg_mongodb_free(lg_mongodb_t *mongodb)
{

    if (!mongodb || !mongodb->client)
	return;

    lg_list_t *p, *temp = NULL;
    p = mongodb->cursor_head.next;
    while(p != &mongodb->cursor_head) {

	temp = p;
	p = p->next;

	lg_mongodb_cursor_t *node = (lg_mongodb_cursor_t *)temp;
	if (node->cursor)
	    mongoc_cursor_destroy(node->cursor);

	if (mongodb->free)
	    mongodb->free(mongodb->memory_pool, temp);
    }

    p = mongodb->collection_head.next;
    while(p != &mongodb->collection_head) {

	temp = p;
	p = p->next;

	lg_mongodb_collection_t *node = (lg_mongodb_collection_t *)temp;
	if (node->collection)
	    mongoc_collection_destroy(node->collection);

	if (mongodb->free)
	    mongodb->free(mongodb->memory_pool, temp);
    }

    if (mongodb->cursor)
	mongoc_cursor_destroy(mongodb->cursor);

    mongoc_collection_destroy(mongodb->collection);

    if (mongodb->conn_pool) {

	mongoc_client_pool_push(mongodb->conn_pool->pool, mongodb->client);
	mongodb->conn_pool->conn_count--;

    } else {

	mongoc_client_destroy (mongodb->client);
    }

    mongodb->client = NULL;
    mongodb->conn_pool = NULL;
    mongodb->collection = NULL;
    mongodb->cursor = NULL;
    mongodb->doc = NULL;
}

int lg_mongodb_new_collection(lg_mongodb_t *mongodb, const char *db_name, const char *table_name)
{

    mongoc_collection_t *collection = NULL;
    if (!(collection = mongoc_client_get_collection (mongodb->client, db_name, table_name)))
	return -1;

    lg_mongodb_collection_t *node = mongodb->malloc(mongodb->memory_pool, sizeof(lg_mongodb_collection_t));
    if (!node) {

	mongoc_collection_destroy(collection);
	return -1;
    }

    node->collection = mongodb->collection;
    lg_list_add(&mongodb->collection_head, &node->next);
    mongodb->collection = collection;
    return 0;
}

int lg_mongodb_insert(lg_mongodb_t *mongodb, bson_t *query)
{

    if (!mongodb || !mongodb->client)
	return -1;

    if (!mongoc_collection_insert (mongodb->collection, 
		MONGOC_INSERT_NONE, 
		query, 
		NULL, 
		&mongodb->error)) {

	return -1;
    }

    return 0;
}

int lg_mongodb_update(lg_mongodb_t *mongodb, bson_t *query, bson_t *update)
{

    if (!mongodb || !mongodb->client)
	return -1;

    if (!mongoc_collection_update(mongodb->collection, 
		MONGOC_UPDATE_NONE, 
		query, 
		update, 
		NULL, 
		&mongodb->error)) {

	return -1;
    }

    return 0;
}

int lg_mongodb_delete(lg_mongodb_t *mongodb, bson_t *query)
{

    if (!mongodb || !mongodb->client)
	return -1;

    if (!mongoc_collection_remove(mongodb->collection,
		MONGOC_REMOVE_NONE,
		query,
		NULL,
		&mongodb->error)) {

	return -1;
    }

    return 0;
}

int lg_mongodb_new_cursor(lg_mongodb_t *mongodb, bson_t *query, int limit)
{

    mongoc_cursor_t *cursor = NULL;
    if (!(cursor = mongoc_collection_find (
		    mongodb->collection,
		    MONGOC_QUERY_NONE,
		    0,				/* skip */
		    limit,			/* limit */
		    0,				/* batch_size */
		    query,
		    NULL,  			/* Fields, NULL for all */
		    NULL))) 			/* Read Prefs, NULL for default */
	return -1;

    if (mongoc_cursor_error (cursor, &mongodb->error)) {

	mongoc_cursor_destroy(cursor);
	return -1;
    }

    if (mongodb->cursor) {

	lg_mongodb_cursor_t *node = mongodb->malloc(mongodb->memory_pool, sizeof(lg_mongodb_cursor_t));
	if (!node) {

	    mongoc_cursor_destroy(cursor);
	    return -1;
	}

	lg_list_add(&mongodb->cursor_head, &node->next);
    }

    mongodb->cursor = cursor;
    return 0;
}

int lg_mongodb_find_limit(lg_mongodb_t *mongodb, bson_t *query, size_t limit)
{

    if (!mongodb || !mongodb->client)
	return -1;

    bson_t _query;
    if (!query) {

	query = &_query;
	bson_init(query);
    }

    if (lg_mongodb_new_cursor(mongodb, query, limit)) {

	bson_destroy(query);
	return -1;
    }

    bson_destroy(query);
    return 0;
}

int lg_mongodb_find(lg_mongodb_t *mongodb, bson_t *query)
{
    return lg_mongodb_find_limit(mongodb, query, 0);
}

int lg_mongodb_find_one(lg_mongodb_t *mongodb, bson_t *query)
{
    if (lg_mongodb_find_limit(mongodb, query, 1))
	return -1;

    return lg_mongodb_next(mongodb);
}

int lg_mongodb_next(lg_mongodb_t *mongodb)
{

    if (!mongodb || !mongodb->client)
	return -1;

    if (!mongoc_cursor_error (mongodb->cursor, &mongodb->error)
	    && mongoc_cursor_more (mongodb->cursor)) {

	if (mongoc_cursor_next(mongodb->cursor, &mongodb->doc)) {

	    return 0;
	}
    }

    return -1;
}

int __lg_mongodb_to_number(bson_iter_t *iter)
{

    int value = 0;
    int type = bson_iter_type(iter);
    switch (type) {

	case BSON_TYPE_INT32:
	    value = bson_iter_int32(iter);
	    break;

	case BSON_TYPE_INT64:
	    value = bson_iter_int64(iter);
	    break;

	case BSON_TYPE_DOUBLE:
	    value = bson_iter_double(iter);
	    break;

	case BSON_TYPE_UTF8:
	    value = atoi(bson_iter_utf8(iter, NULL));
	    break;
    }

    return value;
}

int lg_mongodb_to_number(lg_mongodb_t *mongodb, char *name)
{

    if (!mongodb || !mongodb->client)
	return -1;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, mongodb->doc, name)) {

	return -1;
    }

    return __lg_mongodb_to_number(&iter);
}

double __lg_mongodb_to_double(bson_iter_t *iter)
{

    double value = 0;
    int type = bson_iter_type(iter);
    switch (type) {

	case BSON_TYPE_INT32:
	    value = bson_iter_int32(iter);
	    break;

	case BSON_TYPE_INT64:
	    value = bson_iter_int64(iter);
	    break;

	case BSON_TYPE_DOUBLE:
	    value = bson_iter_double(iter);
	    break;

	case BSON_TYPE_UTF8:
	    value = atof(bson_iter_utf8(iter, NULL));
	    break;
    }   

    return value;
}

double lg_mongodb_to_double(lg_mongodb_t *mongodb, char *name)
{

    if (!mongodb || !mongodb->client)
	return -1;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, mongodb->doc, name)) {

	return -1;
    }

    return __lg_mongodb_to_double(&iter);
}

char *lg_mongodb_to_string(lg_mongodb_t *mongodb, char *name, uint32_t *size)
{

    if (!mongodb || !mongodb->client)
	return NULL;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, mongodb->doc, name)) {

	return NULL;
    }

    if (!BSON_ITER_HOLDS_UTF8(&iter)) {

	return NULL;
    }

    return (char *)bson_iter_utf8(&iter, size);
}

char *lg_mongodb_to_string_r(lg_mongodb_t *mongodb, const bson_t *doc, char *name, uint32_t *size)
{

    if (!mongodb || !mongodb->client)
	return NULL;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, doc, name)) {

	return NULL;
    }

    if (!BSON_ITER_HOLDS_UTF8(&iter)) {

	return NULL;
    }

    return (char *)bson_iter_utf8(&iter, size);
}

time_t lg_mongodb_to_time(lg_mongodb_t *mongodb, char *name)
{

    if (!mongodb || !mongodb->client)
	return 0;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, mongodb->doc, name)) {

	return 0;
    }

    if (!BSON_ITER_HOLDS_DATE_TIME(&iter)) {

	return 0;
    }

    return bson_iter_time_t(&iter);
}

char *lg_mongo_to_time_string(lg_mongodb_t *mongodb, char *name, char *buffer, int type)
{

    time_t t = 0;
    if (!(t = lg_mongodb_to_time(mongodb, name)))
	return NULL;

    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    gmtime_r(&t, &tm);

    switch(type) {

	case 1://年
	    sprintf(buffer, "%04d", tm.tm_year + 1900);
	    return buffer;

	case 2://月
	    sprintf(buffer, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);
	    return buffer;

	case 3://日
	    sprintf(buffer, "%04d-%02d-%02d",
		    tm.tm_year + 1900,
		    tm.tm_mon + 1,
		    tm.tm_mday);
	    return buffer;

	default://时分秒
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

int lg_mongodb_to_document(lg_mongodb_t *mongodb, char *name, bson_t *document_out)
{

    if (!mongodb || !mongodb->client)
	return -1;

    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, mongodb->doc, name)) {

	return -1;
    }

    if (!BSON_ITER_HOLDS_DOCUMENT(&iter)) {

	return -1;
    }

    uint32_t document_len = 0;
    const uint8_t *document = NULL;
    bson_iter_document(&iter, &document_len, &document);
    if (!bson_init_static (document_out, document, document_len))
	return -1;

    return 0;
}



