#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

// regex compile structure. I know its better to avoid global vars but have no idea how to implement this differently                                                                      
ngx_regex_compile_t rc;

ngx_module_t ngx_http_redirect_all_module;

typedef struct {
    ngx_flag_t         enable;
    ngx_int_t 	       requestnum;
} ngx_http_redirect_all_conf_t;

/*void
ngx_http_redirect_all_body_hander(ngx_http_request_t *r)
{
	ngx_chain_t  *in, out;

// if key words found redirect
	ngx_http_finalize_request(r, NGX_HTTP_MOVED_TEMPORARILY);
}*/

static ngx_int_t ngx_http_redirect_all_handler(ngx_http_request_t *r)
{
	ngx_http_redirect_all_conf_t 	 *conf;
	ngx_list_part_t 		 *part;
	ngx_table_elt_t		       *header;
	ngx_uint_t		   headers_num;
	ngx_uint_t 		   	 i = 0;

	ngx_int_t  		       matches; 
	int    captures[(1 + rc.captures) * 3];

	conf = ngx_http_get_module_loc_conf(r, ngx_http_redirect_all_module);

	if(!conf->enable) {
		return NGX_DECLINED;
	}
	// search with regex in uri
	matches = ngx_regex_exec(rc.regex, &r->uri, captures, (1 + rc.captures) * 3);
	fprintf(stderr, "Regex output is %zd\n", matches);				
	fprintf(stderr, "Regex value is %s\n", rc.pattern.data);				

	// search in headers
	part = &r->headers_in.headers.part;
	
	while(part != NULL && matches <= NGX_REGEX_NO_MATCHED)
	{
		headers_num = part->nelts;
		header = part->elts;

		for(i = 0; i < headers_num; i++)
		{
			matches += ngx_regex_exec(rc.regex, &header[i].key, captures, (1 + rc.captures) * 3);
	fprintf(stderr, "Regex output is %zd\n", matches);				
			matches += ngx_regex_exec(rc.regex, &header[i].value, captures, (1 + rc.captures) * 3);
	fprintf(stderr, "Regex output is %zd\n", matches);
			matches++;
		}
		part = part->next;
	}

	// if the request has body process it and search for the key words inside
	/*if (r->method == NGX_HTTP_POST) 
	{
		rc = ngx_http_read_client_request_body(r, ngx_http_redirect_all_body_hander);

		if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
			return rc;
		}

		ngx_http_finalize_request(r, NGX_DONE);
		return NGX_DONE;		
	}*/
	// otherwise continue with what you have
	if(matches >= 0)
	{
		ngx_table_elt_t *location;
		location = ngx_list_push(&r->headers_out.headers);
		location->hash = 1;
		ngx_str_set(&location->key, "Location");
		ngx_str_set(&location->value, "https://cybersec.kz/ru");

		ngx_http_clear_location(r);
		r->headers_out.location = location;

		conf->requestnum++;

		/* to avoid using shell try sending requestnum as ngx_str_t to buf->pos calloced from r->pool 
		just like it was done in footer filter */

		fprintf(stderr, "detected %zd redirections to cybersec\n", conf->requestnum);

		return NGX_HTTP_MOVED_TEMPORARILY;
	}
	else if (matches != NGX_REGEX_NO_MATCHED)
	{
		fprintf(stderr, "Regex error\n");
	}
	fprintf(stderr, "Regex output is %zd\n", matches);
	return NGX_DECLINED;
}


static ngx_int_t
ngx_http_redirect_all_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;



     #if (NGX_PCRE)
     u_char                errstr[NGX_MAX_CONF_ERRSTR];

     ngx_str_t  value = ngx_string("/.*i.*am.*hacker.*/i");

     ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

     rc.pattern = value;
     rc.pool = cf->pool;
     rc.err.len = NGX_MAX_CONF_ERRSTR;
     rc.err.data = errstr;
     /* rc.options are passed as is to pcre_compile() */

     if (ngx_regex_compile(&rc) != NGX_OK) {
         ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%V", &rc.err);
         return NGX_ERROR;
     }
     #endif

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
	
	//push to content phase
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

// for testing purpose only, remove later
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "detected 0 redirections to cybersec\n");

    *h = ngx_http_redirect_all_handler;

    return NGX_OK;
}

static void *
ngx_http_redirect_all_create_loc_conf(ngx_conf_t *cf)
{ 
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
