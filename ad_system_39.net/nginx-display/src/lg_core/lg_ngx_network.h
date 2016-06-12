
#ifndef __LG_NGX_NETWORK_H__
#define __LG_NGX_NETWORK_H__

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	ngx_chain_t *head;
	ngx_chain_t *next;
	ngx_str_t content_type;
	size_t content_length;

    } lg_ngx_network_t;

    extern int lg_ngx_network_buffer_add(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size);

    extern int lg_ngx_network_send(ngx_http_request_t *r, lg_ngx_network_t *p);
    extern void lg_ngx_network_post_send(ngx_http_request_t *r, lg_ngx_network_t *p);

    extern int lg_ngx_network_str_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *str);
    extern void lg_ngx_network_str_post_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *str);

    extern int lg_ngx_network_buffer_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size);
    extern void lg_ngx_network_buffer_post_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size);
    
    extern int lg_ngx_network_buffer_send_str(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer);
    extern void lg_ngx_network_buffer_post_send_str(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer);

    extern int lg_ngx_network_not_found(ngx_http_request_t *r);
    extern void lg_ngx_network_post_not_found(ngx_http_request_t *r);

    extern ngx_str_t *lg_ngx_network_get_post_buffer(ngx_http_request_t *r);
    extern ngx_str_t *lg_ngx_network_post_upload_file_parse(ngx_http_request_t *r, ngx_str_t *str);

#ifdef __cplusplus
}
#endif

#endif


