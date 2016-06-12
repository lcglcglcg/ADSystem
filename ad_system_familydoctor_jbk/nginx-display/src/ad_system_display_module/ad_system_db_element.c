
#include "lg_db.h"
#include "lg_string.h"
#include "lg_ip_addr_find.h"
#include "ad_system_display_db.h"
#include <stdarg.h>
#include <pthread.h>

static ad_system_db_t *db = NULL;
static lg_rbtree_head g_ip = RB_ROOT; //ip库

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
    lg_list_init(&db->info_default_img);

    // img
    db->info_img = ad_system_db_load_info_img(db, &db->info_img_list, "img");

    // txt
    db->info_txt = ad_system_db_load_info_txt(db, &db->info_txt_list, "txt");

    // dict
    db->dict_place = ad_system_db_load_dict_place();
    db->dict_region = ad_system_db_load_dict_region();
    db->dict_disease = ad_system_db_load_dict_disease();

    fprintf(stdout, "info_img=%p\n",              db->info_img);
    fprintf(stdout, "info_txt=%p\n",              db->info_txt);
    fprintf(stdout, "dict_place=%p\n",            db->dict_place);
    fprintf(stdout, "dict_region=%p\n",           db->dict_region);
    fprintf(stdout, "dict_disease=%p\n",          db->dict_disease);

    return db;
};

void ad_system_db_info_list_free(lg_list_t *head)
{

    lg_list_t *p = head->next;
    while(p != head) {

	ad_system_info_t *info = (ad_system_info_t *)p;
	p = p->next;

	free(info->buffer);
	free(info);
    }
}

void ad_system_db_data_free(lg_list_t *m_list, void *m_data)
{
    if (m_data) free(m_data);
}

void ad_system_db_free(ad_system_db_t *db)
{

    if (!db) return;

    lg_db_free(db->info_img, ad_system_db_data_free);
    lg_db_free(db->info_txt, ad_system_db_data_free);

    lg_db_free(db->dict_place, ad_system_db_data_free);
    lg_db_free(db->dict_region, ad_system_db_data_free);
    lg_db_free(db->dict_disease, ad_system_db_data_free);

    ad_system_db_info_list_free(&db->info_img_list);
    ad_system_db_info_list_free(&db->info_txt_list);

    free(db);
}

int ad_system_db_img_sort_cmp(void *priv, lg_list_t *a, lg_list_t *b) 
{

    lg_db_list_node_t *node_a = (lg_db_list_node_t *)a;
    lg_db_list_node_t *node_b = (lg_db_list_node_t *)b;

    ad_system_info_t *info_a = (ad_system_info_t *)node_a->m_data;
    ad_system_info_t *info_b = (ad_system_info_t *)node_b->m_data;

    if (info_b->cpc_price > info_a->cpc_price)
	return 1;

    return -1;
}

int ad_system_db_element_add_list_default(ad_system_db_t *db, ad_system_info_t *info)
{

    lg_db_list_node_t *node = lg_db_tree_list_add(&db->info_default_img, info);
    lg_list_add(&db->info_default_img, &node->next);
    lg_list_sort(NULL, &db->info_default_img, 4096, ad_system_db_img_sort_cmp);
    return 0;
}

int ad_system_db_element_add_list(ad_system_db_t *root_db, lg_db_t *db,
	int region_type,
	char *region_code_array,
	char *disease_code_array,
	ad_system_info_t *info)
{

    if (strcmp(disease_code_array, "1999000000") == 0)
	return ad_system_db_element_add_list_default(root_db, info);

    char *s1 = region_code_array;
    char *s2 = disease_code_array;
    if (region_type == 0) s1 = get_region_code();

    char region_code[128], *region_save_ptr = NULL;
    char disease_code[128], *disease_save_ptr = NULL;
    while (lg_string_strtok(s1, region_code, &region_save_ptr, ',') && *region_code) {

	lg_db_node_t *region_node = lg_db_string_node_add(db, region_code);
	if (region_node) {

	    s2 = disease_code_array;
	    while (lg_string_strtok(s2, disease_code, &disease_save_ptr, ',') && *disease_code) {

		lg_db_node_t *disease_node = lg_db_string_node_child_add(region_node, disease_code);
		if (disease_node) {

		    lg_db_list_node_t *node = lg_db_tree_list_add(&disease_node->m_list, info);
		    lg_list_add(&disease_node->m_list, &node->next);
		    lg_list_sort(NULL, &disease_node->m_list, 4096, ad_system_db_img_sort_cmp);
		}

		s2 = NULL;
	    }
	}

	s1 = NULL;
    }

    return 0;
}

int ad_system_db_element_add_kv(lg_db_t *db, char *key, char *value, int type)
{

    lg_db_node_t *node = lg_db_string_find(db, key);
    if (node) return 0;

    ad_system_kv_t *kv = malloc(sizeof(ad_system_kv_t));
    strcpy(kv->key, key);
    strcpy(kv->value, value);
    kv->type = type;

    node = lg_db_string_node_add(db, key);
    node->m_data = kv;
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
    return ad_system_db_element_find_kv(db->dict_disease, code, NULL);
}

ad_system_kv_t *ad_system_db_element_find_place(char *code)
{

    return ad_system_db_element_find_kv(db->dict_place, code, NULL);
}

lg_list_t *ad_system_db_element_find_img(char *region_code, char *disease_code, char *db_type)
{

    if (!region_code || !disease_code) return NULL;
    return ad_system_db_element_find_list(db->info_img, region_code, disease_code, NULL);
}

lg_list_t *ad_system_db_element_find_txt(char *region_code, char *disease_code, char *db_type)
{

    if (!region_code || !disease_code) return NULL;
    return ad_system_db_element_find_list(db->info_txt, region_code, disease_code, NULL);
}

lg_list_t *ad_system_db_element_default_img()
{
    return &db->info_default_img;
}

char *ad_system_db_element_find_ip(char *ip_str)
{

    if (!ip_str) return NULL;
    lg_ip_addr_t *ip = lg_ip_addr_find(&g_ip, ip_str);
    if (!ip) return NULL;

    fprintf(stderr, "[ip name][%s]\n", ip->name);
    return ip->code;
}






