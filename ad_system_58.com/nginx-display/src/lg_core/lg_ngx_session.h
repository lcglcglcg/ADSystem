
#ifndef __LG_NGX_SESSION_H__
#define __LG_NGX_SESSION_H__

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

#ifdef __cplusplus
extern "C" { 
#endif

    extern char *lg_ngx_network_get_user_agent(ngx_http_request_t *r);
    extern char *lg_ngx_network_get_ipaddr(ngx_http_request_t *r, char *addr);
    extern char *lg_ngx_network_get_forwarded_ipaddr(ngx_http_request_t *r);

    extern int lg_ngx_network_set_cookie(ngx_http_request_t *r, char *cookie, size_t cookie_size);
    extern char *lg_ngx_network_get_cookie(ngx_http_request_t *r, char *cookie);

    extern int lg_ngx_network_set_session(ngx_http_request_t *r, char *key, char *session_id);
    extern int lg_ngx_network_get_session(ngx_http_request_t *r, char *key, char **session_id);

#ifdef __cplusplus
}
#endif

#endif


