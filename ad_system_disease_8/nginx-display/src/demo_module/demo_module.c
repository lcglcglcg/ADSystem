
#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "lg_ngx_session.h"
#include "lg_ngx_network.h"

#include "lg_ocilib.h"
#include "lg_captcha.h"

static ngx_int_t demo_module_worker_init(ngx_cycle_t *cycle);
static char *demo_module_command(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

/* Commands */
static ngx_command_t  demo_module_commands[] = {
    { ngx_string("demo_module"),
	NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
	demo_module_command,
	NGX_HTTP_LOC_CONF_OFFSET,
	0,
	NULL },

    ngx_null_command
};

static ngx_http_module_t demo_module_ctx = {
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
ngx_module_t demo_module = {
    NGX_MODULE_V1,
    &demo_module_ctx,		/* module context */
    demo_module_commands,	/* module directives */
    NGX_HTTP_MODULE,		/* module type */
    NULL,			/* init master */
    NULL,			/* init module */
    demo_module_worker_init,	/* init process */
    NULL,			/* init thread */
    NULL,			/* exit thread */
    NULL,			/* exit process */
    NULL,			/* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t demo_module_worker_init(ngx_cycle_t *cycle)
{
    // int ret = lg_captcha_init(NULL);
    // fprintf(stdout, "ret=%d\n", ret);

    if (lg_ocilib_init()) {

	fprintf(stdout, "lg_ocilib_init::err\n");
	return NGX_ERROR;
    }

    fprintf(stdout, "lg_ocilib_init::ok\n");
    return NGX_OK;
}

static void demo_module_handler_post(ngx_http_request_t *r)
{

    // ngx_str_t *nstr = lg_ngx_network_post_upload_file_parse(r, lg_ngx_network_get_post_buffer(r));
    // char *str = "{\"code\":0}";
    // lg_ngx_network_buffer_post_send(r, NULL, str, strlen(str));
}

static ngx_int_t demo_module_handler_get(ngx_http_request_t *r)
{

    // lg_captcha_t img;
    // lg_captcha_create(&img);
    // fprintf(stdout, "key=%s\n", img.key);
    // return lg_ngx_network_buffer_send(r, NULL, img.buffer, img.size);

    lg_ocilib_t conn;
    lg_ocilib_conn(&conn, "188.188.3.152/dev", "lcg", "lcg");
    lg_ocilib_find(&conn, "select * from lcglcg");
    while(lg_ocilib_fetch_next(&conn)) {

	fprintf(stdout, "[%s][%s][%s][%s][%s]\n",
		lg_ocilib_get_string(&conn, 1),
		lg_ocilib_get_string(&conn, 2),
		lg_ocilib_get_string(&conn, 3),
		lg_ocilib_get_string(&conn, 4),
		lg_ocilib_get_string(&conn, 5)
	       );
    }

    lg_ocilib_free(&conn);
    return lg_ngx_network_buffer_send(r, NULL, "ok", 2);
}

static ngx_int_t demo_module_handler(ngx_http_request_t *r)
{

    if (r->method == NGX_HTTP_GET) {

	return demo_module_handler_get(r);

    } else if (r->method == NGX_HTTP_POST) {

	ngx_int_t rc = ngx_http_read_client_request_body(r, demo_module_handler_post);
	if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
	    return rc; 

	return NGX_DONE;
    }

    return NGX_ERROR;
}

static char *demo_module_command(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *loc_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    loc_conf->handler = demo_module_handler;
    return NGX_CONF_OK;
}



