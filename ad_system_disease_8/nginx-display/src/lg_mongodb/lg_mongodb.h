
#ifndef __LG_MONGODB_H__
#define __LG_MONGODB_H__

#include <bson.h>
#include <mongoc.h>
#include "lg_list.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef void *(lg_mongodb_call_malloc)(void *pool, size_t size);
    typedef void (lg_mongodb_call_free)(void *pool, void *p);

    typedef struct {

	void *pool;
	int conn_count;

	void *memory_pool;
	lg_mongodb_call_malloc *malloc;
	lg_mongodb_call_free *free;

    } lg_mongodb_pool_t;

    typedef struct {

	lg_list_t next;
	mongoc_collection_t *collection;

    } lg_mongodb_collection_t;

    typedef struct {

	lg_list_t next;
	mongoc_cursor_t *cursor;

    } lg_mongodb_cursor_t;

    typedef struct {

	mongoc_client_t *client;
	lg_mongodb_pool_t *conn_pool;

	mongoc_collection_t *collection;
	lg_list_t collection_head;

	mongoc_cursor_t *cursor;
	lg_list_t cursor_head;

	const bson_t *doc;
	bson_error_t error;

	void *memory_pool;
	lg_mongodb_call_malloc *malloc;
	lg_mongodb_call_free *free;

    } lg_mongodb_t;

    extern int lg_mongodb_pool_init(lg_mongodb_pool_t *mongodb_pool, char *host, int port);
    extern int lg_mongodb_pool_conn(lg_mongodb_pool_t *mongodb_pool, lg_mongodb_t *mongodb, const char *db_name, const char *table_name);
    extern int lg_mongodb_pool_ping(lg_mongodb_pool_t *mongodb_pool);

    extern int lg_mongodb_conn(lg_mongodb_t *mongodb, char *host, int port, const char *db_name, const char *table_name);
    extern void lg_mongodb_free(lg_mongodb_t *mongodb);

    extern int lg_mongodb_new_collection(lg_mongodb_t *mongodb, const char *db_name, const char *table_name);
    extern int lg_mongodb_insert(lg_mongodb_t *mongodb, bson_t *query);
    extern int lg_mongodb_update(lg_mongodb_t *mongodb, bson_t *query, bson_t *update);
    extern int lg_mongodb_delete(lg_mongodb_t *mongodb, bson_t *query);

    extern int lg_mongodb_find_one(lg_mongodb_t *mongodb, bson_t *query);
    extern int lg_mongodb_find_limit(lg_mongodb_t *mongodb, bson_t *query, size_t limit);
    extern int lg_mongodb_find(lg_mongodb_t *mongodb, bson_t *query);
    extern int lg_mongodb_next(lg_mongodb_t *mongodb);

    extern int lg_mongodb_to_number(lg_mongodb_t *mongodb, char *name);
    extern double lg_mongodb_to_double(lg_mongodb_t *mongodb, char *name);
    extern char *lg_mongodb_to_string(lg_mongodb_t *mongodb, char *name, uint32_t *size);
    extern time_t lg_mongodb_to_time(lg_mongodb_t *mongodb, char *name);
    extern char *lg_mongo_to_time_string(lg_mongodb_t *mongodb, char *name, char *buffer, int type);
    extern int lg_mongodb_to_document(lg_mongodb_t *mongodb, char *name, bson_t *document_out);

    extern char *lg_mongodb_to_string_r(lg_mongodb_t *mongodb, const bson_t *doc, char *name, uint32_t *size);

#ifdef __cplusplus
}
#endif

#endif


