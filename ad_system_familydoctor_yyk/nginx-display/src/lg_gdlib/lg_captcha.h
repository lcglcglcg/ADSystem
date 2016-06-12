
#ifndef __LG_CAPTCHA_H__
#define __LG_CAPTCHA_H__


#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct lg_captcha_t {

	char key[5];
	char *buffer;
	int size;

    } lg_captcha_t;

    extern int lg_captcha_init(char *symbols);
    extern int lg_captcha_create(lg_captcha_t *p);


#ifdef __cplusplus
}
#endif
#endif


