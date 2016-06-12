

#ifndef __AD_SYSTEM_MODULE_H__
#define __AD_SYSTEM_MODULE_H__

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {


} ad_system_loc_conf_t;

static ngx_int_t ad_system_module_worker_init(ngx_cycle_t *cycle);
static char *ad_system_module_command(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

#endif




