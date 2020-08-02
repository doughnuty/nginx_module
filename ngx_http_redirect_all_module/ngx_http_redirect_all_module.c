#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

//ngx_module_t ngx_http_redirect_all_module;
/*
typedef struct {
    ngx_flag_t         enable;
} ngx_http_redirect_all_conf_t;
*/

static ngx_int_t ngx_http_redirect_all_handler(ngx_http_request_t *r)
{
	ngx_table_elt_t *h;
	h = ngx_list_push(&r->headers_out.headers);
	h->hash = 1;
	ngx_str_set(&h->key, "Location");
	ngx_str_set(&h->value, "https://cybersec.kz/ru");

	return NGX_HTTP_MOVED_TEMPORARILY;
}


/*static ngx_int_t
ngx_http_redirect_all_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt *h;
	ngx_http_core_main_conf_t *cmcf;

	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

	h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
 	if (h == NULL) {
	    return NGX_ERROR;
	}

  	*h = ngx_http_redirect_all_handler;

	return NGX_OK;
}

*/
static ngx_http_module_t ngx_http_redirect_all_module_ctx = {
  NULL,                                 /* preconfiguration */
  NULL,           /* postconfiguration */

  NULL,                                 /* create main configuration */
  NULL,                                 /* init main configuration */
  NULL,                                 /* create server configuration */
  NULL,                                 /* merge server configuration */
  NULL,                                 /* create location configuration */
  NULL                                  /* merge location configuration */
};

static char *ngx_http_redirect_all(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	clcf->handler = ngx_http_redirect_all_handler;

	return NGX_CONF_OK;
}
static ngx_command_t  ngx_http_redirect_all_commands[] = {

    { ngx_string("redirect_all"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF,
      ngx_http_redirect_all,
      0,
      0,
      NULL },

      ngx_null_command
};

ngx_module_t ngx_http_redirect_all_module = {
  NGX_MODULE_V1,
  &ngx_http_redirect_all_module_ctx, /* module context */
  ngx_http_redirect_all_commands, /* module directives */
  NGX_HTTP_MODULE,                 /* module type */
  NULL,                            /* init master */
  NULL,                            /* init module */
  NULL,                            /* init process */
  NULL,                            /* init thread */
  NULL,                            /* exit thread */
  NULL,                            /* exit process */
  NULL,                            /* exit master */
  NGX_MODULE_V1_PADDING
};
