#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t         enable;
    ngx_int_t          requestnum;
} ngx_http_redirect_all_conf_t;

// regex compile structure. I know its better to avoid global vars but have no idea how to implement this differently
ngx_regex_compile_t rc;

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
        if (cmp == 0 && min > 1)
        {
		fprintf(stderr, "The requestnum request (location hacker_count detetcted) on the uri %s\n", r->uri.data);
                return redirect_all_module_send_requestnum(r, conf->requestnum);
        }

        // search in uri using regex

        matches = ngx_regex_exec(rc.regex, &r->uri, captures, (1 + rc.captures) * 3);
        if(matches >= 0)
        {
                conf->requestnum++;
                return create_redirect_to_location(r);
        }
        // DISCARD for test only
        fprintf(stderr, "Regex output is %zd\n", matches);
        fprintf(stderr, "Regex value is %s\n", rc.pattern.data);
        // DISCARD for test only


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
                                break;
                        }
                        matches = ngx_regex_exec(rc.regex, &header[i].value, captures, (1 + rc.captures) * 3);
                        if (matches >= 0)
                        {
                                break;
                        }
                }
                part = part->next;
        }

        // if the request has body, process it and search for the key words inside
        if (r->request_body != NULL)
        {
                ngx_int_t  read_body;
                // access the body
                read_body = ngx_http_read_client_request_body(r, ngx_http_redirect_all_body_handler);

                if (read_body >= NGX_HTTP_SPECIAL_RESPONSE) {

                        // if request was redirected increase counter
                        if (read_body == NGX_HTTP_MOVED_TEMPORARILY)
                        {
                                conf->requestnum++;
                        }

                        return read_body;
                }

                // REDO: otherwise finish the request (need to redo so everything will work properly if no matches found)
                ngx_http_finalize_request(r, NGX_DONE);
                return NGX_DONE;
                // REDO: otherwise finish the request (need to redo so everything will work properly if no matches found)

        }

        // if no body continue with what you have (uri & headers result)
        if(matches >= 0)
        {
                return create_redirect_to_location(r);
        }

        // no matches exit handler
        else if (matches != NGX_REGEX_NO_MATCHED)
        {
                fprintf(stderr, "Regex error\n");
        }

        // DISCARD for test only
        fprintf(stderr, "Regex output is %zd\n", matches);
        // DISCARD for test only

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
                // loop through buf contents
                tmp = in->buf->pos;
                while(tmp <= in->buf->last)
                {
                        // check for matches with regex
                        ngx_str_t temp_string = ngx_string(tmp);
                        matches = ngx_regex_exec(rc.regex, &temp_string, captures, (1 + rc.captures) * 3);
                        if(matches >= 0)
                        {
                                // if found redirect
                                ngx_http_finalize_request(r, create_redirect_to_location(r));
                                return;
                        }
                        // else continue
                        tmp++;
                }
        }

        // if key words not found - DONE
        ngx_http_finalize_request(r, NGX_DONE);
}

// function to create response if location is hackers_count
static ngx_int_t
redirect_all_module_send_requestnum(ngx_http_request_t *r, ngx_int_t requestnum)
{
    ngx_int_t      rc;
    ngx_buf_t      *b;
    ngx_chain_t   out;

    // send the header
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = 40;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    // send the number in body

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->last_buf = (r == r->main) ? 1: 0;
    b->last_in_chain = 1;

    b->memory = 1;

    // create a string which will be sent
    char str[40];
    sprintf(str, "Number of redirects is equal to %zd\n", requestnum);

    b->pos = (u_char *) str;
    b->last = b->pos + 40;

    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
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

    // DISCARD for test only
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "detected 0 redirections to cybersec\n");

    *h = ngx_http_redirect_all_handler;

    return NGX_OK;
}
