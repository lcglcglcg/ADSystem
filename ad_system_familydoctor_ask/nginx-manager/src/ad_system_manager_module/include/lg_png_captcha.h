
#ifndef	__LG_PNG_CAPTCHA_H__
#define	__LG_PNG_CAPTCHA_H__

#ifdef __cplusplus
extern "C" { 
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "gd.h"
#include "gdfontl.h"

    typedef struct {

	int size;
	char *buffer;
	char result[64];

    }lg_png_captcha_t;

    extern int lg_png_captcha_create(lg_png_captcha_t *img);

#ifdef __cplusplus
}
#endif

#endif
