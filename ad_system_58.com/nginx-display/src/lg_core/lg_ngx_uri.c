
#include "lg_string.h"
#include "lg_ngx_session.h"
#include "lg_ngx_network.h"
#include "lg_ngx_uri.h"

int lg_ngx_uri_key_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    char *s = (char *)r->uri_start;
    char *e = (char *)r->uri_end;

    uri->key = (char *)s;
    uri->key_size = 0;

    if ((s == e) || (*s == '/' && (s + 1) == e)) { 

	uri->key = "/";
	uri->key_size = 1;
	return 0;
    }

    char *str = NULL;
    for (str = s; str <= e; str++) {

	if (*str == ' ' || *str == '?')
	    break;

	if (*str == '/' || *str == '.' || *str == '_' || *str == '-' || isalnum(*str))
	    uri->key_size++;
	else
	    return -1;
    }

    if (uri->key_size <= 0)
	return -1;

    uri->key[uri->key_size] = 0;
    return 0;
}

int lg_ngx_uri_domain_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    ngx_http_headers_in_t *headers_in = &r->headers_in;
    if (!headers_in || headers_in->server.len == 0)
	return -1;

    uri->domain = (char *)headers_in->server.data;
    uri->domain_size = headers_in->server.len;
    if (uri->domain_size <= 0)
	return -1;

    return 0;
}

void lg_ngx_uri_mobile_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    uri->mobile_type = LG_NGX_URI_TYPE_URL;
    char *user_agent = lg_ngx_network_get_user_agent(r);
    if (user_agent) {

	if (
		lg_string_strstr(user_agent, "ipad")
		|| lg_string_strstr(user_agent, "iphone")
		|| lg_string_strstr(user_agent, "android")
		|| lg_string_strstr(user_agent, "windows ce")
		|| lg_string_strstr(user_agent, "windows mobile")
	   ) {

	    uri->mobile_type = LG_NGX_URI_TYPE_MOBILE_URL;
	}
    }
}

int lg_ngx_uri_content_type_root_parse(char *key)
{

    char *str = NULL;
    for (str = key; *str; str++)
	if (*str == '.')
	    return -1;

    return 0;
}

void lg_ngx_uri_content_type_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    char *key = uri->key;
    int size = uri->key_size;
    if (!key)
	uri->type = LG_NGX_URI_TYPE_HTML;

    else if (lg_ngx_uri_content_type_root_parse(key) == 0)
	uri->type = LG_NGX_URI_TYPE_ROOT;

    else if (size <= 5) //防止越界
	uri->type = LG_NGX_URI_TYPE_HTML;

    else if (strncmp(&key[size - 4], ".htm", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_HTML;

    else if (strncmp(&key[size - 5], ".html", 5) == 0)
	uri->type = LG_NGX_URI_TYPE_HTML;

    else if (strncmp(&key[size - 4], ".css", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_CSS;

    else if (strncmp(&key[size - 3], ".js", 3) == 0)
	uri->type = LG_NGX_URI_TYPE_JS;

    else if (strncmp(&key[size - 4], ".gif", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_GIF;

    else if (strncmp(&key[size - 4], ".ico", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_ICO;

    else if (strncmp(&key[size - 4], ".jpe", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_JPEG;

    else if (strncmp(&key[size - 4], ".jpg", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_JPEG;

    else if (strncmp(&key[size - 5], ".jpeg", 5) == 0)
	uri->type = LG_NGX_URI_TYPE_JPEG;

    else if (strncmp(&key[size - 4], ".png", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_PNG;

    else if (strncmp(&key[size - 4], ".bmp", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_BMP;

    else if (strncmp(&key[size - 4], ".img", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_IMG;

    else if (strncmp(&key[size - 4], ".txt", 4) == 0)
	uri->type = LG_NGX_URI_TYPE_TXT;

    else
	uri->type = LG_NGX_URI_TYPE_HTML;
}

void lg_ngx_uri_affiliated_file_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    char *str = NULL;
    uri->affiliated_file = uri->key;
    for (str = uri->key; *str; str++) {

	if (*str == '/') uri->affiliated_file = str;
    }
}

void lg_ngx_uri_content_type_to_network(lg_ngx_uri_t *uri, lg_ngx_network_t *network)
{

    switch(uri->type) {

	case LG_NGX_URI_TYPE_HTML:
	    network->content_type.data = LG_NGX_URI_HTML_STR;
	    network->content_type.len = LG_NGX_URI_HTML_SIZE;
	    break;

	case LG_NGX_URI_TYPE_CSS:
	    network->content_type.data = LG_NGX_URI_CSS_STR;
	    network->content_type.len = LG_NGX_URI_CSS_SIZE;
	    break;

	case LG_NGX_URI_TYPE_JS:
	    network->content_type.data = LG_NGX_URI_JS_STR;
	    network->content_type.len = LG_NGX_URI_JS_SIZE;
	    break;

	case LG_NGX_URI_TYPE_GIF:
	    network->content_type.data = LG_NGX_URI_GIF_STR;
	    network->content_type.len = LG_NGX_URI_GIF_SIZE;
	    break;

	case LG_NGX_URI_TYPE_ICO:
	    network->content_type.data = LG_NGX_URI_ICO_STR;
	    network->content_type.len = LG_NGX_URI_ICO_SIZE;
	    break;

	case LG_NGX_URI_TYPE_JPEG:
	    network->content_type.data = LG_NGX_URI_JPEG_STR;
	    network->content_type.len = LG_NGX_URI_JPEG_SIZE;
	    break;

	case LG_NGX_URI_TYPE_PNG:
	    network->content_type.data = LG_NGX_URI_PNG_STR;
	    network->content_type.len = LG_NGX_URI_PNG_SIZE;
	    break;

	case LG_NGX_URI_TYPE_BMP:
	    network->content_type.data = LG_NGX_URI_BMP_STR;
	    network->content_type.len = LG_NGX_URI_BMP_SIZE;
	    break;

	case LG_NGX_URI_TYPE_IMG:
	    network->content_type.data = LG_NGX_URI_IMG_STR;
	    network->content_type.len = LG_NGX_URI_IMG_SIZE;
	    break;

	case LG_NGX_URI_TYPE_TXT:
	    network->content_type.data = LG_NGX_URI_TXT_STR;
	    network->content_type.len = LG_NGX_URI_TXT_SIZE;
	    break;

	default:
	    network->content_type.data = LG_NGX_URI_HTML_STR;
	    network->content_type.len = LG_NGX_URI_HTML_SIZE;
    }
}

int lg_ngx_uri_parse(ngx_http_request_t *r, lg_ngx_uri_t *uri)
{

    if (lg_ngx_uri_key_parse(r, uri))
	return -1;

    // if (lg_ngx_uri_domain_parse(r, uri))
    // 	return -1;

    // lg_ngx_uri_mobile_parse(r, uri);
    lg_ngx_uri_content_type_parse(r, uri);
    lg_ngx_uri_affiliated_file_parse(r, uri);

    // fprintf(stdout, "1-->[%s]\n", uri->domain);
    // fprintf(stdout, "2-->[%s]\n", uri->key);
    // fprintf(stdout, "3-->[%s]\n", uri->affiliated_file);
    return 0;
}


