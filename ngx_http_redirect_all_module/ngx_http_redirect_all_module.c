#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

ngx_module_t ngx_http_redirect_all_module;

typedef struct {
    ngx_flag_t         enable;
    ngx_int_t 	       requestnum;
} ngx_http_redirect_all_conf_t;

static ngx_int_t ngx_http_redirect_all_handler(ngx_http_request_t *r)
{

	ngx_http_redirect_all_conf_t *conf;
	
	conf = ngx_http_get_module_loc_conf(r, ngx_http_redirect_all_module);

	if(!conf->enable) {
		return NGX_DECLINED;
	}

	ngx_table_elt_t *h;
	h = ngx_list_push(&r->headers_out.headers);
	h->hash = 1;
	ngx_str_set(&h->key, "Location");
	ngx_str_set(&h->value, "https://cybersec.kz/ru");

	conf->requestnum++;

	fprintf(stderr, "detected %zd redirections to cybersec\n", conf->requestnum);

	return NGX_HTTP_MOVED_TEMPORARILY;
}


static ngx_int_t
ngx_http_redirect_all_init(ngx_conf_t *cf)
{
/*
	ngx_http_core_loc_conf_t *clcf;

	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	clcf->handler = ngx_http_redirect_all_handler;

	return NGX_OK;

*/
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
	fprintf(stderr, "detected 0 redirections to cybersec\n");
    *h = ngx_http_redirect_all_handler;

    return NGX_OK;
}

static void *
ngx_http_redirect_all_create_loc_conf(ngx_conf_t *cf)
{ 
    fprintf(stderr, "How the fuck it works\n");
    ngx_http_redirect_all_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_redirect_all_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;
    conf->requestnum = 0;

    return conf;
}


static char *
ngx_http_redirect_all_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{

    ngx_http_redirect_all_conf_t *prev = parent;
    ngx_http_redirect_all_conf_t *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_redirect_all_module_ctx = {
  NULL,                                 /* preconfiguration */
  ngx_http_redirect_all_init,           /* postconfiguration */

  NULL,                                 /* create main configuration */
  NULL,                                 /* init main configuration */
  NULL,                                 /* create server configuration */
  NULL,                                 /* merge server configuration */
  ngx_http_redirect_all_create_loc_conf,                                 /* create location configuration */
  ngx_http_redirect_all_merge_loc_conf                                  /* merge location configuration */
};

static ngx_command_t  ngx_http_redirect_all_commands[] = {

    { ngx_string("redirect_all"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redirect_all_conf_t, enable),
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
