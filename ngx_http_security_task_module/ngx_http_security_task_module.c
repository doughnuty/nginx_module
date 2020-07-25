#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#define I_FOUND 1
#define AM_FOUND 2
#define I_AM_FOUND 3
#define HACKER_FOUND 4
#define I_HACKER_FOUND 5
#define AM_HACKER_FOUND 6
#define ALL_FOUND 7


typedef struct {
    ngx_flag_t enable;
} ngx_http_security_task_loc_conf_t;

static void *ngx_http_security_task_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_security_task_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_security_task_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_security_task_commands[] = {

    { ngx_string("security_task_if"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_security_task_loc_conf_t, enable),
      NULL },
	
      ngx_null_command
};

static ngx_http_module_t  ngx_http_security_task_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_http_security_task_init,            /* postconfiguration */

    NULL,
                                            /* create main configuration */
    NULL,
                                            /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    ngx_http_security_task_create_loc_conf,
                                            /* create location configuration */
    ngx_http_security_task_merge_loc_conf,
                                            /* merge location configuration */
};

ngx_module_t  ngx_http_security_task_module = {
    NGX_MODULE_V1,
    &ngx_http_security_task_module_ctx,     /* module context */
    ngx_http_security_task_commands,        /* module directives */
    NGX_HTTP_MODULE,                            /* module type */
    NULL,                                       /* init master */
    NULL,                                       /* init module */
    NULL,                                       /* init process */
    NULL,                                       /* init thread */
    NULL,                                       /* exit thread */
    NULL,                                       /* exit process */
    NULL,                                       /* exit master */
    NGX_MODULE_V1_PADDING
};

//static ngx_http_request_body_filter_pt ngx_http_next_request_body_filter;
static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
int key_words_search(ngx_str_t line)
{
	int i_found = 0;
	int am_found = 0;
	int ha_found = 0;
	int ret_value = 0;
	unsigned int i = 0;
	char target[7] = {'h', 'a', 'c', 'k', 'e', 'r', '\0'};
	for(i = 0; i < line.len; i++)
	{
		switch (line.data[i])
		{
			case 'i':
				if(i_found == 0) ret_value += 1;
				break;
			case 'a':
				if(i + 1 != line.len)
			{
				if(line.data[i + 1] == 'm' && am_found == 0) 
					ret_value += 2;
					am_found += 1;
			}
				break;
			case 'h':
				if (line.len < i + 5 || ha_found != 0)
					break;
				int j;
				for(j = 1; j < 6; j++)
				{
					i++;
					if(line.data[i+1] != target[j])
						break;
					else j++;
				}
				if(j == 6) ret_value += 4;
				ha_found += 1;
				break;
		}
	}
	return ret_value;
}

static ngx_int_t
ngx_http_security_task_header_filter(ngx_http_request_t *r)
{
    ngx_http_security_task_loc_conf_t  *conf;
    conf = ngx_http_get_module_loc_conf(r, ngx_http_security_task_module);
    if (!conf->enable) {
    return ngx_http_next_header_filter(r);
    }
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "search for the request key words");
    if (key_words_search(r->uri) == ALL_FOUND) {
    	ngx_str_t  uri, args;

    	ngx_str_set(&uri, "/security");
    	ngx_str_set(&args, "");

    	ngx_http_internal_redirect(r, &uri, &args);
    	ngx_http_finalize_request(r, NGX_DONE);
    	return NGX_HTTP_MOVED_PERMANENTLY;
     }
     return ngx_http_next_header_filter(r);
}
/*
static ngx_int_t
ngx_http_security_task_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_http_security_task_conf_t  *conf;
    conf = ngx_http_get_module_loc_conf(r, ngx_http_security_task_filter_module);
    if (!conf->enable) {
    return ngx_http_next_request_body_filter(r, in);
}
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "search for the request key words");
    if (key_words_search(r->uri) == ALL_FOUND) {
    	ngx_str_t  uri, args;
    	ngx_str_set(&uri, "/security");
    	ngx_str_set(&args, "");
    	ngx_http_internal_redirect(r, &uri, &args);
    	ngx_http_finalize_request(r, NGX_DONE);
    	return NGX_HTTP_MOVED_PERMANENTLY;
     }
     return ngx_http_next_request_body_filter(r, in);
}
*/

static void *
ngx_http_security_task_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_security_task_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_security_task_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;
    
    return conf;
}


static char *
ngx_http_security_task_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child)
{
    ngx_http_security_task_loc_conf_t  *prev = parent;
    ngx_http_security_task_loc_conf_t  *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_security_task_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_security_task_header_filter;

    /*ngx_http_next_request_body_filter = ngx_http_top_request_body_filter;
    ngx_http_top_body_filter = ngx_http_security_task_body_filter;
    */
    return NGX_OK;
}
