
#ifndef __AD_SYSTEM_DISPLAY_DB_H__
#define __AD_SYSTEM_DISPLAY_DB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lg_db.h"
#include "lg_ocilib.h"

typedef struct {

    lg_db_t	*info_img;
    lg_list_t 	info_img_list;

    lg_db_t	*info_txt;
    lg_list_t	info_txt_list;

    // 剩余流量
    lg_list_t	info_default_img;

    lg_db_t	*dict_place;
    lg_db_t	*dict_region;
    lg_db_t	*dict_disease;

} ad_system_db_t;

typedef struct {

    lg_list_t next;

    double cpc_price;
    // double cpm_price;
    char img_size[64];

    char *buffer;
    size_t size;

} ad_system_info_t;

typedef struct {

    char key[64];
    char value[64];
    int type;

} ad_system_kv_t;

// init
extern void ad_system_db_init();
extern int ad_system_db_load_init();
extern void *ad_system_db_reload(void *p);
extern ad_system_db_t *ad_system_db_create();
extern void ad_system_db_free(ad_system_db_t *db);
extern char *get_region_code();

// add
extern int ad_system_db_element_add_list(ad_system_db_t *root_db, lg_db_t *db,
	int region_type,
	char *region_code_array,
	char *disease_code_array,
	ad_system_info_t *info);
extern int ad_system_db_element_add_kv(lg_db_t *db, char *key, char *value, int type);


// load
extern lg_db_t *ad_system_db_load_info_img(ad_system_db_t *root_db, lg_list_t *head, char *db_name);
extern lg_db_t *ad_system_db_load_info_txt(ad_system_db_t *root_db, lg_list_t *head, char *db_name);

extern lg_db_t *ad_system_db_load_dict_place();
extern lg_db_t *ad_system_db_load_dict_region();
extern lg_db_t *ad_system_db_load_dict_disease();

// find
extern ad_system_kv_t *ad_system_db_element_find_code(char *code);
extern ad_system_kv_t *ad_system_db_element_find_place(char *code);
extern lg_list_t *ad_system_db_element_find_img(char *region_code, char *disease_code, char *db_type);
extern lg_list_t *ad_system_db_element_find_txt(char *region_code, char *disease_code, char *db_type);
extern lg_list_t *ad_system_db_element_find_brand(char *region_code, char *disease_code, char *db_type);
extern lg_list_t *ad_system_db_element_default_img();
extern char *ad_system_db_element_find_ip(char *ip_str);

#endif


