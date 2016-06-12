/*
 * $Id: printbuf.c,v 1.5 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "json_bits.h"
#include "json_print.h"

struct printbuf* printbuf_new(void)
{
    struct printbuf *p;

    p = (struct printbuf*)calloc(1, sizeof(struct printbuf));
    if(!p) return NULL;
    p->size = 32;
    p->bpos = 0;
    if(!(p->buf = (char*)malloc(p->size))) {
	free(p);
	return NULL;
    }
    return p;
}

int printbuf_memappend(struct printbuf *p, const char *buf, int size)
{
    char *t;
    if(p->size - p->bpos <= size) {
	int new_size = json_max(p->size * 2, p->bpos + size + 8);
	if(!(t = (char*)realloc(p->buf, new_size))) return -1;
	p->size = new_size;
	p->buf = t;
    }
    memcpy(p->buf + p->bpos, buf, size);
    p->bpos += size;
    p->buf[p->bpos]= '\0';
    return size;
}

int sprintbuf(struct printbuf *p, const char *msg, ...)
{
    va_list ap;
    int size;
    char buf[128];

    va_start(ap, msg);
    size = vsnprintf(buf, 128, msg, ap);
    va_end(ap);
    if(size == -1 || size > 127) {
	va_start(ap, msg);
	va_end(ap);
	return size;
    } else {
	printbuf_memappend(p, buf, size);
	return size;
    }
}

void printbuf_reset(struct printbuf *p)
{
    p->buf[0] = '\0';
    p->bpos = 0;
}

void printbuf_free(struct printbuf *p)
{
    if(p) {
	free(p->buf);
	free(p);
    }
}
