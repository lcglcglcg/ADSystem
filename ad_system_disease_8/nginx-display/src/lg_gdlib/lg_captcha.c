
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <gd.h>
#include "lg_captcha.h"

#define MIN(a,b) ((a)>(b)?(b):(a))

struct lg_captcha_font_t {

    int x, y, c;
    gdImagePtr i;

} lg_captcha_font[36];

int lg_captcha_init(char *symbols)
{
    int i, brect[8];
    gdFTStringExtra strex;
    memset(&strex, 0, sizeof(gdFTStringExtra));

    srand(time(NULL));
    if (!symbols) symbols = "1234567890abcdefghijklmnopqrstuvwxyz";
    int symbols_len = strlen(symbols);

    for (i = 0; i < symbols_len; i++) {

	if (!isalpha(symbols[i]) && !isdigit(symbols[i]))
	    return -1;

	char str[2];
	sprintf(str, "%c", symbols[i]);
	if (gdImageStringFT(NULL, &brect[0], 0, "/make/nginx-1.8.1/src/lg_gdlib/lg_captcha.ttf", 30, 0, 0, 0, str))
	    return -1;

	int x = brect[2] - brect[6];
	int y = brect[3] - brect[7];

	lg_captcha_font[i].c  = symbols[i];
	lg_captcha_font[i].x  = x;
	lg_captcha_font[i].y  = y;

	gdImagePtr im = gdImageCreateTrueColor(x, y);
	gdImageAlphaBlending(im, 0);
	gdImageSaveAlpha(im, 1);

	int ft_color = gdImageColorAllocate(im, 0, 0, 0);
	int trans = gdImageColorAllocateAlpha(im, 255, 255, 255, 255);

	gdImageFilledRectangle(im, 0, 0, 74, 74, trans);

	x = 0 - brect[6];
	y = 0 - brect[7];
	if (gdImageStringFTEx(im, brect, ft_color, "/make/nginx-1.8.1/src/lg_gdlib/lg_captcha.ttf", 30, 0.0, x, y, str, &strex))
	    return -1;

	lg_captcha_font[i].i = im;
    }

    return 0;
}

int lg_captcha_create(lg_captcha_t *p)
{
    int i, j;
    gdImagePtr img, imo;

    img = gdImageCreateTrueColor(120, 60);
    gdImageAlphaBlending(img, 1);

    int white = gdImageColorAllocate(img, 255, 255, 255);

    gdImageFilledRectangle(img, 0, 0, 120 - 1, 60 - 1, white);

    int x = 1, y = 1, shift = 0, sx, sy;
    int rgb, opacity, left, px, py;
    int odd = rand()%2;
    if (odd == 0) odd =-1;
    float color, color_x, color_y, color_xy;

    for (i = 0; i < 4; i++) {

	j = rand() % 36;

	y = ((i%2) * 8 - 8/2) * odd
	    + (rand() % ((int)(8*0.33 + 0.5) * 2 + 1) - ((int)(8*0.33 + 0.5)))
	    + (60 - 30)/2;

	shift = 0;

	if (i > 0) {

	    shift=10000;
	    for (sy = 1; sy < lg_captcha_font[j].y; sy++) {
		for (sx = 1; sx < lg_captcha_font[j].x; sx++) {
		    rgb = gdImageTrueColorPixel(lg_captcha_font[j].i, sx, sy);
		    opacity = rgb>>24;
		    if (opacity < 127) {
			left = sx - 1 + x;
			py = sy + y;
			if (py > 60) break;
			for (px = MIN(left, 120-1); px > left-200 && px >= 0; px -= 1) {
			    color = gdImageTrueColorPixel(img, px, py) & 0xff;
			    if (color + opacity<170) { // 170 - threshold
				if (shift > left - px) {
				    shift = left - px;
				}
				break;
			    }
			}
			break;
		    }
		}
	    }

	    if (shift == 10000){
		shift = rand() % 3 + 4;
	    }
	}

	gdImageCopy(img, lg_captcha_font[j].i, x - shift, y, 0, 0, lg_captcha_font[j].x, lg_captcha_font[j].y);

	x += lg_captcha_font[j].x - shift;
	p->key[i] = lg_captcha_font[j].c;
    }

    int center = x/2;

    // periods
    float rand1 = (rand() % 45 + 75)/1000.0;
    float rand2 = (rand() % 45 + 75)/1000.0;
    float rand3 = (rand() % 45 + 75)/1000.0;
    float rand4 = (rand() % 45 + 75)/1000.0;
    // phases
    float rand5 = (rand() % 314)/100.0;
    float rand6 = (rand() % 314)/100.0;
    float rand7 = (rand() % 314)/100.0;
    float rand8 = (rand() % 314)/100.0;
    // amplitudes
    float rand9 = (rand() %  90 + 330)/110.0;
    float rand10= (rand() % 120 + 330)/100.0;

    float newr, newg, newb;
    float frsx, frsy, frsx1, frsy1;
    float newcolor, newcolor0;

    imo = gdImageCreateTrueColor(120, 60);
    gdImageAlphaBlending(imo, 1);
    int bg = gdImageColorAllocate(imo, 255, 255, 255);
    gdImageFilledRectangle(imo, 0, 0, 120 - 1, 60 - 1, bg);

    for (x = 0; x < 120; x++) {
	for (y = 0; y < 60; y++) {

	    sx = x +(sin(x*rand1+rand5)+sin(y*rand3+rand6))*rand9-120/2+center+1;
	    sy = y +(sin(x*rand2+rand7)+sin(y*rand4+rand8))*rand10;

	    if (sx<0 || sy<0 || sx>=120-1 || sy>=60-1){
		continue;
	    } else {
		color   = gdImageTrueColorPixel(img, sx, sy)    & 0xFF;
		color_x = gdImageTrueColorPixel(img, sx+1, sy)  & 0xFF;
		color_y = gdImageTrueColorPixel(img, sx, sy+1)  & 0xFF;
		color_xy= gdImageTrueColorPixel(img, sx+1, sy+1)& 0xFF;
	    }

	    if (color==255 && color_x==255 && color_y==255 && color_xy==255){
		continue;
	    } else if(color==0 && color_x==0 && color_y==0 && color_xy==0){
		newr = 0;
		newg = 0;
		newb = 0;
	    } else {
		frsx  = sx - floor(sx);
		frsy  = sy - floor(sy);
		frsx1 = 1 - frsx;
		frsy1 = 1 - frsy;

		newcolor = ( (color * frsx1 * frsy1) + (color_x * frsx * frsy1)
			+ (color_y * frsx1 * frsy) + (color_xy * frsx * frsy) );

		if (newcolor > 255) newcolor = 255;
		newcolor = newcolor/255;
		newcolor0 = 1 - newcolor;

		newr  = newcolor0 + newcolor * 255;
		newg  = newcolor0 + newcolor * 255;
		newb  = newcolor0 + newcolor * 255;
	    }

	    gdImageSetPixel(imo, x, y, gdImageColorAllocate(imo, newr, newg, newb));
	}
    }

    p->buffer = (char *) gdImagePngPtr(imo, &p->size);
    if (!p->buffer)
	return -1;

    //free(imodata);
    gdImageDestroy(img);
    gdImageDestroy(imo);
    p->key[4] = 0;
    return 0;
}


