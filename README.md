# NGINX HTTP Test Module

A simple nginx module which redirects unwanted requests to the given website. 
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
Following version solved the previous problems with body & regex. Currently the request_body proccessing is supported only in the POST requests. Moreover, if the words are not found request code 403 is returned. It was made to simulate the response of the local machine module was built on. However for any other normal web site should be redone.

The working prototype with self-made search algorithm can be found at https://github.com/doughnuty/nginx_module/blob/e4614559e804dce9e885574e125cf8a580d66f91/ngx_http_redirect_all_module/ngx_http_redirect_all_module.c .
Although it does not support body proccessing and requires a shell file to enable hackers_count location, it's good at header & uri processing and likely works properly.

## Changes
* PCRE regex updated and works properly.
* Body proccessing added and supports only POST request method. *QUESTION: should I finalize with NGX_DONE or I can somehow DECLINE handler. If not, how to manage all the internal redirects (ex. 404) or they will be managed automaticly? - has not been answered yet* 
* hackers_count location works properly

## More to add
* Logging in separate file, most probably without help of fprintf :)

