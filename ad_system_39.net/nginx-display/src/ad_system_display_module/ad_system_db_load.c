
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
    if (info->buffer) {

	lg_list_del(&info->pool);
	lg_list_add(&info_pool_free, &info->pool);

	free(info->buffer);
	info->buffer = NULL;
    }
}

int ad_system_db_load_init()
{

    lg_list_t config_file_head;
    if (lg_config_file_load(&config_file_head, "/ad_system/ad_system_39.net/conf/ad_system.conf")) {
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

    node->cpc_price = cpc_price;
    node->buffer = malloc(size);
    node->size = size;

    strcpy(node->account_id, account_id);
    strcpy(node->campaign_id, campaign_id);
    strcpy(node->adgroup_id, adgroup_id);
    strcpy(node->creative_id, creative_id);

    if (img_size) strcpy(node->img_size, img_size);
    if (node->buffer) memcpy(node->buffer, buffer, size);

#if 0
    fprintf(stdout, "[add][%s][%s][%s][%s][%p]\n", 
	    account_id,
	    campaign_id,
	    adgroup_id,
	    creative_id,
	    node->buffer);
#endif

    return node;
}

int ad_system_db_load_info_img(ad_system_db_t *db, int type)
{

    lg_db_t *db_info = lg_db_create("img");
    lg_db_t *db_info_all_region = lg_db_create("img");
    if (!db_info || !db_info_all_region)
	return -1;

    if (type == 1) {

	db->info_doctors_img = db_info;
	db->info_doctors_img_all_region = db_info_all_region;

    } else {

	db->info_hospitals_img = db_info;
	db->info_hospitals_img_all_region = db_info_all_region;
    }

    char buffer[409600];
    sprintf(buffer, "select "
	    "creative.account_id,"
	    "creative.campaign_id,"
	    "creative.adgroup_id,"
	    "creative.creative_id,"
	    "campaign.region_type,"
	    "campaign.region_serialize,"
	    "adgroup_targeting.code,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price,"
	    "creative.img_size,"
	    "creative.file_path,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    "account.webim"
	    " from t_creative creative"
	    " left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id"
	    " left join t_adgroup_targeting adgroup_targeting on creative.adgroup_id = adgroup_targeting.adgroup_id"
	    " left join t_campaign campaign on creative.campaign_id = campaign.campaign_id"
	    " left join t_account account on creative.account_id = account.account_id"
	    " where campaign.pause = 0"
	    " and adgroup.adgroup_state = 21"
	    " and creative.pause = 0"
	    " and account.row_state = 1"
	    " and campaign.type=%d",
	type);

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
	char *img_size			= lg_ocilib_get_string(&conn, 10);
	char *img_path			= lg_ocilib_get_string(&conn, 11);
	char *destination_url		= lg_ocilib_get_string(&conn, 12);
	char *display_url 		= lg_ocilib_get_string(&conn, 13);
	char *webim			= lg_ocilib_get_string(&conn, 14);

	size_t size = lg_json_sprintf(buffer, NULL, NULL, 7,
		"%s",	"displayUrl",		display_url,
		"%s",	"imgSize",		img_size,
		"%s",	"webIM",		webim,
		"%s",	"creativeId",		creative_id,
		"%s",	"filePath",		img_path,
		"%s",	"destinationUrl",	destination_url,
		"%s",	"act",			account_id);

	ad_system_info_t *info = ad_system_info_create(
		cpc_price,
		img_size,
		buffer,
		size,
		account_id,
		campaign_id,
		adgroup_id,
		creative_id);

	if (info) {

	    if (strcmp(disease_code_array, "1999000000") == 0) {

		lg_list_add(&db->info_default, &info->select_next);
		lg_list_sort(NULL, &db->info_default, 40960, ad_system_db_load_sort);

	    } else {

		if (region_type == 0) {

		    ad_system_db_element_add_list_r(db_info_all_region,
			    disease_code_array,
			    info);

		} else {

		    ad_system_db_element_add_list(db_info,
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

int ad_system_db_load_info_txt(ad_system_db_t *db, int type, int brand_type)
{

    lg_db_t *db_info = lg_db_create("txt");
    lg_db_t *db_info_all_region = lg_db_create("txt");
    if (!db_info || !db_info_all_region)
	return -1;

    if (brand_type == 1) {

	if (type == 1) {
	    db->info_doctors_txt = db_info;
	    db->info_doctors_txt_all_region = db_info_all_region;

	} else {
	    db->info_hospitals_txt = db_info;
	    db->info_hospitals_txt_all_region = db_info_all_region;
	}


    } else {

	if (type == 1) {
	    db->info_doctors_brand = db_info;
	    db->info_doctors_brand_all_region = db_info_all_region;

	} else {
	    db->info_hospitals_brand = db_info;
	    db->info_hospitals_brand_all_region = db_info_all_region;

	}
    }

    char buffer[409600];
    sprintf(buffer, "select creative.account_id,"
	    "creative.campaign_id,"
	    "creative.adgroup_id,"
	    "creative.creative_id,"
	    "campaign.region_type,"
	    "campaign.region_serialize,"
	    "adgroup_targeting.code,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price,"
	    "creative.title,"
	    "creative.description1,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    "creative.mobile_destination_url,"
	    "creative.mobile_display_url,"
	    "creative.img_path,"
	    "account.webim"
	    " from t_text_creative creative"
	    " left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id"
	    " left join t_adgroup_targeting adgroup_targeting on creative.adgroup_id = adgroup_targeting.adgroup_id"
	    " left join t_campaign campaign on creative.campaign_id = campaign.campaign_id"
	    " left join t_account account on creative.account_id = account.account_id"
	    " where campaign.pause = 0"
	    " and adgroup.adgroup_state = 21"
	    " and creative.pause = 0"
	    " and account.row_state = 1"
	    " and campaign.type=%d and creative.creative_type=%d",
	type,
	brand_type);

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
	char *title			= lg_ocilib_get_string(&conn, 10);
	char *description               = lg_ocilib_get_string(&conn, 11);
	char *destination_url           = lg_ocilib_get_string(&conn, 12);
	char *display_url               = lg_ocilib_get_string(&conn, 13);
	char *mobile_destination_url    = lg_ocilib_get_string(&conn, 14);
	char *mobile_display_url        = lg_ocilib_get_string(&conn, 15);
	char *img_path			= lg_ocilib_get_string(&conn, 16);
	char *webim			= lg_ocilib_get_string(&conn, 17);

	size_t size = lg_json_sprintf(buffer, NULL, NULL, 10,
		"%s",	"displayUrl",			display_url,
		"%s",	"title",			title,
		"%s",	"webIM",			webim,
		"%s",	"creativeId",			creative_id,
		"%s",	"description1",			description,
		"%s",	"destinationUrl",		destination_url,
		"%s",	"mobile_destination_url",	mobile_destination_url,
		"%s",	"mobile_display_url",		mobile_display_url,
		"%s",	"img_path",			img_path,
		"%s",	"act",				account_id);

	ad_system_info_t *info = ad_system_info_create(
		cpc_price,
		NULL,
		buffer,
		size,
		account_id,
		campaign_id,
		adgroup_id,
		creative_id);

	if (info) {

	    if (region_type == 0) {

		ad_system_db_element_add_list_r(db_info_all_region,
			disease_code_array,
			info);

	    } else {

		ad_system_db_element_add_list(db_info,
			region_code_array,
			disease_code_array,
			info);
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

lg_db_t *ad_system_db_load_dict_disease()
{
    return ad_system_db_load_kv("disease", "select uri,code from t_code_39_disease", 3);
}



