
#ifndef	__IMG_LIBGD_CAPTCHA_H__
#define	__IMG_LIBGD_CAPTCHA_H__

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	int size;
	char *buffer;
	char result[64];

    } img_libgd_captcha_t;

    extern int get_img_libgd_captcha(img_libgd_captcha_t *img);

#ifdef __cplusplus
}
#endif

#endif
