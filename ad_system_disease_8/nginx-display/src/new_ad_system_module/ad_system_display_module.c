
#include "ad_system_module.h"
#include "ad_system_packet.h"

/* Commands */
static ngx_command_t  ad_system_module_commands[] = {
    { ngx_string("ad_system_module"),
	NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
	ad_system_module_command,
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
    ad_system_module_worker_init,	/* init process */
    NULL,			/* init thread */
    NULL,			/* exit thread */
    NULL,			/* exit process */
    NULL,			/* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t ad_system_module_worker_init(ngx_cycle_t *cycle)
{

    if (lg_ocilib_init()) {

	fprintf(stdout, "init oracle:: err\n");
	return NGX_ERROR;
    }

    if (lg_captcha_init(NULL)) {

	fprintf(stdout, "init libgd:: err\n");
	return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t ad_system_module_handler_get(ngx_http_request_t *r)
{

    char *buffer = (char *)r->args.data;
    size_t size = r->args.len;

    if (size > PACKET_HEAD_LEN && strncmp(buffer, PACKET_HEAD, PACKET_HEAD_LEN) == 0) {

	ad_system_packet_parse(r, (buffer + PACKET_HEAD_LEN), size - PACKET_HEAD_LEN);
	return NGX_OK;
    }

    return NGX_ERROR;
}

static void ad_system_module_handler_post(ngx_http_request_t *r)
{

    ngx_str_t *str = lg_ngx_network_get_post_buffer(r);
    if (!str) lg_ngx_network_buffer_post_send_str(r, NULL, "{\"CODE\":\"9998\"}");

    char *buffer = (char *)(str->data);
    if (str->len >PACKET_HEAD_LEN && strncmp(buffer, PACKET_HEAD, PACKET_HEAD_LEN) == 0) {

	return ad_system_packet_parse(r, (buffer + PACKET_HEAD_LEN), str->len);
    }

    return lg_ngx_network_buffer_post_send_str(r, NULL, "{\"CODE\":\"9998\"}");
}

static ngx_int_t ad_system_module_handler(ngx_http_request_t *r)
{

    if (r->method == NGX_HTTP_GET) {

	return ad_system_module_handler_get(r);

    } else if (r->method == NGX_HTTP_POST) {

	ngx_int_t rc = ngx_http_read_client_request_body(r, ad_system_module_handler_post);
	if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
	    return rc; 

	return NGX_DONE;
    }

    return NGX_ERROR;
}

static char *ad_system_module_command(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

    ngx_http_core_loc_conf_t *loc_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    loc_conf->handler = ad_system_module_handler;
    return NGX_CONF_OK;
}



