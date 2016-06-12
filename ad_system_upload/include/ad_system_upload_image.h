
#ifndef	__AD_SYSTEM_UPLOAD_IMG_H__
#define	__AD_SYSTEM_UPLOAD_IMG_H__

#ifdef __cplusplus
extern "C" { 
#endif

#define UPLOAD_IMG_FILE_PATH "/ad_system/images"

    typedef struct {

	size_t packet_size;
	char boundary[128];
	char file_name[4096];
	size_t boundary_size;
	size_t Content_Length;

	char *image_buffer;
	size_t image_size;
	int image_width;
	int image_height;
	char path[1024];

    } ad_system_upload_image_t;

#ifdef __cplusplus
}
#endif

#endif


