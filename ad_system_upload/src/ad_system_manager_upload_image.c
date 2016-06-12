
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ad_system_packet.h"
#include "ad_system_upload_image.h"
#include "gd.h"

typedef union {

    void *arg;
    int sockfd;

} ad_system_upload_image_sockfd_t;

char *ad_system_upload_image_http_value(char *buffer, char *value, int value_size, char *delim, int delim_size)
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

int ad_system_upload_image_http_parse(char *buffer, size_t buffer_size, ad_system_upload_image_t *http)
{

    char *str = NULL;
    if (!(str = strstr(buffer, "Content-Type:")))
	return -1;

    char Content_Type[4096] = {0};
    if (!ad_system_upload_image_http_value(str, Content_Type, 4096, "\r\n", 2))
	return -1;

    if (!(str = (strstr(Content_Type, "boundary="))))
	return -1;

    if (strlen(str) > 64)
	return -1;

    http->boundary_size = sprintf(http->boundary, "--%s", &str[9]);

    if (!(str = strstr(buffer, "Content-Length:")))
	return -1;

    char Content_Length[4096] = {0};
    if (!ad_system_upload_image_http_value(str, Content_Length, 4096, "\r\n", 2))
	return -1;

    if (!(http->Content_Length = atoi(&Content_Length[15])))
	return -1;

    if (!(str = strstr(buffer, "\r\n\r\n")))
	return -1;

    size_t http_head_size = str - buffer + 4;
    http->packet_size = http->Content_Length + http_head_size;
    return 0;
}

int ad_system_upload_image_buffer_file_name_parse(char *file_name, ad_system_upload_image_t *http)
{

    int i, j;
    for (i = 0, j = 0; file_name[i]; i++) {

	if (file_name[i] == '\r' && file_name[i + 1] == '\n')
	    return 0;

	http->file_name[j++] = file_name[i];
    }

    return -1;
}

int ad_system_upload_image_buffer_parse(char *buffer, ad_system_upload_image_t *http)
{

    size_t http_head_size = http->packet_size - http->Content_Length;
    if (strncmp(&buffer[http_head_size], http->boundary, http->boundary_size))
	return -1;

    char *file_name = NULL;
    if (!(file_name = strstr(&buffer[http_head_size], "Content-Disposition: form-data; name=\"Filename\"\r\n\r\n")))
	return -1;

    if (ad_system_upload_image_buffer_file_name_parse(file_name + 51, http) != 0)
	return -1;

    if (!(http->image_buffer = strstr(&buffer[http_head_size], "Content-Type: application/octet-stream\r\n\r\n")))
	return -1;

    http->image_buffer += 42;
    http->image_size = http->packet_size - (http->image_buffer - buffer) - 153;

    if (strncmp(&http->image_buffer[http->image_size + 2], http->boundary, http->boundary_size))
	return -1;

    //fprintf(stdout, "Packet-size=%zd\n", http->packet_size);
    //fprintf(stdout, "Content-Length=%zd\n", http->Content_Length);
    //fprintf(stdout, "Image-size=%zd\n", http->image_size);
    //fprintf(stdout, "File_name=%s\n", http->file_name);
    return 0;
}

int ad_system_upload_image_read(int sockfd, char *buffer, ad_system_upload_image_t *http)
{

    size_t size = 0;
    size_t buffer_size = 0;

    for(;;){

	size = read(sockfd, &buffer[buffer_size], 4096);
	if (size <= 0)
	    return -1;

	if (buffer_size == 0) {

	    if (ad_system_upload_image_http_parse(buffer, size, http) != 0)
		return -1;
	}

	buffer_size += size;
	if (buffer_size > http->packet_size)
	    return -1;

	if (http->packet_size == buffer_size)
	    return 0;
    }

    return -1;
}

int ad_system_upload_image_write(ad_system_upload_image_t *http)
{

    int i, j;
    char key[64] = {0};
    char path[1024] = {0};

    uuid_t uuid;
    uuid_generate(uuid);

    for (i = 0, j = 0; i < 16; i++, j++, j++)
	sprintf(&key[j], "%02x", uuid[i]);

    for (i = 0, j = 0; i < 32; i++) {

	http->path[j++] = '/';
	http->path[j++] = key[i];
	sprintf(path, "%s%s", UPLOAD_IMG_FILE_PATH, http->path);

	if (access(path, 0))
	    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		return -1;
    }

    sprintf(&http->path[64], "/temp.jpg");//%s", http->file_name);
    sprintf(path, "%s%s", UPLOAD_IMG_FILE_PATH, http->path);
    //fprintf(stdout, "-------->[path=%s]\n", path);

    FILE *fp = NULL;
    if (!(fp = fopen(path, "wb")))
	return -1;

    size_t size = fwrite(http->image_buffer, 1, http->image_size, fp);
    fclose(fp);

    if (size != http->image_size)
	return -1;

    return 0;
}

int ad_system_upload_image_check(ad_system_upload_image_t *http)
{

    /***************************************
      BMP 0x4D42,
      JPG 0xD8FF,
      PNG[4]={0x5089,0x474E,0x0A0D,0x0A1A},
      GIF[3]={0x4947,0x3846,0x6139};
     ***************************************/

    unsigned short img_type = *(unsigned short *)http->image_buffer;

    if (img_type == 0xd8ff) {

	gdImagePtr im = NULL;
	if ((im = gdImageCreateFromJpegPtr(http->image_size, http->image_buffer))) {

	    http->image_height = im->sy;
	    http->image_width = im->sx;
	    gdImageDestroy(im);
	    return 0;
	}

    } else if ((strncmp(http->image_buffer, "GIF87a", 6) == 0) || (strncmp(http->image_buffer, "GIF89a", 6) == 0)) {

	gdImagePtr im = NULL;
	if ((im = gdImageCreateFromGifPtr(http->image_size, http->image_buffer))) {

	    http->image_height = im->sy;
	    http->image_width = im->sx;
	    gdImageDestroy(im);
	    return 0;
	}
    }

    return -1;
}

void *ad_system_upload_image_thread(void *arg)
{
    pthread_detach(pthread_self());

    ad_system_upload_image_sockfd_t temp;
    temp.arg = arg;
    int sockfd = temp.sockfd;

    struct timeval time_out = {0};
    time_out.tv_sec = 5;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (void *)&time_out, sizeof(time_out));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&time_out, sizeof(time_out));

    char buffer[1048576] = {0};
    ad_system_upload_image_t http;
    memset(&http, 0, sizeof(ad_system_upload_image_t));
    //fprintf(stdout, "upload image load...\n");

    if (ad_system_upload_image_read(sockfd, buffer, &http) == 0) {

	if (ad_system_upload_image_buffer_parse(buffer, &http) == 0) {

	    if (ad_system_upload_image_check(&http) == 0) {

		if (ad_system_upload_image_write(&http) == 0) {

		    size_t size = sprintf(buffer, 
			    "%s{\"CODE\":\"0\","
			    "\"FILE_NAME\":\"%s\","
			    "\"FILE_SIZE\":\"%zd\","
			    "\"FILE_WIDTH\":\"%d\","
			    "\"FILE_HEIGHT\":\"%d\","
			    "\"FILE_PATH\":\"%s\"}",

			    HTTP_HEAD,
			    http.file_name,
			    http.image_size,
			    http.image_width,
			    http.image_height,
			    http.path);

		    write(sockfd, buffer, size);
		    close(sockfd);
		    pthread_exit(NULL);
		}
	    }
	}
    }

    size_t size = sprintf(buffer, "%s{\"CODE\":\"%u\"}", HTTP_HEAD, PACKET_CODE_IMG_UPLOAD_FILE_ERR);
    write(sockfd, buffer, size);
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
    addr.sin_port = htons(6003);
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {

	close(listenfd);
	return -1;
    }

    if (listen(listenfd, 1000) == -1) {

	close(listenfd);
	return -1;
    }

    for (;;) {

	ad_system_upload_image_sockfd_t temp;
	if ((temp.sockfd = accept(listenfd, NULL, NULL)) > 0) {

	    pthread_t tid;
	    pthread_create(&tid, NULL, ad_system_upload_image_thread, temp.arg);
	}
    }

    return 0;
}

