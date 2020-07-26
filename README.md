# NGINX HTTP Test Module

A simple nginx module which `<ideally have to>` redirect unwanted requests to the given website. 

## Synopsis
```nginx
 location / {
 security_task_if on;
 root html;
 }
 
 location /security {
    try_files $uri /var/log/nginx/security.log;
    access_log /var/log/nginx/security.log;
    return 301 http://cybersec.kz/;
 }
 ```

* Module type - header & body filter;
* Algorithm - ngx_str_t parts of the request are being passed to the key_words function and evaluated. 
If all three key words are met (*"i", "am", "hacker"*) the request is being internally redirected with the help of `ngx_http_internal_redirect()` function to the */security* location, 
which logged to the special security.log file (*path /var/log/nginx/security.log*) and returns 301, readdressing to *cybersec.kz*.

## Installation
```shell
    cd nginx-**version**
    ./configure --add-module=/path/to/this/directory
    make
    make install
```
## Sad truth
Currently (as for 8.24am 26.07) does not work at all. Neither activates, nor interrupts web server's work. Introduced debuging tools to search for the ngx_log_debug() 
function output but no useful outcome has been found. Moreover, even if the function worked it could handle search in the uri only due to the fact that no other components were introduced.
Possible fail reasons: 
  1. Misuse of `ngx_http_internal_redirect()`;
  2. Badly installed;
  3. Badly configured in `nginx.conf`;
  4. Misgenerated the response;
  5. Main algorithm does not work properly and therefore unable to detect words;
  6. URI sent to the function is not the same with the request URI;
  7. Thousands of unknown for me ascpects of NGINX which can be the case.
  
`I hope I will be able to finish this project and figure out the reason for its lack of action.`
