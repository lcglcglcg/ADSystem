
#ifndef __LG_NGX_URI_H__
#define __LG_NGX_URI_H__

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "lg_ngx_network.h"

#define LG_NGX_URI_TYPE_URL		0X01
#define LG_NGX_URI_TYPE_MOBILE_URL	0X02

#define LG_NGX_URI_HTML_STR (u_char *)"text/html"
#define LG_NGX_URI_HTML_SIZE 9
#define LG_NGX_URI_CSS_STR (u_char *)"text/css"
#define LG_NGX_URI_CSS_SIZE 8
#define LG_NGX_URI_JS_STR (u_char *)"application/x-javascript"
#define LG_NGX_URI_JS_SIZE 24
#define LG_NGX_URI_GIF_STR (u_char *)"image/gif"
#define LG_NGX_URI_GIF_SIZE 9
#define LG_NGX_URI_ICO_STR (u_char *)"image/x-icon"
#define LG_NGX_URI_ICO_SIZE 12
#define LG_NGX_URI_JPEG_STR (u_char *)"image/jpeg"
#define LG_NGX_URI_JPEG_SIZE 10
#define LG_NGX_URI_PNG_STR (u_char *)"image/png"
#define LG_NGX_URI_PNG_SIZE 9
#define LG_NGX_URI_BMP_STR (u_char *)"application/x-bmp"
#define LG_NGX_URI_BMP_SIZE 17
#define LG_NGX_URI_IMG_STR (u_char *)"application/x-img"
#define LG_NGX_URI_IMG_SIZE 17
#define LG_NGX_URI_TXT_STR (u_char *)"text/plain"
#define LG_NGX_URI_TXT_SIZE 10
#define LG_NGX_URI_JS2_STR (u_char *)"text/javascript"
#define LG_NGX_URI_JS2_SIZE 15

#define LG_NGX_URI_TYPE_ROOT		1  // home
#define LG_NGX_URI_TYPE_HTML		2  // .html 		text/html
#define LG_NGX_URI_TYPE_CSS		3  // .css		text/css
#define LG_NGX_URI_TYPE_JS		4  // .js		application/x-javascript
#define LG_NGX_URI_TYPE_GIF		5  // .gif 		image/gif
#define LG_NGX_URI_TYPE_ICO		6  // .ico 		image/x-icon
#define LG_NGX_URI_TYPE_JPEG		7  // .jpe .jpg .jpeg 	image/jpeg
#define LG_NGX_URI_TYPE_PNG		8  // .png		image/png
#define LG_NGX_URI_TYPE_BMP		9  // .bmp 		application/x-bmp
#define LG_NGX_URI_TYPE_IMG		10 // .img		application/x-img
#define LG_NGX_URI_TYPE_TXT             11 // .txt		text/plain

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	int type;
	int mobile_type;

	char *key;
	int key_size;

	char *domain;
	int domain_size;

	char *affiliated_file;

    } lg_ngx_uri_t;

    extern int lg_ngx_uri_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri);

#ifdef __cplusplus
}
#endif

#endif


