#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t         enable;
} ngx_http_redirect_all_conf_t;

// regex compile structure. I know its better to avoid global vars but have no idea how to implement this differently
ngx_regex_compile_t rc;
ngx_int_t requestnum = 0;

// functions declaration
ngx_module_t ngx_http_redirect_all_module;
static ngx_int_t redirect_all_module_send_requestnum(ngx_http_request_t *r, ngx_int_t requestnum);
static ngx_int_t create_redirect_to_location(ngx_http_request_t *r);
void ngx_http_redirect_all_body_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_redirect_all_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_redirect_all_init(ngx_conf_t *cf);
static void * ngx_http_redirect_all_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_redirect_all_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);


static ngx_command_t  ngx_http_redirect_all_commands[] = {

    { ngx_string("redirect_all"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redirect_all_conf_t, enable),
      NULL },

      ngx_null_command
};

static ngx_http_module_t ngx_http_redirect_all_module_ctx = {
  NULL,                                 /* preconfiguration */
  ngx_http_redirect_all_init,           /* postconfiguration */

  NULL,                                 /* create main configuration */
  NULL,                                 /* init main configuration */
  NULL,                                 /* create server configuration */
  NULL,                                 /* merge server configuration */
  ngx_http_redirect_all_create_loc_conf, /* create location configuration */
  ngx_http_redirect_all_merge_loc_conf   /* merge location configuration */
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

static ngx_int_t
ngx_http_redirect_all_handler(ngx_http_request_t *r)
{
        ngx_http_redirect_all_conf_t     *conf;
        ngx_list_part_t                  *part;
        ngx_table_elt_t                *header;
        ngx_uint_t                 headers_num;
        ngx_uint_t                       i = 0;

        ngx_int_t                      matches;
        int    captures[(1 + rc.captures) * 3];

        // get module configuration
        conf = ngx_http_get_module_loc_conf(r, ngx_http_redirect_all_module);

        // quit if unabled
        if(!conf->enable) {
                return NGX_DECLINED;
        }

        // check if the uri is hackers_count

        // check which is shorter
        int min = ngx_strlen("/hackers_count");
        if(r->uri.len < ngx_strlen("/hackers_count"))
        {
                min = r->uri.len;
        }

        int cmp = ngx_strncmp(r->uri.data, "/hackers_count", min);
	
        // if it is, call the function which will create body with the requestnum and finalize request
        if (cmp == 0  && min > 1)
        {
                return redirect_all_module_send_requestnum(r, requestnum);
        }

        // search in uri using regex

        matches = ngx_regex_exec(rc.regex, &r->uri, captures, (1 + rc.captures) * 3);
        if(matches >= 0)
        {
                requestnum++;
                return create_redirect_to_location(r);
        }

        // search in headers if didn't find in uri
        part = &r->headers_in.headers.part;

        while(part != NULL && matches <= NGX_REGEX_NO_MATCHED)
        {
                headers_num = part->nelts;
                header = part->elts;

                // loop through headers
                for(i = 0; i < headers_num; i++)
                {
                        // search in header, ugly, needs improvement
                        matches = ngx_regex_exec(rc.regex, &header[i].key, captures, (1 + rc.captures) * 3);
                        if (matches >= 0)
                        {
                                requestnum++;
                                break;
                        }
                        matches = ngx_regex_exec(rc.regex, &header[i].value, captures, (1 + rc.captures) * 3);
                        if (matches >= 0)
                        {
                                requestnum++;
                                break;
                        }
                }
                part = part->next;
        }

        // if the request has body, process it and search for the key words inside
        if (r->method == NGX_HTTP_POST)
        {
                ngx_int_t  read_body;
                // access the body
                read_body = ngx_http_read_client_request_body(r, ngx_http_redirect_all_body_handler);

                if (read_body >= NGX_HTTP_SPECIAL_RESPONSE) {
                     return read_body;
                }

                // REDO: otherwise finish the request (need to redo so everything will work properly if no matches found)
                return NGX_DONE;
        }

        // if no body continue with what you have (headers result)
        if(matches >= 0)
        {
                return create_redirect_to_location(r);
        }

        // no matches exit handler
        else if (matches != NGX_REGEX_NO_MATCHED)
        {
                fprintf(stderr, "Regex error\n");
        }

        return NGX_DECLINED;
}

// function to handle the body
void
ngx_http_redirect_all_body_handler(ngx_http_request_t *r)
{
        ngx_chain_t       *in;
        u_char           *tmp;
        ngx_int_t     matches;
        int    captures[(1 + rc.captures) * 3];

        // loop through body;
        for(in = r->request_body->bufs; in; in = in->next)
        {
                // check the buf contents
                tmp = in->buf->pos;
                ngx_str_t temp_string = ngx_string(tmp);
                temp_string.len = in->buf->last - in->buf->pos + 1;
                fprintf(stderr, "key words are searched in the bodypart: %s. The length of the string is %zd\n", temp_string.data, temp_string.len);
                matches = ngx_regex_exec(rc.regex, &temp_string, captures, (1 + rc.captures) * 3);
                fprintf(stderr, "regex output is %zd\n", matches);
                if(matches >= 0)
                {
                        // if found redirect
                        requestnum++;
                        ngx_int_t rcode;
                        rcode = create_redirect_to_location(r);
                        ngx_http_finalize_request(r, rcode);
                        return;

                }
        }

        // if key words not found - forbidden
	// NOTE: this is not the best (and not proper) way to do it, but I have no idea how it should work
	// will edit later
        ngx_http_finalize_request(r, NGX_HTTP_FORBIDDEN);
}


// function to create response if location is hackers_count
static ngx_int_t
redirect_all_module_send_requestnum(ngx_http_request_t *r, ngx_int_t requestnum)
{
    ngx_int_t   rcode;
    ngx_buf_t      *b;
    ngx_chain_t   out;

    b = ngx_create_temp_buf(r->pool, NGX_OFF_T_LEN);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->last = ngx_sprintf(b->pos, "%O", requestnum);
    b->last_buf = (r == r->main) ? 1: 0;
    b->last_in_chain = 1;

    // send the header
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    rcode = ngx_http_send_header(r);

    if (rcode == NGX_ERROR || rcode > NGX_OK || r->header_only) {
        return rcode;
    }

    // send the number in body

    out.buf = b;
    out.next = NULL;

    ngx_http_output_filter(r, &out);
    return NGX_DONE;
}

// function to redirect if words matched
static ngx_int_t
create_redirect_to_location(ngx_http_request_t *r)
{
        ngx_table_elt_t *location;
        location = ngx_list_push(&r->headers_out.headers);
        location->hash = 1;
        ngx_str_set(&location->key, "Location");
        ngx_str_set(&location->value, "https://cybersec.kz/ru");

        ngx_http_clear_location(r);
        r->headers_out.location = location;

        return NGX_HTTP_MOVED_TEMPORARILY;
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

static ngx_int_t
ngx_http_redirect_all_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;



     #if (NGX_PCRE)
     u_char                errstr[NGX_MAX_CONF_ERRSTR];

     ngx_str_t  value = ngx_string(".*i.*am.*hacker.*");

     ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

     rc.pattern = value;
     rc.options = NGX_REGEX_CASELESS;
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

    *h = ngx_http_redirect_all_handler;

    return NGX_OK;
}
