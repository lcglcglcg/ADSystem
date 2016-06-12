
#include "lg_json.h"
#include "lg_config_file.h"
#include "ad_system_display_db.h"

char *oracle_host = NULL;
char *oracle_name = NULL;
char *oracle_pass = NULL;

int ad_system_db_load_init()
{

    lg_list_t config_file_head;
    if (lg_config_file_load(&config_file_head, "/ad_system/ad_system_familydoctor_jbk/conf/ad_system.conf")) {

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

ad_system_info_t *ad_system_info_create(lg_list_t *head,
	double cpc_price,
	char *img_size,
	char *buffer,
	size_t size)
{

    ad_system_info_t *node = malloc(sizeof(ad_system_info_t));
    if (!node) return NULL;

    node->cpc_price = cpc_price;
    node->buffer = malloc(size);
    node->size = size;

    if (!node->buffer) {

	free(node);
	return NULL;
    }

    if (img_size) strcpy(node->img_size, img_size);
    memcpy(node->buffer, buffer, size);
    return node;
}

lg_db_t *ad_system_db_load_info_img(ad_system_db_t *root_db, lg_list_t *head, char *db_name)
{

    lg_list_init(head);
    lg_db_t *db = lg_db_create(db_name);
    if (!db) return NULL;

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
	    );

    lg_ocilib_t conn;
    if (lg_ocilib_conn(&conn, oracle_host, oracle_name, oracle_pass))
	return NULL;

    if (lg_ocilib_find(&conn, buffer)) {

	lg_ocilib_free(&conn);
	return NULL;
    }

    while(lg_ocilib_fetch_next(&conn)) {

	char *account_id		= lg_ocilib_get_string(&conn, 1);
	// char *campaign_id		= lg_ocilib_get_string(&conn, 2);
	// char *adgroup_id		= lg_ocilib_get_string(&conn, 3);
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

	ad_system_info_t *info = ad_system_info_create(head,
		cpc_price,
		img_size,
		buffer,
		size);

	if (info) ad_system_db_element_add_list(root_db, db,
		region_type,
		region_code_array,
		disease_code_array,
		info);
    }

    lg_ocilib_free(&conn);
    return db;
}

lg_db_t *ad_system_db_load_info_txt(ad_system_db_t *root_db, lg_list_t *head, char *db_name)
{

    lg_list_init(head);
    lg_db_t *db = lg_db_create(db_name);
    if (!db) return NULL;

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
	    );

    lg_ocilib_t conn;
    if (lg_ocilib_conn(&conn, oracle_host, oracle_name, oracle_pass))
	return NULL;

    if (lg_ocilib_find(&conn, buffer)) {

	lg_ocilib_free(&conn);
	return NULL;
    }

    while(lg_ocilib_fetch_next(&conn)) {

	char *account_id		= lg_ocilib_get_string(&conn, 1);
	// char *campaign_id		= lg_ocilib_get_string(&conn, 2);
	// char *adgroup_id		= lg_ocilib_get_string(&conn, 3);
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

	ad_system_info_t *info = ad_system_info_create(head,
		cpc_price,
		NULL,
		buffer,
		size);

	if (info) ad_system_db_element_add_list(root_db, db,
		region_type,
		region_code_array,
		disease_code_array,
		info);
    }

    lg_ocilib_free(&conn);
    return db;
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



