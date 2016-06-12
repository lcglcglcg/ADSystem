
#include "lg_db.h"
#include "lg_string.h"
#include "lg_ip_addr_find.h"
#include "ad_system_display_db.h"
#include <stdarg.h>
#include <pthread.h>

static ad_system_db_t *db = NULL;
static lg_rbtree_head g_ip = RB_ROOT; //ip库

static lg_list_t ad_system_db_list_info_pool = {&ad_system_db_list_info_pool, &ad_system_db_list_info_pool};
static lg_list_t ad_system_db_list_info_pool_free = {&ad_system_db_list_info_pool_free, &ad_system_db_list_info_pool_free};
static lg_list_t ad_system_db_kv_pool = {&ad_system_db_kv_pool, &ad_system_db_kv_pool};
static lg_list_t ad_system_db_kv_pool_free = {&ad_system_db_kv_pool_free, &ad_system_db_kv_pool_free};

static ad_system_list_info_t *ad_system_db_get_list_info()
{

    lg_list_t *p = lg_queue(&ad_system_db_list_info_pool_free);
    if (p) {
	lg_list_add(&ad_system_db_list_info_pool, p);
	return lg_list_entry(p, ad_system_list_info_t, pool);
    }

    ad_system_list_info_t *node = malloc(sizeof(ad_system_list_info_t));
    if (!node) return NULL;

    lg_list_add(&ad_system_db_list_info_pool, &node->pool);
    return node;
}

static ad_system_kv_t *ad_system_db_get_kv()
{

    lg_list_t *p = lg_queue(&ad_system_db_kv_pool_free);
    if (p) {
	lg_list_add(&ad_system_db_kv_pool, p);
	return lg_list_entry(p, ad_system_kv_t, pool);
    }

    ad_system_kv_t *node = malloc(sizeof(ad_system_kv_t));
    if (!node) return NULL;

    lg_list_add(&ad_system_db_kv_pool, &node->pool);
    return node;
}

void ad_system_db_init()
{

    lg_ip_addr_load(&g_ip, "/etc/ip.csv");

    ad_system_db_load_init();

    db = ad_system_db_create();
    fprintf(stdout, "runing: %p ...\n", db);

    pthread_t tid;
    pthread_create(&tid, NULL, ad_system_db_reload, NULL);
}

void *ad_system_db_reload(void *p)
{

    ad_system_db_t *old_db = NULL;
    ad_system_db_t *new_db = NULL;

    sleep(1200);
    for (;;) {

	fprintf(stdout, "reload: %p ...\n", db);
	new_db = ad_system_db_create();
	if (new_db) {

	    old_db = db;
	    db = new_db;

	} else {

	    old_db = new_db;
	}

	sleep(1200);
	ad_system_db_free(old_db);
	old_db = NULL;
    }

    pthread_exit(NULL);
}

ad_system_db_t *ad_system_db_create()
{

    ad_system_db_t *db = malloc(sizeof(ad_system_db_t));
    if (!db) return NULL;

    // default
    // lg_list_init(&db->info_txt_default);

    // info
    ad_system_db_load_info_img(db);
    ad_system_db_load_info_txt(db);

    // dict
    db->dict_place = ad_system_db_load_dict_place();
    db->dict_region = ad_system_db_load_dict_region();
    db->dict_disease = ad_system_db_load_dict_disease();

    fprintf(stdout, "info_img=%p\n",              db->info_img);
    fprintf(stdout, "info_img_all_region=%p\n",   db->info_img_all_region);
    fprintf(stdout, "info_txt=%p\n",              db->info_txt);
    fprintf(stdout, "info_txt_all_region=%p\n",   db->info_txt_all_region);
    fprintf(stdout, "dict_place=%p\n",            db->dict_place);
    fprintf(stdout, "dict_region=%p\n",           db->dict_region);
    fprintf(stdout, "dict_disease=%p\n",          db->dict_disease);

    return db;
};

void ad_system_db_data_free(char *key, lg_list_t *m_list, void *m_data)
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, m_list) {

	ad_system_list_info_t *list_info = (ad_system_list_info_t *)p;
	lg_list_del(&list_info->pool);
	lg_list_add(&ad_system_db_list_info_pool_free, &list_info->pool);

	ad_system_info_t *info = list_info->data;
	ad_system_db_info_free(info);
    }

    if (m_data) {

	ad_system_kv_t *kv = m_data;
	lg_list_del(&kv->pool);
	lg_list_add(&ad_system_db_kv_pool_free, &kv->pool);
    }
}

void ad_system_db_free(ad_system_db_t *db)
{

    if (!db) return;

    lg_db_free(db->info_img, ad_system_db_data_free);
    lg_db_free(db->info_img_all_region, ad_system_db_data_free);
    
    lg_db_free(db->info_txt, ad_system_db_data_free);
    lg_db_free(db->info_txt_all_region, ad_system_db_data_free);

    lg_db_free(db->dict_place, ad_system_db_data_free);
    lg_db_free(db->dict_region, ad_system_db_data_free);
    lg_db_free(db->dict_disease, ad_system_db_data_free);

    free(db);
}

void ad_system_db_element_add_list_from_info(lg_db_node_t *db, ad_system_info_t *info)
{

    ad_system_list_info_t *node = ad_system_db_get_list_info();
    if (node) {

	lg_list_add(&db->m_list, &node->next);
	node->data = info;
    }
}

int ad_system_db_element_add_list(lg_db_t *db,
	char *region_code_array,
	char *disease_code_array,
	ad_system_info_t *info)
{

    char region_code[128], *region_save_ptr = NULL;
    char disease_code[128], *disease_save_ptr = NULL;

    char *s1 = region_code_array;
    while (lg_string_strtok(s1, region_code, &region_save_ptr, ',') && *region_code) {

	lg_db_node_t *region_node = lg_db_string_node_add(db, region_code);
	ad_system_db_element_add_list_from_info(region_node, info);	// add element

	char *s2 = disease_code_array;
	while (lg_string_strtok(s2, disease_code, &disease_save_ptr, ',') && *disease_code) {

	    lg_db_node_t *disease_node = lg_db_string_node_child_add(region_node, disease_code);
	    ad_system_db_element_add_list_from_info(disease_node, info); // add element db->region->disease

	    // disease_node = lg_db_string_node_add(db_disease, disease_code);
	    // ad_system_db_element_add_list_from_info(disease_node, info); // add element db_disease

	    s2 = NULL;
	}

	s1 = NULL;
    }

    return 0;
}

int ad_system_db_element_add_list_r(lg_db_t *db,
	char *disease_code_array,
	ad_system_info_t *info)
{

    char disease_code[128], *disease_save_ptr = NULL;
    char *s1 = disease_code_array;
    while (lg_string_strtok(s1, disease_code, &disease_save_ptr, ',') && *disease_code) {

	lg_db_node_t *disease_node = lg_db_string_node_add(db, disease_code);
	ad_system_db_element_add_list_from_info(disease_node, info); // add element db_disease

	s1 = NULL;
    }

    return 0;
}

int ad_system_db_element_add_kv(lg_db_t *db, char *key, char *value, int type)
{

    if (!key || !value || strlen(key) >= 64 || strlen(value) >= 64)
	return -1;

    lg_db_node_t *node = lg_db_string_find(db, key);
    if (node) return 0;

    node = lg_db_string_node_add(db, key);
    if (node) {

	ad_system_kv_t *kv = ad_system_db_get_kv();
	// fprintf(stdout, "[add][%s][%p]\n", key, kv);
	if (kv) {

	    kv->type = type;
	    node->m_data = kv;
	    strcpy(kv->value, value);
	}
    }

    return 0;
}

void ad_system_db_element_find_print(char *db_name, char *key, void *node)
{

    fprintf(stdout, "[find][%s][%s][%p]\n", db_name, key, node);
}

ad_system_kv_t *ad_system_db_element_find_kv(lg_db_t *db, ...)
{

    va_list args;
    va_start(args, db);

    char *key = va_arg(args, char *);
    lg_db_node_t *node = lg_db_string_find(db, key);
    ad_system_db_element_find_print(db->db_name, key, node);

    while(key && node) {

	key = va_arg(args, char *);
	if (key) {

	    node = lg_db_string_child_find(node, key);
	    ad_system_db_element_find_print(db->db_name, key, node);
	}
    }

    va_end(args);

    if (!node) return NULL;
    return node->m_data;
}

lg_list_t *ad_system_db_element_find_list(lg_db_t *db, ...)
{

    va_list args;
    va_start(args, db);

    char *key = va_arg(args, char *);
    lg_db_node_t *node = lg_db_string_find(db, key);
    ad_system_db_element_find_print(db->db_name, key, node);

    while(key && node) {

	key = va_arg(args, char *);
	if (key) {

	    node = lg_db_string_child_find(node, key);
	    ad_system_db_element_find_print(db->db_name, key, node);
	}
    }

    va_end(args);
    if (!node) return NULL;
    return &node->m_list;
}

/*************************/
/*****全局 find ***************/
/*************************/

ad_system_kv_t *ad_system_db_element_find_code(char *code)
{
    if (!code) return NULL;

    ad_system_kv_t *kv = ad_system_db_element_find_kv(db->dict_region, code, NULL);
    if (kv) return kv;

    return ad_system_db_element_find_kv(db->dict_disease, code, NULL);
}

ad_system_kv_t *ad_system_db_element_find_disease(char *code)
{
    if (!code) return NULL;
    return ad_system_db_element_find_kv(db->dict_disease, code, NULL);
}

ad_system_kv_t *ad_system_db_element_find_place(char *code)
{
    if (!code) return NULL;
    return ad_system_db_element_find_kv(db->dict_place, code, NULL);
}

lg_list_t *ad_system_db_element_find_img(char *region_code, char *disease_code)
{
    return ad_system_db_element_find_list(db->info_img, region_code, disease_code, NULL);
}

lg_list_t *ad_system_db_element_find_img_all_region(char *disease_code)
{
    return ad_system_db_element_find_list(db->info_img_all_region, disease_code, NULL);
}

lg_list_t *ad_system_db_element_find_txt(char *region_code, char *disease_code)
{
    return ad_system_db_element_find_list(db->info_txt, region_code, disease_code, NULL);
}

lg_list_t *ad_system_db_element_find_txt_all_region(char *disease_code)
{
    return ad_system_db_element_find_list(db->info_txt_all_region, disease_code, NULL);
}

char *ad_system_db_element_find_ip(char *ip_str)
{

    if (!ip_str) return "1000100000";

    lg_ip_addr_t *ip = lg_ip_addr_find(&g_ip, ip_str);
    if (!ip) return "1000100000";

    return ip->code;
}






