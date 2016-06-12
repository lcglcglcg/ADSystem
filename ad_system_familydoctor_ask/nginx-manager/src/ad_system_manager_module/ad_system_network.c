
#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <pthread.h>
#include "ad_system_sql.h"

void ad_system_http_send_print(ngx_http_request_t *r, ngx_chain_t *buffer, size_t size)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    ngx_chain_t *p = NULL;
    for (p = buffer; p; p = p->next) {

	fprintf(stdout, "[send][%04d-%02d-%02d %02d:%02d:%02d][%d][%s]",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec,
		r->connection->fd, r->ad_system_account_id);
	fwrite(p->buf->pos, (p->buf->last - p->buf->pos), 1, stdout);
	fprintf(stdout, "\n");
	fflush(stdout);

	fprintf(stderr, "[send][%04d-%02d-%02d %02d:%02d:%02d][%d][%s]",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec,
		r->connection->fd, r->ad_system_account_id);
	fwrite(p->buf->pos, (p->buf->last - p->buf->pos), 1, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);
    }   
}

int ad_system_http_send(ngx_http_request_t *r, ngx_chain_t *buffer, size_t size)
{

    //ad_system_sql_free(&r->sql_conn);

    //r->headers_out.content_type.len = sizeof("text/plain") - 1;
    //r->headers_out.content_type.data = (u_char *) "text/plain";
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = size;
    int rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {

	ngx_http_finalize_request(r, rc);
	return -1;
    }

    ad_system_http_send_print(r, buffer, size);

    ngx_http_output_filter(r, buffer);
    ngx_http_finalize_request(r, NGX_OK);
    return 0;
}

ngx_chain_t *ad_system_buffer_to_ngx_chain(ngx_http_request_t *r, char *p_buffer, size_t size)
{

    ngx_chain_t *buffer = NULL;
    if (!(buffer = ngx_pcalloc(r->pool, sizeof(ngx_chain_t))))
	return NULL;

    buffer->buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    buffer->next = NULL;

    buffer->buf->pos = ngx_pcalloc(r->pool, size);
    ngx_memcpy(buffer->buf->pos, p_buffer, size);
    buffer->buf->last = buffer->buf->pos + size;

    buffer->buf->memory = 1;
    buffer->buf->last_buf = 1;
    return buffer;
}

int ad_system_http_send_buffer(ngx_http_request_t *r, char *buffer, size_t size)
{

    ngx_chain_t *ngx_buffer = ad_system_buffer_to_ngx_chain(r, buffer, size);
    return ad_system_http_send(r, ngx_buffer, size);
}

void ad_system_chain_add(ngx_chain_t **head, ngx_chain_t **next, ngx_chain_t *p)
{
    p->next = NULL;

    if (!(*head)) {

	*head = p;

    } else if (!(*next)) {

	(*head)->buf->last_buf = 0;
	(*head)->next = *next = p;

    } else {

	(*next)->buf->last_buf = 0;
	(*next)->next = p;
	*next = p;
    }
}

void ad_system_chain_add_r(ngx_http_request_t *r, ngx_chain_t **head, ngx_chain_t **next, char *buffer, size_t size)
{

    ngx_chain_t *p = ad_system_buffer_to_ngx_chain(r, buffer, size);
    ad_system_chain_add(head, next, p);
}

int ngx_ad_system_get_session(ngx_http_request_t *r)
{

    ngx_str_t name = ngx_string("ADXKEY");
    ngx_str_t *value = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
    if (ngx_http_parse_multi_header_lines(&r->headers_in.cookies, &name, value) == NGX_DECLINED)
	return -1;

    int i;
    char session_id[1024] = {0};
    char *buffer = (char *)value->data;
    for (i = 0; buffer[i]; i++) {

	if (buffer[i] == ';')
	    break;

	if (i >= 1024)
	    return -1;

	session_id[i] = buffer[i];
    }

    int size = strlen(session_id) + 1;
    r->ad_system_session_id = ngx_pcalloc(r->pool, size);
    memcpy(r->ad_system_session_id, session_id, size);
    return 0;
}

int ngx_ad_system_set_session(ngx_http_request_t *r, char *session_id)
{

    ngx_table_elt_t *set_cookie;
    set_cookie = ngx_list_push (&r->headers_out.headers);
    if (set_cookie == NULL)
	return -1;

    char *buffer = ngx_pcalloc(r->pool, 128);
    size_t len = sprintf(buffer, "ADXKEY=%s; Path=/action;", session_id);

    set_cookie->hash = 1;
    ngx_str_set (&set_cookie->key, "Set-Cookie");
    set_cookie->value.data = (void *)buffer;
    set_cookie->value.len = len;
    return 0;
}

char *ngx_ad_system_get_ipaddr(ngx_http_request_t *r)
{

#if 0
    ngx_connection_t *connection = r->connection;
    if (!connection) return NULL;

    ngx_str_t *addr_text = &connection->addr_text;
    return addr_text->data;
#else 
    ngx_http_headers_in_t *headers_in = &r->headers_in;
    if (!headers_in) return NULL;

    ngx_array_t *x_forwarded_for = &headers_in->x_forwarded_for;
    if (!x_forwarded_for) return NULL;

    ngx_int_t count = x_forwarded_for->nelts;
    if (count <= 0) return NULL;

    ngx_table_elt_t **p = x_forwarded_for->elts;
    return (char *)p[0]->value.data;
#endif
    return NULL;
}

void ad_system_ngx_http_close_request(ngx_http_request_t *r)
{
    r = r->main;
    ngx_connection_t *connection = r->connection;
    ngx_http_free_request(r, 0);
    ngx_http_close_connection(connection);
}


