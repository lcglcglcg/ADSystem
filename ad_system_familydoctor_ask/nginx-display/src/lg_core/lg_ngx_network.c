
#include "lg_ngx_network.h"

void lg_ngx_network_recv_print(char *buffer)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    fprintf(stdout, "[send][%04d-%02d-%02d %02d:%02d:%02d]",
	    tm.tm_year + 1900,
	    tm.tm_mon + 1,
	    tm.tm_mday,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec);
    fprintf(stdout, "%s\n", buffer);
}

void lg_ngx_network_send_print(ngx_chain_t *buffer)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);

    ngx_chain_t *p = NULL;
    for (p = buffer; p; p = p->next) {

	fprintf(stdout, "[send][%04d-%02d-%02d %02d:%02d:%02d]",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec);

	fwrite(p->buf->pos, (p->buf->last - p->buf->pos), 1, stdout);
	fprintf(stdout, "\n");
    }
}

int lg_ngx_network_send(ngx_http_request_t *r, lg_ngx_network_t *p)
{
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_type = p->content_type;
    r->headers_out.content_length_n = p->content_length;

    int rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	return rc;

    // lg_ngx_network_send_print(p->head);
    return ngx_http_output_filter(r, p->head);
}

void lg_ngx_network_post_send(ngx_http_request_t *r, lg_ngx_network_t *p)
{
    int rc = lg_ngx_network_send(r, p);
    return ngx_http_finalize_request(r, rc); 
}

ngx_chain_t *lg_ngx_network_buffer_to_chain(ngx_http_request_t *r, const char *p_buffer, size_t size)
{

    ngx_chain_t *buffer = NULL;
    if (!(buffer = ngx_pcalloc(r->pool, sizeof(ngx_chain_t))))
	return NULL;

    buffer->buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    buffer->next = NULL;

    buffer->buf->pos = ngx_pcalloc(r->pool, size);
    if (!buffer->buf->pos)
	return NULL;

    buffer->buf->last = buffer->buf->pos + size;
    ngx_memcpy(buffer->buf->pos, p_buffer, size);

    buffer->buf->memory = 1;
    buffer->buf->last_buf = 1;
    return buffer;
}

int lg_ngx_network_buffer_add(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size)
{

    ngx_chain_t *buffer = lg_ngx_network_buffer_to_chain(r, p_buffer, size);
    if (!buffer) return -1;

    if (!network->head) {

	network->head = buffer;
	network->head->next = NULL;

    } else if (!network->next) {

	network->head->buf->last_buf = 0;
	network->head->next = network->next = buffer;

    } else {

	network->next->buf->last_buf = 0;
	network->next->next = buffer;
	network->next = buffer;
    }

    network->content_length += size;
    return 0;
}

int lg_ngx_network_buffer_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size)
{
    lg_ngx_network_t *p = network;
    if (!network) {

	p = ngx_pcalloc(r->pool, sizeof(lg_ngx_network_t));
	memset(p, 0, sizeof(lg_ngx_network_t));
    }

    if (lg_ngx_network_buffer_add(r, p, p_buffer, size))
	return -1;

    return lg_ngx_network_send(r, p);
}

int lg_ngx_network_str_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *str)
{
    return lg_ngx_network_buffer_send(r, network, str, strlen(str));
}

void lg_ngx_network_buffer_post_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer, size_t size)
{
    int rc = lg_ngx_network_buffer_send(r, network, p_buffer, size);
    return ngx_http_finalize_request(r, rc); 
}

void lg_ngx_network_str_post_send(ngx_http_request_t *r, lg_ngx_network_t *network, const char *str)
{
    return lg_ngx_network_buffer_post_send(r, network, str, strlen(str));
}

int lg_ngx_network_buffer_send_str(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer)
{

    if (p_buffer)
	return lg_ngx_network_buffer_send(r, NULL, p_buffer, strlen(p_buffer));

    return lg_ngx_network_not_found(r);
}

void lg_ngx_network_buffer_post_send_str(ngx_http_request_t *r, lg_ngx_network_t *network, const char *p_buffer)
{
    if (p_buffer)
	return lg_ngx_network_buffer_post_send(r, NULL, p_buffer, strlen(p_buffer));

    return lg_ngx_network_post_not_found(r);
}

int lg_ngx_network_not_found(ngx_http_request_t *r)
{

    r->headers_out.status = NGX_HTTP_NOT_FOUND;
    r->headers_out.content_length_n = 13;
    int rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	return rc;

    ngx_chain_t *buffer = lg_ngx_network_buffer_to_chain(r, "404 Not Found", 13);
    if (!buffer) return NGX_ERROR;

    return ngx_http_output_filter(r, buffer);
}

void lg_ngx_network_post_not_found(ngx_http_request_t *r)
{

    int rc = lg_ngx_network_not_found(r);
    return ngx_http_finalize_request(r, rc); 
}

ngx_str_t *lg_ngx_network_get_post_buffer(ngx_http_request_t *r)
{

    if (!r->headers_in.content_length)
	return NULL;

    if (!r->headers_in.content_length->value.data || r->headers_in.content_length->value.len <= 0)
	return NULL;

    size_t content_length = atoi((char *)r->headers_in.content_length->value.data);
    if (content_length <= 0) return NULL;

    ngx_str_t *str = ngx_palloc(r->pool, sizeof(ngx_str_t));
    if (!str) return NULL;

    char *buffer = ngx_palloc(r->pool, content_length + 1);
    if (!buffer) return NULL;

    size_t size = 0;
    ngx_chain_t *p = NULL;
    for (p = r->request_body->bufs; p; p = p->next) {

	ngx_buf_t *node_buffer = p->buf;
	memcpy(&buffer[size], node_buffer->pos, node_buffer->last - node_buffer->pos);
	size += node_buffer->last - node_buffer->pos;

	if (size > content_length)
	    return NULL;
    }

    if (content_length != size)
	return NULL;

    buffer[size] = 0;
    str->data = (void *)buffer;
    str->len = size;
    return str;
}

ngx_str_t *lg_ngx_network_post_upload_file_parse(ngx_http_request_t *r, ngx_str_t *p_str)
{

    if (!p_str || !p_str->data || !p_str->len)
	return NULL;

    // buffer
    char *buffer = (char *)p_str->data;
    buffer[p_str->len - 1] = 0; //防止越界

    // Content-Disposition
    char *Content_Disposition = strstr(buffer, "\r\n");
    if (!Content_Disposition) return NULL;
    *Content_Disposition = 0;	// key END
    Content_Disposition++;	// \r
    Content_Disposition++;	// \n

    // Content-Type
    char *Content_Type = strstr(Content_Disposition, "\r\n");
    if (!Content_Type) return NULL;
    *Content_Type = 0;		// Content_Disposition END
    Content_Type++;		// \r
    Content_Type++;		// \n

    // data
    char *data= (strstr(Content_Type, "\r\n\r\n"));
    if (!data) return NULL;
    *data = 0;			// Content_Type END
    data++;			// \r
    data++;			// \n
    data++;			// \r
    data++;			// \n

    // key
    char key[128];
    size_t key_size = sprintf(key, "\r\n%s", buffer);
    if (key_size < 32 || key_size > 120)
	return NULL;

    // data END
    size_t i, size = 0, max = p_str->len - (data - buffer);
    for (i = 0; i < max; i++) {

	if (strncmp(&data[i], key, key_size) == 0) {

	    size = i;
	    break;
	}
    }

    if (!size) return NULL;

    // ngx str
    ngx_str_t *str = ngx_palloc(r->pool, sizeof(ngx_str_t));
    if (!str) return NULL;

    str->data = (void *)data;
    str->len = size;

    // fprintf(stdout, "[key][%s]\n", key);
    // fprintf(stdout, "[Content_Disposition][%s]\n", Content_Disposition);
    // fprintf(stdout, "[Content_Type][%s]\n", Content_Type);
    // fprintf(stdout, "[data][%s]\n", data);
    return str;
}





