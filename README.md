# NGINX HTTP Test Module

A simple nginx module which `<ideally have to>` redirect unwanted requests to the given website. 
Last version can be found in redirect_all directory, but the final one will be renamed and moved to the security task module

## Synopsis
```nginx
 load_module modules/ngx_http_redirect_all_module.so;
 
 ...
 
 location / {
 redirect_all on;
 root html;
 }
 ```

* Module type - request handler;
* Algorithm - Using pcre regex, search the contents of the request for the key words (*"i", "am", "hacker"*) 
If all three key words are met, the request is being moved temporarily to the *http://cybersec.kz/*.
The fact of the redirection is saved into module configuration struct and the number of redirects displayed on the /hackers_count location.

## Installation
```shell
    cd nginx-**version**
    ./configure --add-dynamic-module=/path/to/this/directory
    make
    make install
    sudo cp objs/ngx_http_redirect_all_module.so /etc/nginx/modules/
```
## Overview 
Following version is **not** tested and therefore requires tons of debugging and improvements, but I think the architecture is final and (i hope) will not be hugely modified. 

The working prototype with self-made search algorithm can be found at https://github.com/doughnuty/nginx_module/blob/e4614559e804dce9e885574e125cf8a580d66f91/ngx_http_redirect_all_module/ngx_http_redirect_all_module.c .
Although it does not support body proccessing and requires a shell file to enable hackers_count location, it's good at header & uri processing and likely works properly.

As for the following commit (in redirect_all directory), it has several changes in multiple aspects which are listed bellow

## Changes
* Self-made algorithm completely removed and changed to PCRE regex. Though latest test has shown that script_exec does not produce required output, returning -1 (NGX_REGEX_NO_MATCHED) when the match has clearly occured. Most probably typo in the ngx_regex_compile related proccess.
* Body proccessing added but has not been tested yet, may need to add memcpy functions. Also not sure about the request finalization process in case if no matches were found. *QUESTION: should I finalize with NGX_DONE or I can somehow DECLINE handler. If not, how to manage all the internal redirects (ex. 404) or they will be managed automaticly?* 
* If the request uri is hackers_count create a response, body of which will tell how much redirects have been made, and return status code 200 (NGX_HTTP_OK). 
* Code refactoring to make the module look more like a module (compared to the previous versions) and some comments to ease the navigation

## More to add
* Logging in separate file, most probably without help of fprintf :)
* Debug existing code

