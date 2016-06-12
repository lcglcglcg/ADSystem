
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lg_redis.h"

#define LG_REDIS_BUF_SIZE 40960
#define LG_REDIS_RANDOMKEY_STR "*1\r\n$9\r\nRANDOMKEY\r\n"
#define LG_REDIS_RANDOMKEY_SIZE 19
#define LG_REDIS_SELECT_HEAD_STR "*2\r\n$6\r\nselect\r\n"
#define LG_REDIS_SELECT_HEAD_SIZE 16
#define LG_REDIS_GET_VALUE_HEAD_STR "*2\r\n$3\r\nget\r\n"
#define LG_REDIS_GET_VALUE_HEAD_SIZE 13
#define LG_REDIS_SET_VALUE_HEAD_STR "*3\r\n$3\r\nset\r\n"
#define LG_REDIS_SET_VALUE_HEAD_SIZE 13
#define LG_REDIS_RPUSH_HEAD_STR "*3\r\n$5\r\nrpush\r\n"
#define LG_REDIS_RPUSH_HEAD_SIZE 15

typedef struct {

    int head_size;
    int buffer_size;

    char *value_buffer;
    int value_size;

    lg_redis_value_t *value;

} lg_redis_value_buffer_info_t;

static int lg_redis_network_noblocking(int sockfd)
{

    int opts = -1;
    if ((opts = fcntl(sockfd, F_GETFD, 0)) == -1)
	return -1;

    if ((fcntl(sockfd, F_SETFL, opts | O_NONBLOCK)) == -1)
	return -1;

    return 0;
}

static int lg_redis_network_check(int sockfd)
{

    int error = -1;
    socklen_t len = sizeof(int);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
	return -1;

    return error;
}

static int lg_redis_network_time_wait(int sockfd)
{

    struct linger opt;
    opt.l_onoff = 0x01;
    opt.l_linger = 0x00;
    return setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &opt, sizeof(struct linger));
}

static int lg_redis_network_connect(char *host, int port)
{

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {\

	close(sockfd);
	return -1;
    }

    return sockfd;
}

static int lg_redis_network_init(lg_redis_conn_t *conn)
{

    int sockfd = lg_redis_network_connect(conn->host, conn->port);
    if (sockfd < 0) return -1;

    if (lg_redis_network_noblocking(sockfd)) {

	close(sockfd);
	conn->sockfd = -1;
	return -1;
    }

    if (lg_redis_network_time_wait(sockfd)) {

	close(sockfd);
	conn->sockfd = -1;
	return -1;
    }

#if 0
    if (lg_redis_network_kernel_buffer(sockfd, LG_REDIS_BUF_SIZE, LG_REDIS_BUF_SIZE)) {

	close(sockfd);
	conn->sockfd = -1;
	return -1;
    }
#endif

    conn->sockfd = sockfd;
    return 0;
}

static int lg_redis_network_recv(lg_redis_conn_t *conn, char *buffer, int buffer_size)
{

    int size = read(conn->sockfd, buffer, buffer_size);
    if (size == -1) {

	if (errno == EAGAIN || errno == EINTR)
	    return 0;

	if (lg_redis_network_check(conn->sockfd)) {

	    close(conn->sockfd);
	    lg_redis_network_init(conn);
	    return -1;
	}

	return 0;

    } else if (size == 0) {

	if (lg_redis_network_check(conn->sockfd)) {

	    close(conn->sockfd);
	    lg_redis_network_init(conn);
	    return -1;
	}

	return 0;
    }

    // fprintf(stdout, "%d\n", size);
    return size;
}

int lg_redis_conn(lg_redis_conn_t *conn, char *host, int port)
{

    int host_len = strlen(host);
    if (host_len >= 16) return -1;

    strcpy(conn->host, host);
    conn->port = port;
    if (lg_redis_network_init(conn))
	return -1;

    return 0;
}

void lg_redis_close(lg_redis_conn_t *conn)
{
    close(conn->sockfd);
}

int lg_redis_value_info_parse(lg_redis_conn_t *conn, lg_redis_value_buffer_info_t *info, char *buffer)
{

    if (!buffer) return -1;

    char *str = strstr(buffer, "\r\n");
    if (!str) return -1;

    info->head_size = (str - buffer);
    info->value_size = atoi(&buffer[1]);
    info->buffer_size = info->head_size + info->value_size + 4;
    if (info->head_size <= 0 || info->value_size <= 0)
	return -1;

    info->value_buffer = malloc(info->value_size + 2);//+2防止越界
    if (!info->value_buffer)
	return -1;

    info->value = malloc(sizeof(lg_redis_value_t));
    if (!info->value) {

	free(info->value_buffer);
	return -1;
    }

    info->value->str = info->value_buffer;
    info->value->size = 0;
    return 0;
}

int lg_redis_get_value_buffer(lg_redis_conn_t *conn, lg_redis_value_buffer_info_t *info)
{

    int buffer_size = 0;
    char buffer[LG_REDIS_BUF_SIZE];
    memset(info, 0, sizeof(lg_redis_value_buffer_info_t));

    for (;;) {

	int size = lg_redis_network_recv(conn, buffer, LG_REDIS_BUF_SIZE);
	if (size > 0) {

	    if (buffer_size == 0) {

		if (buffer[0] != '$')
		    return -1;

		//if (size == 5 && strncmp(buffer, "$-1\r\n", 5) == 0)
		//return -1;

		if (size < 7) return -1;

		if (lg_redis_value_info_parse(conn, info, buffer))
		    return -1;

		if (size > info->value_size) {

		    memcpy(info->value->str, &buffer[info->head_size + 2], info->value_size);
		    info->value->size = info->value_size;
		    return 0;
		}

		int s_size = size - info->head_size - 2;
		memcpy(info->value->str, &buffer[info->head_size + 2], s_size);
		info->value->str += s_size;
		info->value->size = s_size;

	    } else {

		if (buffer_size + size == info->buffer_size) {

		    int e_size = size - 2;
		    memcpy(info->value->str, buffer, e_size);
		    info->value->str = info->value_buffer;
		    info->value->size += e_size;
		    info->value->str[info->value->size] = 0;
		    // fprintf(stdout, "%d-%d\n", info->value_size, info->value->size); 
		    return 0;

		} else if (buffer_size + size > info->buffer_size) {

		    return -1;

		} else {

		    memcpy(info->value->str, buffer, size);
		    info->value->str += size;
		    info->value->size += size;
		}
	    }

	    buffer_size += size;
	}
    }

    return 0;
}

int lg_redis_base(lg_redis_conn_t *conn, const char *base)
{

    if (!conn || !base) return -1;

    char buffer[128];
    int base_size = strlen(base);
    if (base_size > 120) return -1;
    int size = sprintf(buffer, "$%d\r\n", base_size);

    if (write(conn->sockfd, LG_REDIS_SELECT_HEAD_STR, LG_REDIS_SELECT_HEAD_SIZE) != LG_REDIS_SELECT_HEAD_SIZE)
	return -1;
    if (write(conn->sockfd, buffer, size) != size)
	return -1;
    if (write(conn->sockfd, base, base_size) != base_size)
	return -1;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return -1;

    // COEE RECV
    return 0;
}

lg_redis_value_t *lg_redis_get_value(lg_redis_conn_t *conn, const char *key, int key_size)
{

    if (!conn || !key || key_size <= 0) return NULL;

    char buffer[128];
    int size = sprintf(buffer, "$%d\r\n", key_size);

    if (write(conn->sockfd, LG_REDIS_GET_VALUE_HEAD_STR, LG_REDIS_GET_VALUE_HEAD_SIZE) != LG_REDIS_GET_VALUE_HEAD_SIZE)
	return NULL;
    if (write(conn->sockfd, buffer, size) != size)
	return NULL;
    if (write(conn->sockfd, key, key_size) != key_size)
	return NULL;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return NULL;

    lg_redis_value_buffer_info_t info;
    if (lg_redis_get_value_buffer(conn, &info))
	return NULL;

    return info.value;
}

int lg_redis_add_key_value(lg_redis_conn_t *conn, const char *key, int key_size, const char *value, int value_size)
{

    if (!conn || !key || key_size <= 0) return -1;

    char buffer[128];
    int size = sprintf(buffer, "$%d\r\n", key_size);

    if (write(conn->sockfd, LG_REDIS_SET_VALUE_HEAD_STR, LG_REDIS_SET_VALUE_HEAD_SIZE) != LG_REDIS_SET_VALUE_HEAD_SIZE)
	return -1;
    if (write(conn->sockfd, buffer, size) != size)
	return -1;
    if (write(conn->sockfd, key, key_size) != key_size)
	return -1;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return -1;

    size = sprintf(buffer, "$%d\r\n", value_size);
    if (write(conn->sockfd, buffer, size) != size)
	return -1;

    if (write(conn->sockfd, value, value_size) != value_size)
	return -1;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return -1;

    // CODE RECV
    return 0;
}

int lg_redis_add_key_number(lg_redis_conn_t *conn, const char *key, int key_size, int number)
{

    char value[128];
    int value_size = sprintf(value, "%d", number);
    return lg_redis_add_key_value(conn, key, key_size, value, value_size);
}

lg_redis_value_t *lg_redis_rand_key(lg_redis_conn_t *conn)
{

    if (!conn) return NULL;
    if (write(conn->sockfd, LG_REDIS_RANDOMKEY_STR, LG_REDIS_RANDOMKEY_SIZE) != LG_REDIS_RANDOMKEY_SIZE)
	return NULL;

    lg_redis_value_buffer_info_t info;
    if (lg_redis_get_value_buffer(conn, &info))
	return NULL;

    return info.value;
}

lg_redis_key_value_t *lg_redis_rand_key_value(lg_redis_conn_t *conn)
{

    lg_redis_key_value_t *r = malloc(sizeof(lg_redis_key_value_t));
    if (!r) return NULL;

    lg_redis_value_t *key = lg_redis_rand_key(conn);
    if (!key) return NULL;

    lg_redis_value_t *value = lg_redis_get_value(conn, key->str, key->size);
    if (!value) return NULL;

    r->key = key;
    r->value = value;
    return r;
}

lg_redis_value_t *lg_redis_rand_value(lg_redis_conn_t *conn)
{

    lg_redis_key_value_t *r = lg_redis_rand_key_value(conn);
    if (r) return r->value;
    return NULL;
}

int lg_redis_add_list_key_value(lg_redis_conn_t *conn, const char *key, int key_size, const char *value, int value_size)
{

    if (!conn || !key || key_size <= 0) return -1;

    char buffer[128];
    int size = sprintf(buffer, "$%d\r\n", key_size);

    if (write(conn->sockfd, LG_REDIS_RPUSH_HEAD_STR, LG_REDIS_RPUSH_HEAD_SIZE) != LG_REDIS_RPUSH_HEAD_SIZE)
	return -1;
    if (write(conn->sockfd, buffer, size) != size)
	return -1;
    if (write(conn->sockfd, key, key_size) != key_size)
	return -1;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return -1;

    size = sprintf(buffer, "$%d\r\n", value_size);
    if (write(conn->sockfd, buffer, size) != size)
	return -1;

    if (write(conn->sockfd, value, value_size) != value_size)
	return -1;
    if (write(conn->sockfd, "\r\n", 2) != 2)
	return -1;

    // CODE RECV
    return 0;
}

int lg_redis_get_list_key_value(lg_redis_conn_t *conn, const char *key, const char *value, int value_size)
{

    return 0;
}

int lg_redis_del_key(lg_redis_conn_t *conn, const char *key)
{


    return 0;
}


