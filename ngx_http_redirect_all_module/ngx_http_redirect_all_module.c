#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct // Struct to hold which words were found
{
	int i_found;
	int am_found;
	int hacker_found;
	
	int all_found;

} redirect_module_counter;

static redirect_module_counter key_words_search(ngx_str_t line, redirect_module_counter counter)
{
        char target[7] = {'h', 'a', 'c', 'k', 'e', 'r', '\0'};

        unsigned int i = 0;
        for(i = 0; i < line.len; i++)
        {
                switch (line.data[i])
                {
                        case 'i':
				counter.i_found += 1;
				fprintf(stderr, "I character found in redirect module\n");
                                break;
                        case 'a':
                                if(i + 1 != line.len)
                                {
                                	if(line.data[i + 1] == 'm' && counter.am_found == 0)
                                    	{
		                                fprintf(stderr, "AM key word found in redirect module\n");
     						counter.am_found += 1;
						i++;
					}
                                }
                                break;
                        case 'h':
                                if (line.len < i + 5 || counter.hacker_found != 0)
                                        break;
                                int j;
                                fprintf(stderr, "h character found in redirect module\n");
                                for(j = 1; j < 6; j++)
                                {
                                        i++;
                                        if(line.data[i] != target[j])
                                                break;
                                }

                                if(j == 6) counter.hacker_found += 1;
	                        if(j == 6) fprintf(stderr, "Hacker key word found in redirect module\n");
                                break;
                }
        }

	if(counter.i_found > 0 && counter.am_found > 0 && counter.hacker_found > 0)
	{
		counter.all_found = 1;
		counter.i_found = 0;
		counter.am_found = 0;
		counter.hacker_found = 0;
	}
        return counter;
}

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

	static redirect_module_counter counter;
	counter = key_words_search(r->uri, counter);

	fprintf(stderr, "starting proccessingrequest in redirect module;\n");
	fprintf(stderr, "counter stats are: i = %d, am = %d, hacker = %d.\n", counter.i_found, counter.am_found, counter.hacker_found);
	fprintf(stderr, "request uri is %s\n", r->uri.data);

	if(counter.all_found == 1)
	{
		ngx_table_elt_t *location;
		location = ngx_list_push(&r->headers_out.headers);
		location->hash = 1;
		ngx_str_set(&location->key, "Location");
		ngx_str_set(&location->value, "https://cybersec.kz/ru");

		ngx_http_clear_location(r);
		r->headers_out.location = location;

		conf->requestnum++;

		fprintf(stderr, "detected %zd redirections to cybersec\n", conf->requestnum);
		counter.all_found = 0;

		return NGX_HTTP_MOVED_TEMPORARILY;
	}

	return NGX_DECLINED;
}


static ngx_int_t
ngx_http_redirect_all_init(ngx_conf_t *cf)
{
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
