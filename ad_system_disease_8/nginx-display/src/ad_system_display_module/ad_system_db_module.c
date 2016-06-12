
#include "lg_string.h"
#include "ad_system_display_module.h"
#include "ad_system_display_packet.h"
#include "ad_system_display_db.h"

/* Commands */
ngx_command_t ad_system_display_module_commands[] = {
    { ngx_string("ad_system_display_module"),
	NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
	ad_system_display_module_command,
	NGX_HTTP_LOC_CONF_OFFSET,
	0,
	NULL },

    ngx_null_command
};

ngx_http_module_t ad_system_display_module_ctx = {
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
ngx_module_t ad_system_display_module = {
    NGX_MODULE_V1,
    &ad_system_display_module_ctx,	/* module context */
    ad_system_display_module_commands,	/* module directives */
    NGX_HTTP_MODULE,		/* module type */
    NULL,			/* init master */
    NULL,			/* init module */
    ad_system_display_module_worker_init,	/* init process */
    NULL,			/* init thread */
    NULL,			/* exit thread */
    NULL,			/* exit process */
    NULL,			/* exit master */
    NGX_MODULE_V1_PADDING
};

ngx_int_t ad_system_display_module_worker_init(ngx_cycle_t *cycle)
{

    lg_ocilib_init();
    ad_system_db_init();
    return NGX_OK;
}

ngx_int_t ad_system_display_module_handler(ngx_http_request_t *r)
{

    if (r->method != NGX_HTTP_GET)
	return NGX_ERROR;

    char *buffer = (char *)r->args.data;
    if (!buffer || !r->args.len)
	return NGX_ERROR;

    size_t size = lg_string_url_decode(buffer, buffer, r->args.len);
    if (size <= 0) return NGX_ERROR;

    return ad_system_display_packet_parse(r, buffer, size);
}

char *ad_system_display_module_command(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

    ngx_http_core_loc_conf_t *loc_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    loc_conf->handler = ad_system_display_module_handler;
    return NGX_CONF_OK;
}



