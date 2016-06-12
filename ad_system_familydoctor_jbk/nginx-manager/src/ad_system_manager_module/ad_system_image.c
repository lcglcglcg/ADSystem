
#include <stdio.h>
#include <stdlib.h>
#include "ad_system_report.h"

int ad_system_image_copy(char *path_dest, char *path_src)
{

    FILE *fp_r = fopen(path_src, "rb");
    if (!fp_r) return -1;

    FILE *fp_w = fopen(path_dest, "wb");
    if (!fp_w) {

	fclose(fp_r);
	return -1;
    }

    size_t size = 0;
    char buffer[4096] = {0};
    while((size = fread(buffer, 1, 4096, fp_r)))
	fwrite(buffer, 1, size, fp_w);

    fclose(fp_r);
    fclose(fp_w);
    return 0;
}

int ad_system_hospital_image_copy(const char *account_id, const char *hospital_img)
{

    char path_src[1024];
    char path_dest[1024];

    fprintf(stdout, "%s\n", path_src);
    fprintf(stdout, "%s\n", path_dest);

    sprintf(path_src, "%s%s", REPORT_FILE_PATH, hospital_img);
    sprintf(path_dest, "%s/HospitalImgFile/%s.JPG", REPORT_FILE_PATH, account_id);
    return ad_system_image_copy(path_dest, path_src);
}


