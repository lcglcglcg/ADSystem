
#include "lg_json.h"
#include "lg_config_file.h"
#include "ad_system_display_db.h"

char *oracle_host = NULL;
char *oracle_name = NULL;
char *oracle_pass = NULL;
static lg_list_t info_pool = {&info_pool, &info_pool};
static lg_list_t info_pool_free = {&info_pool_free, &info_pool_free};

ad_system_info_t *ad_system_db_get_info()
{

    lg_list_t *p = lg_queue(&info_pool_free);
    if (p) {
	lg_list_add(&info_pool, p);
	return lg_list_entry(p, ad_system_info_t, pool);
    }

    ad_system_info_t *node = malloc(sizeof(ad_system_info_t));
    if (!node) return NULL;

    lg_list_add(&info_pool, &node->pool);
    return node;
}

void ad_system_db_info_free(ad_system_info_t *info)
{
    if (info ) {

	lg_list_del(&info->pool);
	lg_list_add(&info_pool_free, &info->pool);

	if (info->buffer) {

	    free(info->buffer);
	    info->buffer = NULL;
	}
    }
}

int ad_system_db_load_init()
{

    lg_list_t config_file_head;
    if (lg_config_file_load(&config_file_head, "/ad_system/ad_system_familydoctor_yyk/conf/ad_system.conf")) {

	fprintf(stdout, "lg_config_file_load err\n");
	return -1;
    }

    if (!(oracle_name = lg_config_file_find(&config_file_head, "oracle_name")))
	return -1;

    if (!(oracle_pass = lg_config_file_find(&config_file_head, "oracle_pass")))
	return -1;

    if (!(oracle_host = lg_config_file_find(&config_file_head, "oracle_host")))
	return -1;

    fprintf(stdout, "ORACLE_NAME=%s\n", oracle_name);
    fprintf(stdout, "ORACLE_PASS=%s\n", oracle_pass);
    fprintf(stdout, "ORACLE_HOST=%s\n", oracle_host);
    return 0;
}

int ad_system_db_load_sort(void *priv, lg_list_t *a, lg_list_t *b) 
{

    ad_system_info_t *info_a = lg_list_entry(a, ad_system_info_t, select_next);
    ad_system_info_t *info_b = lg_list_entry(b, ad_system_info_t, select_next);

    if (info_b->cpc_price > info_a->cpc_price)
	return 1;

    return -1; 
}

ad_system_info_t *ad_system_info_create(
	int hospital_levle,
	double cpc_price,
	char *img_size,
	char *buffer,
	size_t size,
	char *account_id,
	char *campaign_id,
	char *adgroup_id,
	char *creative_id)
{

    ad_system_info_t *node = ad_system_db_get_info();
    if (!node) return NULL;

    node->level = hospital_levle;
    node->cpc_price = cpc_price;
    node->buffer = malloc(size);
    node->size = size;

    strcpy(node->account_id, account_id);
    strcpy(node->campaign_id, campaign_id);
    strcpy(node->adgroup_id, adgroup_id);
    strcpy(node->creative_id, creative_id);

#if 0
    fprintf(stdout, "[add][%s][%s][%s][%s][%p]\n", 
	    account_id,
	    campaign_id,
	    adgroup_id,
	    creative_id,
	    node->buffer);
#endif

    if (node->buffer) memcpy(node->buffer, buffer, size);
    return node;
}

int ad_system_db_load_info_img(ad_system_db_t *db)
{
    return 0;
}

int ad_system_db_load_info_txt(ad_system_db_t *db)
{

    db->info_txt = lg_db_create("txt");
    db->info_txt_all_region = lg_db_create("txt_all_region");
    if (!db->info_txt || !db->info_txt_all_region)
	return -1;

    char buffer[409600];
    sprintf(buffer, "select "
	    "t.account_id,"
	    "t.campaign_id,"
	    "t.adgroup_id,"
	    "t.creative_id,"
	    "campaign.region_type,"
	    "campaign.region_serialize,"
	    "adgroup_targeting.code,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price,"
	    "t.creative_name,"
	    "t.creative_append_name,"
	    "t.tel,"
	    "t.creative_description,"
	    "t.map_url,"
	    "t.time_url,"
	    "t.comment_url,"
	    "t.advisory_url,"
	    "t.mobile_url,"
	    "t.display_url,"
	    "t.doctors_name,"
	    "t.doctors_description,"
	    "t.doctors_img,"
	    "t.hospital_img,"
	    "t.hospital_levle,"
	    "account.webim"
	    "    from t_text_creative t"
	    "    left join t_account account on t.account_id = account.account_id"
	    "    left join t_campaign campaign on t.campaign_id = campaign.campaign_id"
	    "    left join t_adgroup adgroup on t.adgroup_id = adgroup.adgroup_id"
	    "    left join t_adgroup_targeting adgroup_targeting on t.adgroup_id = adgroup_targeting.adgroup_id"
	    "    where account.row_state = 1"
	    "    and campaign.pause = 0"
	    "    and adgroup.adgroup_state = 21"
	    "    and t.status = 1"
	    );

    lg_ocilib_t conn;
    if (lg_ocilib_conn(&conn, oracle_host, oracle_name, oracle_pass))
	return -1;

    if (lg_ocilib_find(&conn, buffer)) {

	lg_ocilib_free(&conn);
	return -1;
    }

    while(lg_ocilib_fetch_next(&conn)) {

	char *account_id		= lg_ocilib_get_string(&conn, 1);
	char *campaign_id		= lg_ocilib_get_string(&conn, 2);
	char *adgroup_id		= lg_ocilib_get_string(&conn, 3);
	char *creative_id		= lg_ocilib_get_string(&conn, 4);
	int region_type			= lg_ocilib_get_number(&conn, 5);
	char *region_code_array		= lg_ocilib_get_string(&conn, 6);
	char *disease_code_array	= lg_ocilib_get_string(&conn, 7);
	double cpc_price		= lg_ocilib_get_double(&conn, 8);
	// double cpm_price		= lg_ocilib_get_double(&conn, 9);
	char *creative_name		= lg_ocilib_get_string(&conn, 10);
	char *creative_append_name      = lg_ocilib_get_string(&conn, 11);
	char *tel                       = lg_ocilib_get_string(&conn, 12);
	char *creative_description      = lg_ocilib_get_string(&conn, 13);
	char *map_url                   = lg_ocilib_get_string(&conn, 14);
	char *time_url                  = lg_ocilib_get_string(&conn, 15);
	char *comment_url               = lg_ocilib_get_string(&conn, 16);
	char *advisory_url              = lg_ocilib_get_string(&conn, 17);
	char *mobile_url                = lg_ocilib_get_string(&conn, 18);
	char *display_url               = lg_ocilib_get_string(&conn, 19);
	char *doctors_name              = lg_ocilib_get_string(&conn, 20);
	char *doctors_description       = lg_ocilib_get_string(&conn, 21);
	char *doctors_img               = lg_ocilib_get_string(&conn, 22);
	char *hospital_img              = lg_ocilib_get_string(&conn, 23);
	int hospital_levle            	= lg_ocilib_get_number(&conn, 24);
	char *webim                     = lg_ocilib_get_string(&conn, 25);

	size_t size = lg_json_sprintf(buffer, NULL, NULL, 18,
		"%s", "act",					account_id,
		"%s", "creative_id",				creative_id,
		"%s", "creative_name",				creative_name,
		"%s", "creative_append_name",                   creative_append_name,
		"%s", "tel",                                    tel,
		"%s", "creative_description",                   creative_description,
		"%s", "map_url",                                map_url,
		"%s", "time_url",                               time_url,
		"%s", "comment_url",                            comment_url,
		"%s", "advisory_url",                           advisory_url,
		"%s", "mobile_url",                             mobile_url,
		"%s", "display_url",                            display_url,
		"%s", "doctors_name",                           doctors_name,
		"%s", "doctors_description",                    doctors_description,
		"%s", "doctors_img",                            doctors_img,
		"%s", "hospital_img",                           hospital_img,
		"%u", "hospital_levle",                         hospital_levle,
		"%s", "webIM",                                  webim
		);

	ad_system_info_t *info = ad_system_info_create(
		hospital_levle,
		cpc_price,
		NULL,
		buffer,
		size,
		account_id,
		campaign_id,
		adgroup_id,
		creative_id);

	if (info) {

	    if (strcmp(disease_code_array, "1001095000") == 0) {

		lg_list_add(&db->info_txt_default, &info->select_next);
		lg_list_sort(NULL, &db->info_txt_default, 40960, ad_system_db_load_sort);

	    } else {

		if (region_type == 0) {

		    ad_system_db_element_add_list_r(db->info_txt_all_region,
			    disease_code_array,
			    info);
		} else {

		    ad_system_db_element_add_list(db->info_txt, 
			    region_code_array,
			    disease_code_array,
			    info);
		}
	    }
	}
    }

    lg_ocilib_free(&conn);
    return 0;
}

// dict
lg_db_t *ad_system_db_load_kv(char *db_name, char *sql_str, int type)
{

    lg_db_t *db = lg_db_create(db_name);
    if (!db) return NULL;

    lg_ocilib_t conn;
    if (lg_ocilib_conn(&conn, oracle_host, oracle_name, oracle_pass))
	return NULL;

    if (lg_ocilib_find(&conn, sql_str)) {

	lg_ocilib_free(&conn);
	return NULL;
    }
    
    while(lg_ocilib_fetch_next(&conn)) {

	char *key	= lg_ocilib_get_string(&conn, 1);
	char *value	= lg_ocilib_get_string(&conn, 2);
	ad_system_db_element_add_kv(db, key, value, type);
    }

    lg_ocilib_free(&conn);
    return db;
}

lg_db_t *ad_system_db_load_kv2(char *db_name, char *sql_str)
{

    lg_db_t *db = lg_db_create(db_name);
    if (!db) return NULL;

    lg_ocilib_t conn;
    if (lg_ocilib_conn(&conn, oracle_host, oracle_name, oracle_pass))
	return NULL;

    if (lg_ocilib_find(&conn, sql_str)) {

	lg_ocilib_free(&conn);
	return NULL;
    }

    while(lg_ocilib_fetch_next(&conn)) {

	char *key	= lg_ocilib_get_string(&conn, 1);
	char *value	= lg_ocilib_get_string(&conn, 2);
	int type 	= lg_ocilib_get_number(&conn, 3);
	ad_system_db_element_add_kv(db, key, value, type);
    }

    lg_ocilib_free(&conn);
    return db;
}

lg_db_t *ad_system_db_load_dict_place()
{
    return ad_system_db_load_kv2("place", "select ad_place_id,ad_size,type from t_ad_place");
}

lg_db_t *ad_system_db_load_dict_region()
{
    return ad_system_db_load_kv("region", "select uri,code from t_code_39_region", 2);
}

lg_db_t *ad_system_db_load_dict_dept()
{
    return ad_system_db_load_kv("dept", "select uri,code from t_code_39_disease where type=1", 3);
}

lg_db_t *ad_system_db_load_dict_disease()
{
    return ad_system_db_load_kv("disease", "select uri,code from t_code_39_disease where type=2", 3);
}

lg_db_t *ad_system_db_load_dict_special()
{
    return ad_system_db_load_kv("special", "select uri,code from t_code_39_disease where type=3", 3);
}





