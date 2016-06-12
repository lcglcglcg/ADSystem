
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <hiredis/hiredis.h>
#include <pthread.h>
#include "lg_string.h"
#include "lg_string_json.h"

extern size_t lg_string_json_sprintf(char *buffer, char *head_str, char *end_str, int count, ...);

typedef struct {

    char boundary[128];
    size_t boundary_size;
    size_t Content_Length;
    size_t size;
    char *buffer;

} ad_system_upload_keyword_http_t;

typedef union {

    int sockfd;
    void *arg;

} ad_system_upload_sockfd_t;

char *ad_system_upload_keyword_http_value(char *buffer, char *value, int value_size, char *delim, int delim_size)
{

    int size = 0;
    char *str = NULL;
    for (str = buffer, size = 0; *str; size++) {

	if (strncmp(str, delim, delim_size) == 0) {

	    *value = 0;
	    return value;
	}

	if (size >= value_size)
	    return NULL;

	*value++ = *str++;
    }

    return NULL;
}

int ad_system_upload_keyword_http_parse(char *buffer, size_t buffer_size, ad_system_upload_keyword_http_t *http)
{

    char *str = NULL;
    if (!(str = strstr(buffer, "Content-Type:")))
	return -1;

    char Content_Type[4096] = {0};
    if (!ad_system_upload_keyword_http_value(str, Content_Type, 4096, "\r\n", 2))
	return -1;

    if (!(str = (strstr(Content_Type, "boundary="))))
	return -1;

    if (strlen(str) > 64)
	return -1;

    http->boundary_size = sprintf(http->boundary, "--%s", &str[9]);
    //fprintf(stdout, "boundary_size=%zd\n", http->boundary_size);

    if (!(str = strstr(buffer, "Content-Length:")))
	return -1;

    char Content_Length[4096] = {0};
    if (!ad_system_upload_keyword_http_value(str, Content_Length, 4096, "\r\n", 2))
	return -1;

    if (!(http->Content_Length = atoi(&Content_Length[15])))
	return -1;

    if (!(str = strstr(buffer, "\r\n\r\n")))
	return -1;

    size_t http_head_size = (size_t)str - (size_t)buffer + 4;
    http->size = http->Content_Length + http_head_size;

    //fprintf(stdout, "boundary=%s\n", http->boundary);
    //fprintf(stdout, "Content_Length=%d\n", http->Content_Length);
    //fprintf(stdout, "size=%d\n", http->size);
    return 0;
}

int ad_system_upload_keyword_buffer_parse(char *buffer, ad_system_upload_keyword_http_t *http)
{

    size_t http_head_size = http->size - http->Content_Length;
    if (strncmp(&buffer[http_head_size], http->boundary, http->boundary_size))
	return -1;

    if (!(http->buffer = strstr(buffer, "Content-Type: application/octet-stream\r\n\r\n")))
	return -1;

    http->buffer += 42;

    char *str = NULL;
    if (!(str = strstr(http->buffer, http->boundary)))
	return -1;

    *str = 0;
    return 0;
}

void ad_system_upload_keyword_send(int sockfd, char *keywords)
{

    char buffer[4096];
    size_t size = sprintf(buffer, "HTTP/1.1 200 OK\r\n\r\n[");
    send(sockfd, buffer, size, 0);
    //fprintf(stdout, buffer);

    char comma[4] = "";
    char *saveptr = NULL;

    for (;;) {

	char *str = strtok_r(keywords, "\r\n", &saveptr);
	if (str == NULL)
	    break;

	char keyword[4096] = {0};
	lg_string_gbk_to_utf8(str, strlen(str), keyword, 4096);
	//size = sprintf(buffer, "%s\"%s\"", comma, keyword);

	struct printbuf *json_str = printbuf_new();
	lg_string_to_json(json_str, keyword);
	size = sprintf(buffer, "%s\"%s\"", comma, json_str->buf);
	printbuf_free(json_str);

	send(sockfd, buffer, size, 0);
	//fprintf(stdout, buffer);

	comma[0] = ',';
	keywords = NULL;
    }

    size = sprintf(buffer, "]");
    send(sockfd, buffer, size, 0);
    //fprintf(stdout, "%s\n", buffer);
}

void *ad_system_upload_keyword_thread(void *arg)
{

    pthread_detach(pthread_self());

    ad_system_upload_sockfd_t temp;
    temp.arg = arg;
    int sockfd = temp.sockfd;

    size_t size = 0;
    size_t buffer_size = 0;
    char buffer[1048576] = {0};

    ad_system_upload_keyword_http_t http;
    memset(&http, 0, sizeof(ad_system_upload_keyword_http_t));

    while(1){

	size = read(sockfd, &buffer[buffer_size], 2048);
	if (size <= 0) {

	    close(sockfd);
	    pthread_exit(NULL);
	}

	if (buffer_size == 0) {

	    if (ad_system_upload_keyword_http_parse(buffer, size, &http) != 0) {

		//fprintf(stdout, "ad_system_upload_keyword_http_parse err\n");
		close(sockfd);
		pthread_exit(NULL);
	    }
	}

	buffer_size += size;
	if (http.size == buffer_size)
	    break;

	if (buffer_size > http.size) {

	    close(sockfd);
	    pthread_exit(NULL);
	}
    }

    //fprintf(stdout, "buffer-size=%zd\n", http.size);
    if (ad_system_upload_keyword_buffer_parse(buffer, &http) != 0)
	fprintf(stdout, "ad_system_upload_keyword_buffer_parse err\n");
    else
	ad_system_upload_keyword_send(sockfd, http.buffer);

    close(sockfd);
    pthread_exit(NULL);
}

int main()
{

    if (fork() > 0) exit(1);

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	return -1;

    int	flag = 0x01;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {

	close(listenfd);
	return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5002);
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {

	close(listenfd);
	return -1;
    }

    if (listen(listenfd, 1000) == -1) {

	close(listenfd);
	return -1;
    }

    for (;;) {

	ad_system_upload_sockfd_t temp;
	if ((temp.sockfd = accept(listenfd, NULL, NULL)) > 0) {

	    pthread_t tid;
	    pthread_create(&tid, NULL, ad_system_upload_keyword_thread, temp.arg);
	}
    }

    return 0;
}




