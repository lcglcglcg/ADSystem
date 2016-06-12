
#ifndef __AD_SYSTEM_DISPLAY_DB_H__
#define __AD_SYSTEM_DISPLAY_DB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lg_db.h"
#include "lg_ocilib.h"

typedef struct {

    // lg_db_t	*info_img;
    lg_db_t	*info_txt;
    // lg_db_t	*info_txt_disease;
    lg_db_t	*info_txt_all_region;
    lg_list_t	info_txt_default;	//剩余流量

    // lg_db_t	*info_img;
    // lg_db_t	*info_img_disease;
    // lg_db_t	*info_img_all_region;
    // lg_db_t	*info_img_default;	//剩余流量

    lg_db_t	*dict_place;
    lg_db_t	*dict_region;
    lg_db_t	*dict_dept;		//科室
    lg_db_t	*dict_disease;		//疾病
    lg_db_t	*dict_special;		//专科医院

} ad_system_db_t;

typedef struct {

    lg_list_t next;
    lg_list_t select_next;

    char account_id[64];
    char campaign_id[64];
    char adgroup_id[64];
    char creative_id[64];

    int level;
    double cpc_price;

    char *buffer;
    size_t size;

    lg_list_t pool;

} ad_system_info_t;

typedef struct {

    lg_list_t 	next;
    void 	*data;

    lg_list_t pool;

} ad_system_list_info_t;

typedef struct {

    int type;
    char value[64];

    lg_list_t pool;

} ad_system_kv_t;

// init
extern void ad_system_db_init();
extern int ad_system_db_load_init();
extern void *ad_system_db_reload(void *p);

// create/free
extern ad_system_db_t *ad_system_db_create();
extern void ad_system_db_free(ad_system_db_t *db);
extern void ad_system_db_info_free(ad_system_info_t *info);

// add
extern int ad_system_db_element_add_list(lg_db_t *db, 
	char *region_code_array,
	char *disease_code_array,
	ad_system_info_t *info);

extern int ad_system_db_element_add_list_r(lg_db_t *db,
	char *disease_code_array,
	ad_system_info_t *info);

extern int ad_system_db_element_add_kv(lg_db_t *db, char *key, char *value, int type);

// load
extern int ad_system_db_load_info_img(ad_system_db_t *db);
extern int ad_system_db_load_info_txt(ad_system_db_t *db);
extern lg_db_t *ad_system_db_load_dict_place();
extern lg_db_t *ad_system_db_load_dict_region();
extern lg_db_t *ad_system_db_load_dict_dept();
extern lg_db_t *ad_system_db_load_dict_disease();
extern lg_db_t *ad_system_db_load_dict_special();

// find
extern lg_list_t *ad_system_db_element_find_img(char *region_code, char *disease_code);
extern lg_list_t *ad_system_db_element_find_txt(char *region_code, char *disease_code);
extern lg_list_t *ad_system_db_element_find_txt_all_region(char *disease_code);
extern lg_list_t *ad_system_db_element_find_txt_default();

extern ad_system_kv_t *ad_system_db_element_find_place(char *code);
extern ad_system_kv_t *ad_system_db_element_find_dept(char *code);
extern ad_system_kv_t *ad_system_db_element_find_disease(char *code);
extern ad_system_kv_t *ad_system_db_element_find_special(char *code);
extern ad_system_kv_t *ad_system_db_element_find_region(char *code);
extern char *ad_system_db_element_find_ip(char *ip_str);

#endif


