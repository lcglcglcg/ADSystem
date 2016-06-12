
#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <pthread.h>

#include "lg_string.h"
#include "ad_system_init.h"
#include "ad_system_packet.h"

static char *ad_system_module_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_ad_system_worker_init(ngx_cycle_t *cycle);

/* Commands */
static ngx_command_t  ad_system_module_commands[] = {
    { ngx_string("ad_system_module"),
	NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
	ad_system_module_init,
	NGX_HTTP_LOC_CONF_OFFSET,
	0,
	NULL },

    ngx_null_command
};

static ngx_http_module_t ad_system_module_ctx = {
    NULL,			/* preconfiguration */
    NULL,			/* postconfiguration */

    NULL,			/* create main configuration */
    NULL,			/* init main configuration */

    NULL,			/* create server configuration */
    NULL,			/* merge server configuration */

    NULL,			/* create location configuration */
    NULL			/* merge location configuration */
};

/* hook */
ngx_module_t ad_system_module = {
    NGX_MODULE_V1,
    &ad_system_module_ctx,	/* module context */
    ad_system_module_commands,	/* module directives */
    NGX_HTTP_MODULE,		/* module type */
    NULL,			/* init master */
    NULL,			/* init module */
    ngx_ad_system_worker_init,	/* init process */
    NULL,			/* init thread */
    NULL,			/* exit thread */
    NULL,			/* exit process */
    NULL,			/* exit master */
    NGX_MODULE_V1_PADDING
};

void *ad_system_packet_thread(void *arg)
{
    pthread_detach(pthread_self());

    ngx_http_request_t *r = arg;
    lg_string_filter(r->ad_system_buffer, r->ad_system_buffer, r->ad_system_buffer_size, '&');
    lg_string_url_decode(r->ad_system_buffer, r->ad_system_buffer, r->ad_system_buffer_size);
    ad_system_packet_parse(r);

    pthread_exit(NULL);
}

void ad_system_module_post_handler(ngx_http_request_t *r)
{

    if (r->headers_in.content_length == NULL) {

	ngx_http_finalize_request(r, NGX_ERROR);
	return;
    }

    r->ad_system_buffer_size = 0;
    size_t content_length = atoi((char *)r->headers_in.content_length->value.data);
    r->ad_system_buffer = ngx_palloc(r->pool, content_length + 1);

    size_t temp_size = 0;
    ngx_buf_t* temp_buf = NULL;
    ngx_chain_t* bufs = r->request_body->bufs;
    while (bufs) {

	temp_buf = bufs->buf;
	temp_size = temp_buf->last - temp_buf->pos;
	bufs = bufs->next;

	if(r->ad_system_buffer_size + temp_size > content_length) {

	    memcpy(r->ad_system_buffer + r->ad_system_buffer_size, temp_buf->pos, content_length - r->ad_system_buffer_size);
	    r->ad_system_buffer_size = content_length;
	    break;
	}

	memcpy(r->ad_system_buffer + r->ad_system_buffer_size, temp_buf->pos, temp_size);
	r->ad_system_buffer_size += temp_size;
    }

    if (r->ad_system_buffer_size <= PACKET_HEAD_LEN) {
	ngx_http_finalize_request(r, NGX_ERROR);
	return;
    }

    if (strncmp(r->ad_system_buffer, PACKET_HEAD, PACKET_HEAD_LEN)) {
	ngx_http_finalize_request(r, NGX_ERROR);
	return;
    }

    r->ad_system_buffer[r->ad_system_buffer_size] = 0;
    r->ad_system_buffer = r->ad_system_buffer + PACKET_HEAD_LEN;
    r->ad_system_buffer_size = r->ad_system_buffer_size - PACKET_HEAD_LEN;
    // fprintf(stdout, "[POST]%s\n",  r->ad_system_buffer);

    r->count++;
    pthread_t pid;
    pthread_create(&pid, NULL, ad_system_packet_thread, r);
}

static ngx_int_t ad_system_module_handler(ngx_http_request_t *r)
{

    if (r->method == NGX_HTTP_POST) {

	ngx_int_t rc = ngx_http_read_client_request_body(r, ad_system_module_post_handler);
	if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
	    return rc; 
	return NGX_DONE;

    } else if (r->method == NGX_HTTP_GET) {

	r->ad_system_buffer_size = r->args.len;
	r->ad_system_buffer = ngx_palloc(r->pool, r->ad_system_buffer_size + 1);
	memcpy(r->ad_system_buffer, r->args.data, r->args.len);
	// fprintf(stderr, "[GET]%s\n",  r->ad_system_buffer);

	if (r->ad_system_buffer_size <= PACKET_HEAD_LEN)
	    return NGX_ERROR;
	if (strncmp(r->ad_system_buffer, PACKET_HEAD, PACKET_HEAD_LEN))
	    return NGX_ERROR;

	r->ad_system_buffer[r->ad_system_buffer_size] = 0;
	r->ad_system_buffer = r->ad_system_buffer + PACKET_HEAD_LEN;
	r->ad_system_buffer_size = r->ad_system_buffer_size - PACKET_HEAD_LEN;
	// fprintf(stderr, "[GET]%s\n",  r->ad_system_buffer);

	r->count++;
	pthread_t pid;
	pthread_create(&pid, NULL, ad_system_packet_thread, r);
	return NGX_DONE;
    }   

    return NGX_ERROR;
}

static char *ad_system_module_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ad_system_module_handler;
    return NGX_CONF_OK;
}

static ngx_int_t ngx_ad_system_worker_init(ngx_cycle_t *cycle)
{

    if (ad_system_init())
	return NGX_ERROR;
    return NGX_OK;
}






