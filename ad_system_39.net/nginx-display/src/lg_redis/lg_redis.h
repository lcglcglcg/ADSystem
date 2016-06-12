
#ifndef	__LG_REDIS_H__
#define	__LG_REDIS_H__

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	int sockfd;
	char host[16];
	int port;

    } lg_redis_conn_t;

    typedef struct {

	char *str;
	int size;

    } lg_redis_value_t;

    typedef struct {

	lg_redis_value_t *key;
	lg_redis_value_t *value;

    } lg_redis_key_value_t;


    // open
    extern int lg_redis_conn(lg_redis_conn_t *conn, char *host, int port);
    extern void lg_redis_close(lg_redis_conn_t *conn);
    extern int lg_redis_base(lg_redis_conn_t *conn, const char *base);

    // key value
    extern int lg_redis_add_key_value(lg_redis_conn_t *conn, const char *key, int key_size, const char *value, int value_size);
    extern int lg_redis_add_key_number(lg_redis_conn_t *conn, const char *key, int key_size, int number);
    extern lg_redis_value_t *lg_redis_get_value(lg_redis_conn_t *conn, const char *key, int key_size);

    // rand key value
    extern lg_redis_value_t *lg_redis_rand_key(lg_redis_conn_t *conn);			//rand key
    extern lg_redis_key_value_t *lg_redis_rand_key_value(lg_redis_conn_t *conn);	//rand key value
    extern lg_redis_value_t *lg_redis_rand_value(lg_redis_conn_t *conn);		//rand value

    // key list
    extern int lg_redis_add_list_key_value(lg_redis_conn_t *conn, const char *key, int key_size, const char *value, int value_size);

#ifdef __cplusplus
}
#endif

#endif


