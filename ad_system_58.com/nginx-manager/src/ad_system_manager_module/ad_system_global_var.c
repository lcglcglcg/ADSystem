
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ad_system_global_var.h"
#include "ad_system_region_code.h"

int lg_file_buffer_load(const char *path, char **buffer, size_t *size)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (*size == 0) {

	fclose(fp);
	return -1;
    }   

    if (!(*buffer = malloc(*size + 1))) {

	fclose(fp);
	return -1;
    }   

    if (fread(*buffer, 1, *size, fp) != *size) {

	fclose(fp);
	free(*buffer);
	return -1;
    }   

    fclose(fp);
    return 0;
}

int ad_system_global_var_init()
{

    if (ad_system_region_code_load("/ad_system/nginx-manager/region_code.csv")) {

	fprintf(stderr, "ad_system_region_code_load err\n");
	return -1;
    }

    srand(time(NULL));
    return lg_file_buffer_load("/ad_system/nginx-manager/region_code.json", &ad_system_global_region_code.buffer, &ad_system_global_region_code.size);
}


