
#include <ocilib.h>
#include <ocilib_internal.h>
#include <arpa/inet.h>

#include "lg_string.h"
#include "ad_system_sql.h"
#include "ad_system_domain_parse.h"

/****************/
/*oracle oci API*/
/****************/

char *oracle_name = NULL;
char *oracle_pass = NULL;
char *oracle_host = NULL;

void ad_system_sql_err_handler(OCI_Error *err)
{
    fprintf(stderr, OCI_ErrorGetString(err));
}

int ad_system_sql_init(lg_list_t *config_file_head)
{

    if (!(oracle_name = lg_config_file_find(config_file_head, "oracle_name")))
	return -1;;

    if (!(oracle_pass = lg_config_file_find(config_file_head, "oracle_pass")))
	return -1;;

    if (!(oracle_host = lg_config_file_find(config_file_head, "oracle_host")))
	return -1;;
    
    fprintf(stdout, "ORACLE_NAME=%s\n", oracle_name);
    fprintf(stdout, "ORACLE_PASS=%s\n", oracle_pass);
    fprintf(stdout, "ORACLE_HOST=%s\n", oracle_host);
    //fprintf(stdout, "ORACLE_POOL=%d\n", oracle_pool);

    // setenv("ORACLE_HOME", "/oracle/10.2/", 1);
    setenv("NLS_LANG","AMERICAN_AMERICA.UTF8",1);
    if (!OCI_Initialize(ad_system_sql_err_handler, NULL, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
	return -1;; 

    return 0;
}

int ad_system_sql_connect(ad_system_sql_t *p)
{

        OCI_Connection *cn = NULL;
	    if (!(cn = OCI_ConnectionCreate(oracle_host, oracle_name, oracle_pass, OCI_SESSION_DEFAULT)))
		        return -1;


    OCI_Statement *st = NULL;
    if (!(st = OCI_StatementCreate(cn))) {

	OCI_ConnectionFree(cn);
	return -1;
    }

    p->cn = cn;
    p->st = st;
    p->rs = NULL;
    return 0;
}

int _ad_system_sql_select(ad_system_sql_t *p, char *sql_str)
{

    if (!(OCI_ExecuteStmt(p->st, sql_str)))
	return -1;

    if (!(p->rs = (OCI_Resultset *)OCI_GetResultset(p->st)))
	return -1;

    return 0;
}

int ad_system_sql_select(ad_system_sql_t *p, char *sql_str)
{

    memset(p, 0, sizeof(ad_system_sql_t));

        OCI_Connection *cn = NULL;
	    if (!(cn = OCI_ConnectionCreate(oracle_host, oracle_name, oracle_pass, OCI_SESSION_DEFAULT)))
		        return -1;


    OCI_Statement *st = NULL;
    if (!(st = OCI_StatementCreate(cn))) {

	OCI_ConnectionFree(cn);
	return -1;
    }

    if (!(OCI_ExecuteStmt(st, sql_str))) {

	OCI_ConnectionFree(cn);
	return -1;
    }

    OCI_Resultset *rs = NULL;
    if (!(rs = OCI_GetResultset(st))) {

	OCI_ConnectionFree(cn);
	return -1;
    }

    p->cn = cn;
    p->st = st;
    p->rs = rs;
    return 0;
}

int ad_system_sql_fetch_next(ad_system_sql_t *p)
{
    return (OCI_FetchNext(p->rs));
}

int ad_system_sql_get_number(ad_system_sql_t *p, char *name, int index)
{
    if (name) return OCI_GetInt2(p->rs, name);
    return OCI_GetInt(p->rs, index);
}

double ad_system_sql_get_double(ad_system_sql_t *p, char *name, int index)
{
    if (name) OCI_GetDouble2(p->rs, name);
    return OCI_GetDouble(p->rs, index);
}

const char *ad_system_sql_get_string(ad_system_sql_t *p, char *name, int index)
{
    if (name) return OCI_GetString2(p->rs, name);
    return OCI_GetString(p->rs, index);
}

int ad_system_sql_execute(ad_system_sql_t *p, char *sql_str)
{

    memset(p, 0, sizeof(ad_system_sql_t));

        OCI_Connection *cn = NULL;
	    if (!(cn = OCI_ConnectionCreate(oracle_host, oracle_name, oracle_pass, OCI_SESSION_DEFAULT)))
		        return -1;


    OCI_Statement *st = NULL;
    if (!(st = OCI_StatementCreate(cn))) {

	OCI_ConnectionFree(cn);
	return -1;
    }

    if (!(OCI_ExecuteStmt(st, sql_str))) {

	OCI_Rollback(cn);
	OCI_ConnectionFree(cn);
	return -1;
    }

    p->cn = cn;
    p->st = st;
    return 0;
}

int _ad_system_sql_execute(ad_system_sql_t *p, char *sql_str)
{

    if (!(OCI_ExecuteStmt(p->st, sql_str)))
	return -1;

    return 0;
}

void ad_system_sql_commit(ad_system_sql_t *p)
{
    OCI_Commit(p->cn);
}

void ad_system_sql_rollback(ad_system_sql_t *p)
{
    OCI_Rollback(p->cn);
}

void ad_system_sql_free(ad_system_sql_t *p)
{
    if (p->st)
    OCI_StatementFree(p->st);
    OCI_ConnectionFree(p->cn);
}

char *ad_system_sql_string_escaped(char *buffer, const char *str, int mode)
{

    char *val = buffer;
    for (; *str; str++) {

	if (mode && *str == '%')
	    *val++ = '\\';

	if (*str == '\'')
	    *val++ = '\'';

	*val++ = *str;
    }

    *val = 0;
    return buffer;
}


/****************/
/*   业务逻辑   */
/****************/

//登录
int ad_system_sql_login(const char *account_name, const char *password_md5, char *account_id)
{

    char sql_str[1024];
    sprintf(sql_str, "select t.manager_id  from t_manager t where t.login_name='%s' and t.login_password_md5='%s'",
	    account_name,
	    password_md5);

    fprintf(stdout, "%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(account_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_get_title_info(char *account_id, char *account_name)
{

    char sql_str[1024];
    sprintf(sql_str, "select login_name from t_manager where manager_id='%s'", account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(account_name, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_append_creative_unique(ad_system_sql_t *p, const char *creative_id)
{

    char sql_str[40960] = {0};
    sprintf(sql_str, "select "
	    "account.account_id,"
	    "account.login_name as account_name,"
	    "creative.append_id as creative_id,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.content,"
	    "to_char(creative.last_update_time, 'yyyy-mm-dd HH24:MI:SS') as last_update_time "
	    "from t_text_creative_append creative "
	    "left join t_account account on creative.account_id = account.account_id "
	    "where creative.row_state=1 and creative.append_id='%s'", creative_id);

    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_append_creative(ad_system_sql_t *p,
	int find_type, 
	const char *find_name, 
	int find_status, 
	int balance_type,
	int sort_type, 
	int sort_column, 
	int index_s, 
	int index_e)
{

    char name[1024] = "";
    if (find_type == 1)
	sprintf(name, "and creative.content like '%%%s%%'", find_name);
    else if (find_type == 2)
	sprintf(name, "and account.login_name like '%%%s%%'", find_name);

    char status[1024] = "";
    switch(find_status) {

	case 1:
	    sprintf(status, "and creative.creative_state=31");
	    break;

	case 2:
	    sprintf(status, "and creative.creative_state=32");
	    break;

	case 4:
	    sprintf(status, "and creative.creative_state=34");
	    break;

	case 5:
	    sprintf(status, "and creative.creative_state=35");
	    break;
    }

    char sort[128] = "account_name";
    if (sort_type == 1) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "account_name");
		break;

	    case 2:
		sprintf(sort, "last_update_time");
		break;

	    case 3:
		sprintf(sort, "creative_state");
		break;
	}

    } else if (sort_type == 2) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "account_name desc");
		break;

	    case 2:
		sprintf(sort, "last_update_time desc");
		break;

	    case 3:
		sprintf(sort, "creative_state desc");
		break;
	}
    }

    char balance[128] = "";
    if (balance_type == 2)
	sprintf(balance, "and account.balance > 0");

    char sql_str[40960] = {0};
    sprintf(sql_str, "select "
	    "t.account_id,"
	    "t.account_name,"
	    "t.creative_id,"
	    "t.creative_state,"
	    "t.creative_refuse_reason,"
	    "t.creative_content,"
	    "t.last_update_time,"
	    "t.auditor_name,"
	    "t.auditor_update_time "
	    "from (select "
	    "rownum page_index,"
	    "account.account_id,"
	    "account.login_name as account_name,"
	    "creative.append_id as creative_id,"
	    "decode(creative.creative_state,31,1,32,2,33,3,34,4,35,5,1) as creative_state,"
	    "creative.refuse_reason as creative_refuse_reason,"
	    "creative.content as creative_content,"
	    "to_char(creative.last_update_time, 'yyyy-mm-dd HH24:MI:SS') as last_update_time,"
	    "manager.login_name as auditor_name,"
	    "to_char(creative.auditor_update_time, 'yyyy-mm-dd HH24:MI:SS') as auditor_update_time "
	    "from t_text_creative_append creative "
	    "left join t_account account on creative.account_id=account.account_id "
	    "left join t_manager manager on creative.auditor_id=manager.manager_id "
	    "where creative.row_state=1 %s %s %s order by %s) t "
	    "where t.page_index between %u and %u", 

	name, 
	status, 
	balance, 
	sort, 
	index_s, 
	index_e);

    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_manager_account_list(ad_system_sql_t *p,
	int find_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	int status,
	const char *account_id,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "LOGIN_NAME");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "COMPANY_NAME");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "REGION_NAME_CN");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "CONSUMPTION");
	    break;

	case 5:
	    sort_size = sprintf(sort_str, "CREATE_TIME");
	    break;

	case 6:
	    sort_size = sprintf(sort_str, "ROW_STATE");
	    break;
    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");

    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and REGION_NAME_CN like '%%%s%%' ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and %s <= nvl(INVEST - BALANCE, 0) ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and %s >= nvl(INVEST - BALANCE, 0) ", find_name);
		break;

	    case 7:
		size += sprintf(&where_str[size], "and SALES like '%%%s%%' ", find_name);
		break;

	    case 8:
		size += sprintf(&where_str[size], "and CUSTOMER_SERVICE like '%%%s%%' ", find_name);
		break;
	}
    }

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if ((find_type & 4) && status != 100)
	size += sprintf(&where_str[size], "and ROW_STATE=%u ", status);

    if (find_type & 8)
	size += sprintf(&where_str[size], "and ACCOUNT_ID=%s ", account_id);

    char sql_str[4096] = {0};
    sprintf(sql_str, "select * from ("
	    "select "
	    "ACCOUNT_ID,"
	    "API_TOKEN,"
	    "LOGIN_NAME,"
	    "COMPANY_NAME,"
	    "COMPANY_SITE,"
	    "DOMAIN,"
	    "INDUSTRY,"
	    "PROVINCE_ID,"
	    "CITY_ID,"
	    "ADDRESS,"
	    "POSTCODE,"
	    "CONTACT,"
	    "TELEPHONE,"
	    "FAX,"
	    "MOBILE,"
	    "EMAIL,"
	    "WEBIM,"
	    "GUAHAO_TOKEN,"
	    "LOGIN_PASSWORD,"
	    "PROTECT_QUESTION1,"
	    "PROTECT_ANSWER1,"
	    "PROTECT_QUESTION2,"
	    "PROTECT_ANSWER2,"
	    "PROTECT_QUESTION3,"
	    "PROTECT_ANSWER3,"
	    "PROTECT_EMAIL,"
	    "PROTECT_MOBILE,"
	    "HOSPITAL_IMG,"
	    "REGION_NAME_CN,"
	    "CONSUMPTION,"
	    "CREATE_TIME,"
	    "ROW_STATE,"
	    "SALES,"
	    "CUSTOMER_SERVICE,"
	    "PLACE_REGION,"
	    "JOIN_DSP,"
	    "rownum as rn "
	    "from (select "
	    "ACCOUNT_ID,"
	    "API_TOKEN as API_TOKEN,"
	    "LOGIN_NAME as LOGIN_NAME,"
	    "COMPANY_NAME as COMPANY_NAME,"
	    "COMPANY_SITE as COMPANY_SITE,"
	    "DOMAIN as DOMAIN,"
	    "INDUSTRY as INDUSTRY,"
	    "PROVINCE_ID as PROVINCE_ID,"
	    "CITY_ID as CITY_ID,"
	    "ADDRESS as ADDRESS,"
	    "POSTCODE as POSTCODE,"
	    "CONTACT as CONTACT,"
	    "TELEPHONE as TELEPHONE,"
	    "FAX as FAX,"
	    "MOBILE as MOBILE,"
	    "EMAIL as EMAIL,"
	    "WEBIM as WEBIM,"
	    "GUAHAO_TOKEN as GUAHAO_TOKEN,"
	    "LOGIN_PASSWORD as LOGIN_PASSWORD,"
	    "PROTECT_QUESTION1 as PROTECT_QUESTION1,"
	    "PROTECT_ANSWER1 as PROTECT_ANSWER1,"
	    "PROTECT_QUESTION2 as PROTECT_QUESTION2,"
	    "PROTECT_ANSWER2 as PROTECT_ANSWER2,"
	    "PROTECT_QUESTION3 as PROTECT_QUESTION3,"
	    "PROTECT_ANSWER3 as PROTECT_ANSWER3,"
	    "PROTECT_EMAIL as PROTECT_EMAIL,"
	    "PROTECT_MOBILE as PROTECT_MOBILE,"
	    "HOSPITAL_IMG as HOSPITAL_IMG,"
	    "REGION_NAME_CN as REGION_NAME_CN,"
	    "nvl(INVEST - BALANCE - BRAND_BALANCE, 0) as CONSUMPTION,"
	    "to_char(CREATE_TIME, 'yyyy-mm-dd HH24:MI:SS') as CREATE_TIME,"
	    "ROW_STATE as ROW_STATE,"
	    "SALES,"
	    "CUSTOMER_SERVICE,"
	    "PLACE_REGION,"
	    "JOIN_DSP "
	    "from T_ACCOUNT "
	    // "where 1=1 %s order by %s))  "
	    "where ROW_STATE != 0 %s order by %s))  "
	    "where rn between %u and %u",

	where_str,
	sort_str,
	page_index_s,
	page_index_e);

    fprintf(stderr, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_text_creative(ad_system_sql_t *p,
	int find_type, 
	char *find_name, 
	int find_status, 
	int balance_type,
	int sort_type, 
	int sort_column, 
	int index_s, 
	int index_e)
{

    char name[1024] = "";
    if (find_type == 1)
	sprintf(name, "and creative.title like '%%%s%%'", find_name);
    else if (find_type == 2)
	sprintf(name, "and account.login_name like '%%%s%%'", find_name);
    else if (find_type == 3)
	sprintf(name, "and creative.description1 like '%%%s%%'", find_name);

    char status[1024] = "";
    switch(find_status) {

	case 1:
	    sprintf(status, "and creative.creative_state=31");
	    break;

	case 2:
	    sprintf(status, "and creative.creative_state=32");
	    break;

	case 4:
	    sprintf(status, "and creative.creative_state=34");
	    break;

	case 5:
	    sprintf(status, "and creative.creative_state=35");
	    break;
    }

    char sort[128] = "account_name";
    if (sort_type == 1) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "creative_name");
		break;

	    case 2:
		sprintf(sort, "account_name");
		break;

	    case 3:
		sprintf(sort, "last_update_time");
		break;

	    case 4:
		sprintf(sort, "creative_state");
		break;
	}

    } else if (sort_type == 2) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "creative_name desc");
		break;

	    case 2:
		sprintf(sort, "account_name desc");
		break;

	    case 3:
		sprintf(sort, "last_update_time desc");
		break;

	    case 4:
		sprintf(sort, "creative_state desc");
		break;
	}
    }

    char balance[128] = "";
    if (balance_type == 2)
	sprintf(balance, "and account.balance > 0");

    char sql_str[40960] = {0};
    sprintf(sql_str, "select "
	    "t.account_id,"
	    "t.account_name,"
	    "t.creative_id,"
	    "t.creative_name,"
	    "t.creative_state,"
	    "t.creative_refuse_reason,"
	    "t.creative_description,"
	    "t.creative_destination_url,"
	    "t.creative_display_url,"
	    "t.creative_append_id,"
	    "t.last_update_time,"
	    "t.auditor_name,"
	    "t.auditor_update_time "
	    "from (select "
	    "rownum page_index,"
	    "account.account_id,"
	    "account.login_name as account_name,"
	    "creative.creative_id,"
	    "creative.title as creative_name,"
	    "decode(creative.creative_state,31,1,32,2,33,3,34,4,35,5,1) as creative_state,"
	    "creative.refuse_reason as creative_refuse_reason,"
	    "creative.description1 as creative_description,"
	    "creative.destination_url as creative_destination_url,"
	    "creative.display_url as creative_display_url,"
	    "creative.append_id as creative_append_id,"
	    "to_char(creative.last_update_time, 'yyyy-mm-dd HH24:MI:SS') as last_update_time,"
	    "manager.login_name as auditor_name,"
	    "to_char(creative.auditor_update_time, 'yyyy-mm-dd HH24:MI:SS') as auditor_update_time "
	    "from t_text_creative creative "
	    "left join t_account account on creative.account_id=account.account_id "
	    "left join t_manager manager on creative.auditor_id=manager.manager_id "
	    "where 1=1 %s %s %s order by %s) t where t.page_index between %u and %u", 
	name, 
	status, 
	balance, 
	sort, 
	index_s, 
	index_e);

    fprintf(stderr, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_account_frozen(const char *account_id)
{

    char sql_str[1024];
    sprintf(sql_str, "update t_campaign set synch_state = 0 where account_id = %s", account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}


int ad_system_sql_get_text_creative_count(int find_type, const char *find_name, int find_status, int balance_type, char *count)
{

    char name[1024] = "";
    if (find_type == 1)
	sprintf(name, "and creative.title like '%%%s%%'", find_name);
    else if (find_type == 2)
	sprintf(name, "and account.login_name like '%%%s%%'", find_name);

    char status[1024] = "";
    switch(find_status) {

	case 1:
	    sprintf(status, "and creative.creative_state=31");
	    break;

	case 2:
	    sprintf(status, "and creative.creative_state=32");
	    break;

	case 4:
	    sprintf(status, "and creative.creative_state=34");
	    break;

	case 5:
	    sprintf(status, "and creative.creative_state=35");
	    break;
    }

    char balance[128] = "";
    if (balance_type == 2)
	sprintf(balance, "and account.balance > 0");

    char sql_str[40960] = {0};
    sprintf(sql_str, "select count(*) from t_text_creative creative left join t_account account on creative.account_id=account.account_id where 1=1 %s %s %s", 
	    name, status, balance);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_append_creative_count(int find_type, char *find_name, int find_status, int balance_type, char *count)
{

    char name[1024] = "";
    if (find_type == 1)
	sprintf(name, "and creative.content like '%%%s%%'", find_name);
    else if (find_type == 2)
	sprintf(name, "and account.login_name like '%%%s%%'", find_name);

    char status[1024] = "";
    switch(find_status) {

	case 1:
	    sprintf(status, "and creative.creative_state=31");
	    break;

	case 2:
	    sprintf(status, "and creative.creative_state=32");
	    break;

	case 4:
	    sprintf(status, "and creative.creative_state=34");
	    break;

	case 5:
	    sprintf(status, "and creative.creative_state=35");
	    break;
    }

    char balance[128] = "";
    if (balance_type == 2)
	sprintf(balance, "and account.balance > 0");

    char sql_str[40960] = {0};
    sprintf(sql_str, "select count(*) from t_text_creative_append creative left join t_account account on creative.account_id=account.account_id where 1=1 %s %s %s", 
	    name, status, balance);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_img_creative_count(int find_type, char *find_name, int find_status, int balance_type, char *count)
{
    char sql_str[40960] = {0};
    size_t sql_size = sprintf(sql_str ,
	    "select count(*) "
	    "from t_creative creative "
	    "left join t_account account on creative.account_id = account.account_id "
	    "left join t_manager manager on creative.auditor_id = manager.manager_id "
	    "left join t_dsp_baidu_creative dsp on creative.creative_id = dsp.creative_id "
	    "where 1 = 1 ");

    if (balance_type == 2)
	sql_size += sprintf(&sql_str[sql_size], " and account.balance > 0");

    if (find_type == 1)
	sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name like '%%%s%%'", find_name);
    else if (find_type == 2)
	sql_size += sprintf(&sql_str[sql_size], " and account.login_name like '%%%s%%'", find_name);

    switch(find_status) {
	case 1:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=31");
	    break;

	case 2:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=32");
	    break;

	case 4:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=34");
	    break;

	case 5:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=35");
	    break;
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    fprintf(stdout, "1-->%s\n", sql_str);
    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_img_creative(ad_system_sql_t *p,
	int find_type, 
	char *find_name, 
	int find_status, 
	int dsp_status,
	int balance_type,
	int sort_type, 
	int sort_column, 
	int index_s, 
	int index_e)
{

    char sql_str[40960] = {0};
    size_t sql_size = sprintf(sql_str ,
	    "select * from (select rownum as page_index, t.* from ("
	    "select account.account_id,"
	    "account.login_name as account_name,"
	    "creative.creative_id,"
	    "creative.creative_name,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.img_size,"
	    "creative.img_size_imglength,"
	    "creative.file_path,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    "to_char(creative.last_update_time, 'yyyy-mm-dd HH24:MI:SS') as last_update_time,"
	    "manager.login_name,"
	    "to_char(creative.auditor_update_time, 'yyyy-mm-dd HH24:MI:SS') as auditor_update_time,"
	    //"decode(creative.creative_state, 31, nvl(dsp.auditing_state, 1), 35, 2, 1) as dsp_auditing_state,"
	    "nvl(dsp.auditing_state, 3) as dsp_auditing_state,"
	    "dsp.refuse_reason as dsp_refuse_reason "
	    "from t_creative creative "
	    "left join t_account account on creative.account_id = account.account_id "
	    "left join t_manager manager on creative.auditor_id = manager.manager_id "
	    "left join t_dsp_baidu_creative dsp on creative.creative_id = dsp.creative_id "
	    "where 1 = 1 ");

    if (balance_type == 2)
	sql_size += sprintf(&sql_str[sql_size], " and account.balance > 0");

    if (find_type == 1)
	sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name like '%%%s%%'", find_name);
    else if (find_type == 2)
	sql_size += sprintf(&sql_str[sql_size], " and account.login_name like '%%%s%%'", find_name);

    switch(find_status) {
	case 1:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=31");
	    //	    if (dsp_status == 1 || dsp_status == 2 || dsp_status == 3)
	    //		sql_size += sprintf(&sql_str[sql_size], " and dsp.auditing_state=%d", dsp_status - 1);
	    break;

	case 2:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=32");
	    break;

	case 4:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=34");
	    break;

	case 5:
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_state=35");
	    break;

	    //case 100:
	    //	    if (dsp_status == 1 || dsp_status == 2 || dsp_status == 3)
	    //		sql_size += sprintf(&sql_str[sql_size], " and dsp.auditing_state=%d", dsp_status - 1);
	    //break;
    }

    switch(dsp_status) {

	case 1:
	    sql_size += sprintf(&sql_str[sql_size], " and dsp.auditing_state=0");
	    break;

	case 2:
	    sql_size += sprintf(&sql_str[sql_size], " and dsp.auditing_state=1");
	    break;

	case 3:
	    sql_size += sprintf(&sql_str[sql_size], " and dsp.auditing_state=2");
	    break;
    }

    switch(sort_column) {

	case 1:
	    if (sort_type == 1)
		sql_size += sprintf(&sql_str[sql_size], " order by creative_name");
	    else if (sort_type == 2)
		sql_size += sprintf(&sql_str[sql_size], " order by creative_name desc");
	    break;

	case 2:
	    if (sort_type == 1)
		sql_size += sprintf(&sql_str[sql_size], " order by account_name");
	    else if (sort_type == 2)
		sql_size += sprintf(&sql_str[sql_size], " order by account_name desc");
	    break;

	case 3:
	    if (sort_type == 1)
		sql_size += sprintf(&sql_str[sql_size], " order by last_update_time");
	    else if (sort_type == 2)
		sql_size += sprintf(&sql_str[sql_size], " order by last_update_time desc");
	    break;

	case 4:
	    if (sort_type == 1)
		sql_size += sprintf(&sql_str[sql_size], " order by creative_state");
	    else if (sort_type == 2)
		sql_size += sprintf(&sql_str[sql_size], " order by creative_state desc");
	    break;
    }

    sprintf(&sql_str[sql_size], ") t) where page_index between %u and %u",
	    index_s,
	    index_e);

    fprintf(stdout, "2-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_set_text_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason)
{

    size_t sql_size = 0;
    char sql_str[4096] = {0};

    if (creative_status == 31)
	sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u where CREATIVE_ID='%s'", account_id, creative_status, creative_id);

    else if (creative_status == 35)
	sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u, REFUSE_REASON='%s' where CREATIVE_ID='%s'", account_id, creative_status, creative_refuse_reason, creative_id);

    else return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_append_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason)
{

    size_t sql_size = 0;
    char sql_str[4096] = {0};

    if (creative_status == 31)
	sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE_APPEND set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u where APPEND_ID='%s'", account_id, creative_status, creative_id);

    else if (creative_status == 35)
	sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE_APPEND set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u, REFUSE_REASON='%s' where APPEND_ID='%s'", account_id, creative_status, creative_refuse_reason, creative_id);

    else return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_img_creative(char *account_id, char *creative_id, int creative_status, const char *creative_refuse_reason)
{

    size_t sql_size = 0;
    char sql_str[4096] = {0};

    if (creative_status == 31)
	sql_size = sprintf(sql_str, "update T_CREATIVE set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u where CREATIVE_ID='%s'", account_id, creative_status, creative_id);

    else if (creative_status == 35)
	sql_size = sprintf(sql_str, "update T_CREATIVE set LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0,AUDITOR_ID=%s,AUDITOR_UPDATE_TIME=sysdate,CREATIVE_STATE=%u, REFUSE_REASON='%s' where CREATIVE_ID='%s'", account_id, creative_status, creative_refuse_reason, creative_id);

    else return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_manager_account_list_count(char *count,
	int find_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	int status,
	const char *account_id)
{

    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and REGION_NAME_CN like '%%%s%%' ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and %s >= nvl(INVEST - BALANCE, 0) ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and %s <= nvl(INVEST - BALANCE, 0) ", find_name);
		break;
	}
    }

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if ((find_type & 4) && status != 100)
	size += sprintf(&where_str[size], "and ROW_STATE=%u ", status);

    if (find_type & 8)
	size += sprintf(&where_str[size], "and ACCOUNT_ID=%s ", account_id);

    char sql_str[4096] = {0};
    // sprintf(sql_str, "select count(*) from T_ACCOUNT where 1=1 %s", where_str);
    sprintf(sql_str, "select count(*) from T_ACCOUNT where ROW_STATE != 0 %s", where_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_manager_account_to_campaign_status(ad_system_sql_t *sql_conn, const char *account_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "update T_CAMPAIGN set SYNCH_STATE=0, LAST_UPDATE_TIME=sysdate where account_id=%s", account_id);

    if (_ad_system_sql_execute(sql_conn, sql_str))
	return -1; 

    return 0;
}

int ad_system_sql_set_manager_account_status(const char *account_id, int status)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "update T_ACCOUNT set ROW_STATE=%u, SYNCH_STATE=0, LAST_UPDATE_TIME=sysdate where account_id=%s", status, account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    if (status == 2)
	if (ad_system_sql_set_manager_account_to_campaign_status(&sql_conn, account_id)) {

	    ad_system_sql_rollback(&sql_conn);
	    ad_system_sql_free(&sql_conn);
	    return -1;
	}

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_manager_account_info(int update_type,
	const char *account_id	    	  ,
	const char *COMPANY_NAME          ,
	const char *COMPANY_SITE          ,
	const char *INDUSTRY              ,
	const char *PROVINCE_ID           ,
	const char *CITY_ID               ,
	const char *ADDRESS               ,
	const char *POSTCODE              ,
	const char *CONTACT               ,
	const char *TELEPHONE             ,
	const char *FAX                   ,
	const char *MOBILE                ,
	const char *EMAIL                 ,
	const char *WEBIM                 ,
	const char *GUAHAO_TOKEN          ,
	const char *HOSPITAL_IMG          ,
	const char *LOGIN_PASSWORD	  ,
	const char *REGION_NAME_CN        ,
	int STATUS,
	const char *SALES,
	const char *CUSTOMER_SERVICE,
	int PLACE_REGION,
	int JOIN_DSP)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_ACCOUNT set ");
    char comma[4] = "";

    if (update_type & 1) {

	sql_size += sprintf(&sql_str[sql_size], "%sCOMPANY_NAME='%s'",   comma,  COMPANY_NAME  );
	comma[0] = ',';
    }
    if (update_type & 2) {

	char domain_str[4096] = {0};
	sql_size += sprintf(&sql_str[sql_size], "%sDOMAIN='%s'", comma,  ad_system_domain_parse(COMPANY_SITE, domain_str));
	sql_size += sprintf(&sql_str[sql_size], ",COMPANY_SITE='%s'",   COMPANY_SITE);
	comma[0] = ',';
    }
    if (update_type & 4) {

	sql_size += sprintf(&sql_str[sql_size], "%sINDUSTRY='%s'",       comma,  INDUSTRY      );
	comma[0] = ',';
    }
    if (update_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], "%sPROVINCE_ID='%s'",    comma,  PROVINCE_ID   );
	comma[0] = ',';
    }
    if (update_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], "%sCITY_ID='%s'",        comma,  CITY_ID       );
	comma[0] = ',';
    }
    if (update_type & 32) {

	sql_size += sprintf(&sql_str[sql_size], "%sADDRESS='%s'",        comma,  ADDRESS       );
	comma[0] = ',';
    }
    if (update_type & 64) {

	sql_size += sprintf(&sql_str[sql_size], "%sPOSTCODE='%s'",       comma,  POSTCODE      );
	comma[0] = ',';
    }
    if (update_type & 128) {

	sql_size += sprintf(&sql_str[sql_size], "%sCONTACT='%s'",        comma,  CONTACT       );
	comma[0] = ',';
    }
    if (update_type & 256) {

	sql_size += sprintf(&sql_str[sql_size], "%sTELEPHONE='%s'",      comma,  TELEPHONE     );
	comma[0] = ',';
    }
    if (update_type & 512) {

	sql_size += sprintf(&sql_str[sql_size], "%sFAX='%s'",            comma,  FAX           );
	comma[0] = ',';
    }
    if (update_type & 1024) {

	sql_size += sprintf(&sql_str[sql_size], "%sMOBILE='%s'",         comma,  MOBILE        );
	comma[0] = ',';
    }
    if (update_type & 2048) {

	sql_size += sprintf(&sql_str[sql_size], "%sEMAIL='%s'",          comma,  EMAIL         );
	comma[0] = ',';
    }
    if (update_type & 4096) {

	sql_size += sprintf(&sql_str[sql_size], "%sWEBIM='%s'",          comma,  WEBIM         );
	comma[0] = ',';
    }
    if (update_type & 8192) {

	sql_size += sprintf(&sql_str[sql_size], "%sGUAHAO_TOKEN='%s'",   comma,  GUAHAO_TOKEN  );
	comma[0] = ',';
    }
    if (update_type & 16384) {

	sql_size += sprintf(&sql_str[sql_size], "%sHOSPITAL_IMG='%s'",   comma,  HOSPITAL_IMG  );
	comma[0] = ',';
    }
    if (update_type & 32768) {

	sql_size += sprintf(&sql_str[sql_size], "%sLOGIN_PASSWORD='%s'", comma,  LOGIN_PASSWORD);
	sql_size += sprintf(&sql_str[sql_size], ",LOGIN_PASSWORD_MD5=MD5('%s')", LOGIN_PASSWORD);
	comma[0] = ',';
    }
    if (update_type & 65536) {

	sql_size += sprintf(&sql_str[sql_size], "%sREGION_NAME_CN='%s'", comma,  REGION_NAME_CN);
	comma[0] = ',';
    }
    if (update_type & 131072) {

	sql_size += sprintf(&sql_str[sql_size], "%sROW_STATE=%u", comma,  STATUS);
	comma[0] = ',';
    }

    if (update_type & 262144) {

	sql_size += sprintf(&sql_str[sql_size], "%sSALES='%s'", comma,  SALES);
	comma[0] = ',';
    }

    if (update_type & 524288) {

	sql_size += sprintf(&sql_str[sql_size], "%sCUSTOMER_SERVICE='%s'", comma,  CUSTOMER_SERVICE);
	comma[0] = ',';
    }

    if (update_type & 1048576) {

	sql_size += sprintf(&sql_str[sql_size], "%sPLACE_REGION=%u", comma,  PLACE_REGION);
	comma[0] = ',';
    }

    if (update_type & 2097152) {

	sql_size += sprintf(&sql_str[sql_size], "%sJOIN_DSP=%u", comma,  JOIN_DSP);
	comma[0] = ',';
    }

    if (comma[0] == ',') {

	sql_size += sprintf(&sql_str[sql_size], " ,SYNCH_STATE=0,LAST_UPDATE_TIME=sysdate where account_id=%s", account_id);

    } else {

	return -1;
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_manager_account_name_check(const char *account_name)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "select LOGIN_NAME from T_ACCOUNT where row_state != 0 and LOGIN_NAME='%s'", account_name);
    fprintf(stdout, "%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return 100;
    }

    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_manager_new_account_id(char *account_id)
{

    char sql_str[] = "select seq_t_account.nextval from dual";

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(account_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_manager_account(char *session_account_id, int insert_type,
	char *account_id,
	const char *LOGIN_NAME,
	const char *LOGIN_PASSWORD,
	const char *API_TOKEN,
	const char *DOMAIN,
	const char *COMPANY_NAME,
	const char *COMPANY_SITE,
	const char *INDUSTRY,
	const char *PROVINCE_ID,
	const char *CITY_ID,
	const char *ADDRESS,
	const char *POSTCODE,
	const char *CONTACT,
	const char *TELEPHONE,
	const char *FAX,
	const char *MOBILE,
	const char *EMAIL,
	const char *WEBIM,
	const char *GUAHAO_TOKEN,
	const char *REGION_NAME_CN,
	const char *HOSPITAL_IMG,
	const char *SALES,
	const char *CUSTOMER_SERVICE,
	int PLACE_REGION,
	int JOIN_DSP)
{

    char domain_str[4096] = "null";
    char INDUSTRY_STR[1024] = "null";
    char FAX_STR[1024] = "null";
    char WEBIM_STR[1024] = "null";
    char GUAHAO_TOKEN_STR[1024] = "null";
    char HOSPITAL_IMG_STR[1024] = "null";
    char SALES_STR[1024] = "null";
    char CUSTOMER_SERVICE_STR[1024] = "null";

    if (insert_type & 1 )
	sprintf(INDUSTRY_STR, "'%s'", INDUSTRY);
    if (insert_type & 2)
	sprintf(FAX_STR, "'%s'", FAX);
    if (insert_type & 4)
	sprintf(WEBIM_STR, "'%s'", WEBIM);
    if (insert_type & 8)
	sprintf(GUAHAO_TOKEN_STR, "'%s'", GUAHAO_TOKEN);
    if (insert_type & 16)
	sprintf(HOSPITAL_IMG_STR, "'%s'", HOSPITAL_IMG);
    if (insert_type & 32)
	sprintf(SALES_STR, "'%s'", SALES);
    if (insert_type & 64)
	sprintf(CUSTOMER_SERVICE_STR, "'%s'", CUSTOMER_SERVICE);

    char sql_str[40960] = {0};
    sprintf(sql_str,
	    "insert into t_account(ACCOUNT_ID,"
	    "LOGIN_NAME,"
	    "LOGIN_PASSWORD,"
	    "LOGIN_PASSWORD_MD5,"
	    "PROTECT_QUESTION1,"
	    "PROTECT_ANSWER1,"
	    "PROTECT_QUESTION2,"
	    "PROTECT_ANSWER2,"
	    "PROTECT_QUESTION3,"
	    "PROTECT_ANSWER3,"
	    "PROTECT_EMAIL,"
	    "PROTECT_MOBILE,"
	    "API_TOKEN,"
	    "DOMAIN,"
	    "MAXIMP,"
	    "COMPANY_NAME,"
	    "COMPANY_SITE,"
	    "INDUSTRY,"
	    "PROVINCE_ID,"
	    "CITY_ID,"
	    "ADDRESS,"
	    "POSTCODE,"
	    "CONTACT,"
	    "TELEPHONE,"
	    "FAX,"
	    "MOBILE,"
	    "EMAIL,"
	    "BALANCE,"
	    "BRAND_BALANCE,"
	    "INVEST,"
	    "PRICE_RATIO,"
	    "CAMPAIGN_COUNT,"
	    "EFFECTIVE_CAMPAIGN_COUNT,"
	    "ROW_STATE,"
	    "CREATE_TIME,"
	    "CREATE_BY,"
	    "LAST_UPDATE_TIME,"
	    "LAST_UPDATE_BY,"
	    "CREATE_BY_UNAME,"
	    "LAST_UPDATE_BY_UNAME,"
	    "SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "WEBIM,"
	    "GUAHAO_TOKEN,"
	    "LATE_LOGIN_TIME,"
	    "LATE_LOGIN_IP,"
	    "LATE_LOGIN_ADDRESS,"
	    "API_TOKEN_STATE,"
	    "REGION_NAME_CN,"
	    "LOCK_STATE,"
	    "HOSPITAL_IMG,"
	    "SALES,"
	    "CUSTOMER_SERVICE,"
	    "PLACE_REGION,"
	    "JOIN_DSP"
	    ")values(%s,"
	    "'%s',"	//"LOGIN_NAME,"
	    "'%s',"     //"LOGIN_PASSWORD,"
	    "MD5('%s'),"//"LOGIN_PASSWORD_MD5,"
	    "null,"     //"PROTECT_QUESTION1,"
	    "null,"     //"PROTECT_ANSWER1,"
	    "null,"     //"PROTECT_QUESTION2,"
	    "null,"     //"PROTECT_ANSWER2,"
	    "null,"     //"PROTECT_QUESTION3,"
	    "null,"     //"PROTECT_ANSWER3,"
	    "null,"     //"PROTECT_EMAIL,"
	    "null,"     //"PROTECT_MOBILE,"
	    "null,"     //"API_TOKEN,"
	    "'%s',"     //"DOMAIN,"
	    "0,"        //"MAXIMP,"
	    "'%s',"     //"COMPANY_NAME,"
	    "'%s',"     //"COMPANY_SITE,"
	    "%s,"     //"INDUSTRY,"
	    "'%s',"     //"PROVINCE_ID,"
	    "'%s',"     //"CITY_ID,"
	    "'%s',"     //"ADDRESS,"
	    "%s,"       //"POSTCODE,"
	    "'%s',"     //"CONTACT,"
	    "'%s',"     //"TELEPHONE,"
	    "%s,"     //"FAX,"
	    "'%s',"     //"MOBILE,"
	    "'%s',"     //"EMAIL,"
	    "0,"        //"BALANCE,"
	    "0,"        //"BRAND_BALANCE,"
	    "0,"        //"INVEST,"
	    "0,"        //"PRICE_RATIO,"
	    "0,"        //"CAMPAIGN_COUNT,"
	    "0,"        //"EFFECTIVE_CAMPAIGN_COUNT,"
	    "1,"        //"ROW_STATE,"
	    "sysdate,"  //"CREATE_TIME,"
	    "%s,"       //"CREATE_BY,"
	    "sysdate,"  //"LAST_UPDATE_TIME,"
	    "%s,"       //"LAST_UPDATE_BY,"
	    "null,"   //"CREATE_BY_UNAME,"
	    "null,"   //"LAST_UPDATE_BY_UNAME,"
	    "0,"        //"SYNCH_STATE,"
	    "0,"        //"SYNCN_INITIALIZED,"
	    "%s,"     //"WEBIM,"
	    "%s,"     //"GUAHAO_TOKEN,"
	    "sysdate,"  //"LATE_LOGIN_TIME,"
	    "null,"     //"LATE_LOGIN_IP,"
	    "null,"     //"LATE_LOGIN_ADDRESS,"
	    "2,"        //"API_TOKEN_STATE,"
	    "'%s',"     //"REGION_NAME_CN,"
	    "0,"        //"LOCK_STATE,"
	    "%s,"    //HOSPITAL_IMG
	    "%s,"    //所属销售
	    "%s,"	//所属客服
	    "%u,"   //PLACE_REGION
	    "%u)"   //JOIN_DSP
	    ,

	account_id,
	LOGIN_NAME,					//LOGIN_NAME
	LOGIN_PASSWORD,                                 //LOGIN_PASSWORD
	LOGIN_PASSWORD,                                 //LOGIN_PASSWORD_MD5
	ad_system_domain_parse(COMPANY_SITE, domain_str),		//DOMAIN
	COMPANY_NAME,                                   //COMPANY_NAME
	COMPANY_SITE,                                   //COMPANY_SITE
	INDUSTRY_STR,					//INDUSTRY
	PROVINCE_ID,                                    //PROVINCE_ID
	CITY_ID,                                        //CITY_ID
	ADDRESS,                                        //ADDRESS
	POSTCODE,                                       //POSTCODE
	CONTACT,                                        //CONTACT
	TELEPHONE,                                      //TELEPHONE
	FAX_STR,					//FAX
	MOBILE,                                         //MOBILE
	EMAIL,						//EMAIL
	session_account_id,                                     //CREATE_BY
	session_account_id,                                     //LAST_UPDATE_BY
	WEBIM_STR, 					//WEBIM
	GUAHAO_TOKEN_STR,				//GUAHAO_TOKEN
	REGION_NAME_CN,                                 //REGION_NAME_CN
	HOSPITAL_IMG_STR,				//HOSPITAL_IMG
	SALES_STR,						//所属销售
	CUSTOMER_SERVICE_STR,			//所属客服
	PLACE_REGION,
	JOIN_DSP);			

    fprintf(stdout, "%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_del_manager_account(const char *account_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "update t_account set row_state=0 where account_id=%s and INVEST = 0", account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_manager_announcement(const char *session_account_id, const char *send_time, const char *title, const char *content)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, 
	    "insert into T_ANNOUNCEMENT(ANNOUNCEMENT_ID,"
	    "ANNOUNCEMENT_TYPE,"
	    "SEND_DATE,"
	    "ANNOUNCEMENT_TITLE,"
	    "SEND_TIME,"
	    "SEND_STATE,"
	    "ROW_STATE,"
	    "CREATE_TIME,"
	    "CREATE_BY,"
	    "LAST_UPDATE_TIME,"
	    "LAST_UPDATE_BY,"
	    "CREATE_BY_UNAME,"
	    "LAST_UPDATE_BY_UNAME,"
	    "CONTENT"
	    ") values (sys_guid(),"
	    "1,"		//ANNOUNCEMENT_TYPE,"
	    "sysdate,"		//SEND_DATE,"
	    "'%s',"		//ANNOUNCEMENT_TITLE,"
	    "to_date('%s', 'yyyy-mm-dd'),"//SEND_TIME
	    "0,"		//SEND_STATE,"
	    "1,"		//ROW_STATE,"
	    "sysdate,"		//CREATE_TIME,"
	    "%s,"		//CREATE_BY,"
	    "sysdate,"		//LAST_UPDATE_TIME,"
	    "%s,"		//LAST_UPDATE_BY,"
	    "'null',"		//CREATE_BY_UNAME,"     	
	    "'null',"		//LAST_UPDATE_BY_UNAME,"	
	    "'%s')",		//CONTENT

	title,
	send_time,
	session_account_id,
	session_account_id,
	content);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_manager_announcement(int update_type, 
	const char *session_account_id,
	const char *announcement_id,
	const char *send_time, 
	const char *title, 
	const char *content)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_ANNOUNCEMENT set ");
    char comma[4] = "";

    if (update_type & 1) {
	sql_size += sprintf(&sql_str[sql_size], "%sSEND_TIME=to_date('%s', 'yyyy-mm-dd') ", comma, send_time);
	comma[0] = ',';
    }

    if (update_type & 2) {
	sql_size += sprintf(&sql_str[sql_size], "%sANNOUNCEMENT_TITLE='%s' ", comma, title);
	comma[0] = ',';
    }

    if (update_type & 4) {
	sql_size += sprintf(&sql_str[sql_size], "%sCONTENT='%s' ", comma, content);
	comma[0] = ',';
    }

    if (comma[0] == ',') {
	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,LAST_UPDATE_BY=%s where ANNOUNCEMENT_ID='%s'",
		session_account_id, announcement_id);

    } else {

	return -1;
    }

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_del_manager_announcement(const char *announcement_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "delete from T_ANNOUNCEMENT where ANNOUNCEMENT_ID='%s'", announcement_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_manager_announcement_list_count(char *count,
	int find_type,
	const char *find_name,
	int time_type,
	const char *time_start,
	const char *time_end,
	int status)
{

    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1)
	size += sprintf(&where_str[size], "and ANNOUNCEMENT.ANNOUNCEMENT_TITLE like '%%%s%%' ", find_name);

    if (find_type & 2) {

	if (time_type == 1) {

	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.SEND_TIME >= to_date('%s', 'yyyy-mm-dd') ", time_start);
	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.SEND_TIME <= to_date('%s  23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", 
		    time_end);

	} else {

	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.CREATE_TIME >= to_date('%s', 'yyyy-mm-dd') ", time_start);
	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.CREATE_TIME <= to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", 
		    time_end);
	}
    }

    if (find_type & 4) {

	if (status == 1) 
	    size += sprintf(&where_str[size], "and trunc(sysdate) < trunc(ANNOUNCEMENT.SEND_TIME) ");
	if (status == 2) 
	    size += sprintf(&where_str[size], "and trunc(sysdate) >= trunc(ANNOUNCEMENT.SEND_TIME) ");
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "select count(*) from T_ANNOUNCEMENT ANNOUNCEMENT where 1=1 %s", where_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_manager_announcement_list(ad_system_sql_t *p,
	int find_type,
	const char *find_name,
	int time_type,
	const char *time_start,
	const char *time_end,
	int status,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "ANNOUNCEMENT.ANNOUNCEMENT_TITLE");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "ANNOUNCEMENT.SEND_TIME");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "ANNOUNCEMENT.CREATE_TIME");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "ANNOUNCEMENT.SEND_STATE");
	    break;

    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");

    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1)
	size += sprintf(&where_str[size], "and ANNOUNCEMENT.ANNOUNCEMENT_TITLE like '%%%s%%' ", find_name);

    if (find_type & 2) {

	if (time_type == 1) {

	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.SEND_TIME >= to_date('%s', 'yyyy-mm-dd') ", time_start);
	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.SEND_TIME <= to_date('%s  23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", 
		    time_end);

	} else {

	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.CREATE_TIME >= to_date('%s', 'yyyy-mm-dd') ", time_start);
	    size += sprintf(&where_str[size], "and ANNOUNCEMENT.CREATE_TIME <= to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", 
		    time_end);
	}
    }

    if (find_type & 4) {

	if (status == 1) 
	    size += sprintf(&where_str[size], "and trunc(sysdate) < trunc(ANNOUNCEMENT.SEND_TIME) ");
	if (status == 2) 
	    size += sprintf(&where_str[size], "and trunc(sysdate) >= trunc(ANNOUNCEMENT.SEND_TIME) ");
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "ANNOUNCEMENT_ID,"
	    "ANNOUNCEMENT_TITLE,"
	    "SEND_TIME,"
	    "SEND_STATE,"
	    "CREATE_TIME,"
	    "CONTENT,"
	    "CREATE_NAME,"
	    "UPDATE_NAME,"
	    "UPDATE_TIME "
	    "from (select rownum as page_index,"
	    "nvl(to_char(ANNOUNCEMENT.ANNOUNCEMENT_ID), 'null') as ANNOUNCEMENT_ID,"
	    "nvl(to_char(ANNOUNCEMENT.ANNOUNCEMENT_TITLE), 'null') as ANNOUNCEMENT_TITLE,"
	    "nvl(to_char(ANNOUNCEMENT.SEND_TIME, 'yyyy-mm-dd'),'null') as SEND_TIME,"
	    "decode(sign(to_date(to_char(ANNOUNCEMENT.SEND_TIME, 'yyyy-mm-dd'),'yyyy-mm-dd')-sysdate),-1, 2, 1) as SEND_STATE, "
	    "nvl(to_char(ANNOUNCEMENT.CREATE_TIME, 'yyyy-mm-dd HH24:MI:SS'),'null') as CREATE_TIME,"
	    "nvl(to_char(ANNOUNCEMENT.CONTENT), 'null') as CONTENT,"
	    "nvl(to_char(MANAGER.LOGIN_NAME), 'null') as CREATE_NAME,"
	    "nvl(to_char(MANAGER2.LOGIN_NAME), 'null') as UPDATE_NAME,"
	    "nvl(to_char(ANNOUNCEMENT.LAST_UPDATE_TIME, 'yyyy-mm-dd HH24:MI:SS'), 'null') as UPDATE_TIME "
	    "from T_ANNOUNCEMENT ANNOUNCEMENT "
	    "left join T_MANAGER MANAGER on ANNOUNCEMENT.CREATE_BY = MANAGER.MANAGER_ID "
	    "left join T_MANAGER MANAGER2 on ANNOUNCEMENT.LAST_UPDATE_BY = MANAGER2.MANAGER_ID "
	    "where 1=1 %s order by %s) t "
	    "where t.page_index between %u and %u",

	where_str,
	sort_str,
	page_index_s,
	page_index_e);

    fprintf(stdout, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_money_operation(ad_system_sql_t *p,
	const char *account_id,
	int find_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	int status,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    char page_str[1024] = {0};
    if (page_index_s && page_index_e)
	sprintf(page_str, "where t.page_index between %u and %u", page_index_s, page_index_e);

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "CREATE_TIME");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "LOGIN_NAME");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "COMPANY_NAME");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "INVEST");
	    break;

	case 5:
	    sort_size = sprintf(sort_str, "BALANCE");
	    break;

	case 6:
	    sort_size = sprintf(sort_str, "ROW_STATE");
	    break;
    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");


    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type != 100 && find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and nvl(INVEST, 0) >= %s ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and nvl(INVEST, 0) <= %s ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and nvl(BALANCE, 0) >= %s ", find_name);
		break;

	    case 7:
		size += sprintf(&where_str[size], "and nvl(BALANCE, 0) <= %s ", find_name);
		break;
	}
    }

    if (find_type != 100 && find_type & 2) {

	size += sprintf(&where_str[size], "and CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if ((find_type != 100 && find_type & 4) && status != 100)
	size += sprintf(&where_str[size], "and ROW_STATE=%u ", status);

    if (find_type != 100 && find_type & 16)
	size += sprintf(&where_str[size], "and nvl(INVEST, 0) != 0 ");

    if (account_id && *account_id)
	size += sprintf(&where_str[size], "and ACCOUNT_ID=%s", account_id);

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "ACCOUNT_ID_STR,"
	    "LOGIN_NAME_STR,"
	    "COMPANY_NAME_STR,"
	    "COMPANY_SITE_STR,"
	    "BALANCE_STR,"
	    "INVEST_STR,"
	    "CREATE_TIME_STR,"
	    "ROW_STATE_STR "
	    "from (select "
	    "rownum as page_index,"
	    "nvl(to_char(ACCOUNT_ID), 'null') as ACCOUNT_ID_STR,"
	    "nvl(to_char(LOGIN_NAME), 'null') as LOGIN_NAME_STR,"
	    "nvl(to_char(COMPANY_NAME), 'null') as COMPANY_NAME_STR,"
	    "nvl(to_char(COMPANY_SITE), 'null') as COMPANY_SITE_STR,"
	    "nvl(to_char(BALANCE), 'null') as BALANCE_STR,"
	    "nvl(to_char(INVEST), 'null') as INVEST_STR,"
	    "nvl(to_char(CREATE_TIME, 'yyyy-mm-dd'), 'null') as CREATE_TIME_STR,"
	    "nvl(to_char(ROW_STATE), 'null') as ROW_STATE_STR "
	    "from T_ACCOUNT "
	    "where ROW_STATE != 0 %s order by %s) t %s",

	where_str,
	sort_str,
	page_str);

    fprintf(stdout, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_money_operation_count(char *count,
	char *INVEST,
	char *BALANCE,
	char *CONSUMPTION,
	int find_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	int status)
{

    size_t size = 0;
    char where_str[4096] = {0};

    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and nvl(INVEST, 0) >= %s ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and nvl(INVEST, 0) <= %s ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and nvl(BALANCE, 0) >= %s ", find_name);
		break;

	    case 7:
		size += sprintf(&where_str[size], "and nvl(BALANCE, 0) <= %s ", find_name);
		break;
	}
    }

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if ((find_type & 4) && status != 100)
	size += sprintf(&where_str[size], "and ROW_STATE=%u ", status);

    if (find_type & 16)
	size += sprintf(&where_str[size], "and nvl(INVEST, 0) != 0 ");

    char sql_str[40960] = {0};
    sprintf(sql_str,	"select  count(*), round(sum(invest), 2), round(sum(balance), 2), round(sum(invest - balance), 2)  from t_account where ROW_STATE != 0 %s",
	    where_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }


    const char *_count = ad_system_sql_get_string(&sql_conn, NULL, 1);
    const char *_INVEST = ad_system_sql_get_string(&sql_conn, NULL, 2);
    const char *_BALANCE = ad_system_sql_get_string(&sql_conn, NULL, 3);
    const char *_CONSUMPTION = ad_system_sql_get_string(&sql_conn, NULL, 4);

    if (_count)
	strcpy(count, _count);

    if (_INVEST)
	strcpy(INVEST, _INVEST);

    if (_BALANCE)
	strcpy(BALANCE, _BALANCE);

    if (_CONSUMPTION)
	strcpy(CONSUMPTION, _CONSUMPTION);

    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_money_list_count(char *count,
	int find_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	const char *account_id)
{

    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and ACCOUNT.LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and ACCOUNT.COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and ACCOUNT.COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and RECHARGE.SWIFT_NUMBER like '%%%s%%' ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and %s = RECHARGE.MONEY ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and %s >= RECHARGE.MONEY ", find_name);
		break;

	    case 7:
		size += sprintf(&where_str[size], "and %s <= RECHARGE.MONEY ", find_name);
		break;
	}
    }

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and RECHARGE.CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and RECHARGE.CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if (find_type & 8)
	size += sprintf(&where_str[size], "and RECHARGE.Account_Id=%s ", account_id);

    char sql_str[4096] = {0};
    sprintf(sql_str, "select count(*) "
	    "from T_RECHARGE RECHARGE "
	    "left join T_ACCOUNT ACCOUNT on RECHARGE.Account_Id=ACCOUNT.Account_Id "
	    "where 1 = 1 %s ",
	    where_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_money_list(ad_system_sql_t *p,
	int find_type,
	int money_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	const char *account_id,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "RECHARGE.CREATE_TIME");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "ACCOUNT.Login_Name");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "ACCOUNT.Company_Name");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "RECHARGE.SWIFT_NUMBER");
	    break;

	case 5:
	    sort_size = sprintf(sort_str, "RECHARGE.MONEY");
	    break;

	case 6:
	    sort_size = sprintf(sort_str, "RECHARGE.INVEST");
	    break;

	case 7:
	    sort_size = sprintf(sort_str, "RECHARGE.CURRENT_MONEY_BALANCE");
	    break;
    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");


    size_t size = 0;
    char where_str[1024] = {0};

    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and ACCOUNT.LOGIN_NAME like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and ACCOUNT.COMPANY_NAME like '%%%s%%' ", find_name);
		break;

	    case 3:
		size += sprintf(&where_str[size], "and ACCOUNT.COMPANY_SITE like '%%%s%%' ", find_name);
		break;

	    case 4:
		size += sprintf(&where_str[size], "and RECHARGE.SWIFT_NUMBER like '%%%s%%' ", find_name);
		break;

	    case 5:
		size += sprintf(&where_str[size], "and %s = RECHARGE.MONEY ", find_name);
		break;

	    case 6:
		size += sprintf(&where_str[size], "and %s <= RECHARGE.MONEY ", find_name);
		break;

	    case 7:
		size += sprintf(&where_str[size], "and %s >= RECHARGE.MONEY ", find_name);
		break;

	    case 8:
		size += sprintf(&where_str[size], "and %s <= RECHARGE.ACCOUNTING_DOCUMENT_NUMBER", find_name);
		break;
	}
    }

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and RECHARGE.CREATE_TIME > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and RECHARGE.CREATE_TIME < to_date('%s 23:59:59', 'yyyy-mm-dd HH24:MI:SS') ", time_end);
    }

    if (find_type & 8 && account_id)
	size += sprintf(&where_str[size], "and RECHARGE.Account_Id=%s ", account_id);

    if (money_type != 100)
	size += sprintf(&where_str[size], "and PAYMENT_TYPE=%u", money_type);

    char page_str[1024] = {0};
    if (page_index_s && page_index_e)
	sprintf(page_str, "where t.page_index between %u and %u",
		page_index_s,
		page_index_e);

    char sql_str[4096] = {0};
    sprintf(sql_str, "select * from ("
	    "select "
	    "ACCOUNT_ID,"
	    "ACCOUNT_NAME,"
	    "COMPANY_NAME,"
	    "COMPANY_SITE,"
	    "SWIFT_NUMBER,"
	    "MONEY,"
	    "CURRENT_MONEY_BALANCE,"
	    "INVEST,"
	    "PAYMENT_TYPE,"
	    "ACCOUNTING_DOCUMENT_NUMBER,"
	    "ACCOUNTING_DOCUMENT_FILE,"
	    "ACCOUNTING_DOCUMENT_REMARK,"
	    "CREATE_TIME,"
	    "rownum as page_index "
	    "from (select "
	    "nvl(to_char(RECHARGE.ACCOUNT_ID), 'null') as ACCOUNT_ID,"
	    "nvl(to_char(ACCOUNT.Login_Name), 'null') as ACCOUNT_NAME,"
	    "nvl(to_char(ACCOUNT.Company_Name), 'null') as COMPANY_NAME,"
	    "nvl(to_char(ACCOUNT.COMPANY_SITE), 'null') as COMPANY_SITE,"
	    "nvl(to_char(RECHARGE.SWIFT_NUMBER), 'null') as SWIFT_NUMBER,"
	    "nvl(to_char(RECHARGE.MONEY), 'null') as MONEY,"
	    "nvl(to_char(RECHARGE.CURRENT_MONEY_BALANCE), 'null') as CURRENT_MONEY_BALANCE,"
	    "nvl(to_char(RECHARGE.INVEST), 'null') as INVEST,"
	    "nvl(to_char(RECHARGE.PAYMENT_TYPE), 'null') as PAYMENT_TYPE,"
	    "nvl(to_char(RECHARGE.ACCOUNTING_DOCUMENT_NUMBER), 'null') as ACCOUNTING_DOCUMENT_NUMBER,"
	    "nvl(to_char(RECHARGE.ACCOUNTING_DOCUMENT_FILE), 'null') as ACCOUNTING_DOCUMENT_FILE,"
	    "nvl(to_char(RECHARGE.ACCOUNTING_DOCUMENT_REMARK), 'null') as ACCOUNTING_DOCUMENT_REMARK,"
	    "nvl(to_char(RECHARGE.CREATE_TIME, 'yyyy-mm-dd'), 'null') as CREATE_TIME "
	    "from T_RECHARGE RECHARGE "
	    "left join T_ACCOUNT ACCOUNT on RECHARGE.Account_Id=ACCOUNT.Account_Id "
	    "where 1 = 1 %s order by %s)) t %s",

	where_str,
	sort_str,
	page_str);

    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_add_brand_money_operation_update_account(ad_system_sql_t *sql_conn, char *ACCOUNT_ID, char *MONEY)
{
    char sql_str[4096] = {0};
    sprintf(sql_str, "update t_account set INVEST=INVEST+%s, BRAND_BALANCE=BRAND_BALANCE+%s where account_id=%s", 
	    MONEY, 
	    MONEY, 
	    ACCOUNT_ID);

    fprintf(stderr, "%s\n", sql_str);
    if (_ad_system_sql_execute(sql_conn, sql_str))
	return -1; 

    return 0;
}

int ad_system_sql_add_money_operation_update_account(ad_system_sql_t *sql_conn, char *ACCOUNT_ID, char *MONEY)
{
    char sql_str[4096] = {0};
    sprintf(sql_str, "update t_account set INVEST=INVEST+%s, BALANCE=BALANCE+%s where account_id=%s", 
	    MONEY, 
	    MONEY, 
	    ACCOUNT_ID);

    if (_ad_system_sql_execute(sql_conn, sql_str))
	return -1; 

    return 0;
}

int campaign_region_check_r(const char *region)
{

    char buffer[40960];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(region, buffer, &save_ptr, ',');

    while(str) {

	if (strlen(str) == 10) {

	    if (atoi(&str[5]))
		return 1;
	}

	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return 0;
}

int campaign_region_check(const char *account_id)
{
    char sql_str[4096] = {0};
    sprintf(sql_str, "select region_serialize from t_campaign where account_id=%s and region_type=1", account_id);
    fprintf(stdout, "%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    while(ad_system_sql_fetch_next(&sql_conn)) {

	const char *region = ad_system_sql_get_string(&sql_conn, NULL, 1);
	if (campaign_region_check_r(region)) {

	    ad_system_sql_free(&sql_conn);
	    return -1;
	}
    }

    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_brand(ad_system_sql_t *p,
	int find_type,
	const char *find_name,
	int brand_status, 
	int DEPT_TYPE, 
	const char *DEPT_ARRAY, 
	int REGION_TYPE, 
	const char *REGION_ARRAY,
	int SHOW_TIME_TYPE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	int CREATE_TIME_TYPE,
	const char *CREATE_TIME_S,
	const char *CREATE_TIME_E,
	int SORT_TYPE,
	int SORT_COLUMN,
	int BALANCE_TYPE,
	int page_index_s,
	int page_index_e)
{

    size_t size = 0;
    char where_str[1024] = {0};
    if (brand_status != 100)
	size += sprintf(&where_str[size], " and status = %d", brand_status);

    if (SHOW_TIME_TYPE == 2) {
	size += sprintf(&where_str[size], " and begin_time <= to_date('%s', 'yyyy-mm-dd') ", SHOW_TIME_E);
	size += sprintf(&where_str[size], " and end_time >= to_date('%s', 'yyyy-mm-dd') ", SHOW_TIME_S);
    }

    if (CREATE_TIME_TYPE == 2) {

	size += sprintf(&where_str[size], " and brand_create_time >= to_date('%s', 'yyyy-mm-dd') ", CREATE_TIME_S);
	size += sprintf(&where_str[size], " and brand_create_time <= to_date('%s', 'yyyy-mm-dd') ", CREATE_TIME_E);
    }

    if (find_name && *find_name) {

	if (find_type == 1)
	    size += sprintf(&where_str[size], " and ad_order_no like '%%%s%%'", find_name);
	if (find_type == 2)
	    size += sprintf(&where_str[size], " and account_name like '%%%s%%'", find_name);
    }


    //地域与病种条件
    char dept_region_str[1024] = {0};
    if (DEPT_TYPE == 2 && REGION_TYPE == 2)
	sprintf(dept_region_str, "select dept.brand_ad_id"
		" from t_brand_ad_dept dept"
		" cross join t_brand_ad_region region"
		" where dept.dept_code in (%s)"
		" and region.region_code in (%s)"
		" group by dept.brand_ad_id) t left join (",
		DEPT_ARRAY,
		REGION_ARRAY);
    else if (DEPT_TYPE == 2)
	sprintf(dept_region_str, "select dept.brand_ad_id"
		" from t_brand_ad_dept dept"
		" where dept.dept_code in (%s)"
		" group by dept.brand_ad_id) t left join (",
		DEPT_ARRAY);
    else if (REGION_TYPE == 2)
	sprintf(dept_region_str, "select region.brand_ad_id"
		" from t_brand_ad_region region"
		" where region.region_code in (%s)"
		" group by region.brand_ad_id) t left join (",
		REGION_ARRAY);

    //隐藏已失效订单
    if (BALANCE_TYPE == 2)
	size += sprintf(&where_str[size], " and (status=1 or status=3 or status=4)");

    //排序 1订单 2登录名 3投放时长 4开始日期 5结束日期 6广告消费 7创建时间 8状态
    size = 0;
    char sort_str[512] = {0};
    switch(SORT_COLUMN){

	case 1:
	    size += sprintf(&sort_str[size], "ad_order_no");
	    break;
	case 2:
	    size += sprintf(&sort_str[size], "account_name");
	    break;
	case 3:
	    size += sprintf(&sort_str[size], "days");
	    break;
	case 4:
	    size += sprintf(&sort_str[size], "begin_time");
	    break;
	case 5:
	    size += sprintf(&sort_str[size], "end_time");
	    break;
	case 6:
	    size += sprintf(&sort_str[size], "money");
	    break;
	case 7:
	    size += sprintf(&sort_str[size], "brand_create_time");
	    break;
	case 8:
	    size += sprintf(&sort_str[size], "status");
	    break;
    }

    if (SORT_TYPE == 2)
	sprintf(&sort_str[size], " desc");

    char sql_str[8192] = {0};
    sprintf(sql_str, "select *"
	    "from (select brand_ad_id,"
	    " ad_order_no,"
	    " account_id,"
	    " account_name,"
	    " dept_code,"
	    " region_type,"
	    " region_serialize,"
	    " days,"
	    " begin_time,"
	    " end_time,"
	    " money,"
	    " to_char(brand_create_time, 'YYYY-MM-DD') as b_create_time,"
	    " status,"
	    " brand_refuse_reason,"
	    " auditor_update_time,"
	    " auditor_name,"
	    " rownum as page_index"
	    " from (select %sbrand_ad_id,"
	    " ad_order_no,"
	    " account_id,"
	    " account_name,"
	    " dept_code,"
	    " region_type,"
	    " region_serialize,"
	    " days,"
	    " begin_time,"
	    " end_time,"
	    " money,"
	    " brand_create_time,"
	    " status,"
	    " brand_refuse_reason,"
	    " auditor_update_time,"
	    " auditor_name"
	    " from (%s"
	    " select brand.brand_ad_id,"
	    " brand.ad_order_no,"
	    " brand.account_id,"
	    " account.login_name as account_name,"
	    " brand.dept_code,"
	    " brand.region_type,"
	    " brand.region_serialize,"
	    " brand.end_time - brand.begin_time + 1 as days,"
	    " brand.begin_time,"
	    " brand.end_time,"
	    " brand.price as money,"
	    " brand.create_time as brand_create_time,"
	    " decode(brand.ad_state,"
	    " 0,"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.begin_time),"
	    " -1,"
	    " 4,"// --申请中"
	    " 5),"// --已过期"
	    " 1,"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.begin_time),"
	    " -1,"
	    " 3,"// --待投放"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.end_time),"
	    " 1,"
	    " 2,"// --已结束"
	    " 1)),"// --投放中"
	    " 6) as status,"// --申请拒绝"
	    " brand.refuse_reason as brand_refuse_reason,"
	    " brand.auditor_update_time,"
	    " manager.login_name as auditor_name"
	    " from t_brand_ad brand"
	    " left join t_account account on brand.account_id=account.account_id"
	    " left join t_manager manager on brand.auditor_id=manager.manager_id"
	    " where brand.row_state = 1) %s"
	    " where 1=1 %s order by %s))"
	    " where page_index between %u and %u",

	*dept_region_str ? " t." : "",
	dept_region_str,
	*dept_region_str ? " tt on t.brand_ad_id=tt.brand_ad_id" : "",
	where_str,
	sort_str,
	page_index_s,
	page_index_e
	    );

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_brand_count(char *count,
	int find_type,
	const char *find_name,
	int brand_status, 
	int DEPT_TYPE, 
	const char *DEPT_ARRAY, 
	int REGION_TYPE, 
	const char *REGION_ARRAY,
	int SHOW_TIME_TYPE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	int CREATE_TIME_TYPE,
	const char *CREATE_TIME_S,
	const char *CREATE_TIME_E,
	int BALANCE_TYPE)
{

    size_t size = 0;
    char where_str[1024] = {0};
    if (brand_status != 100)
	size += sprintf(&where_str[size], " and status = %d", brand_status);

    if (SHOW_TIME_TYPE == 2) {
	size += sprintf(&where_str[size], " and begin_time <= to_date('%s', 'yyyy-mm-dd') ", SHOW_TIME_E);
	size += sprintf(&where_str[size], " and end_time >= to_date('%s', 'yyyy-mm-dd') ", SHOW_TIME_S);
    }

    if (CREATE_TIME_TYPE == 2) {

	size += sprintf(&where_str[size], " and brand_create_time >= to_date('%s', 'yyyy-mm-dd') ", CREATE_TIME_S);
	size += sprintf(&where_str[size], " and brand_create_time <= to_date('%s', 'yyyy-mm-dd') ", CREATE_TIME_E);
    }

    if (find_name && *find_name) {

	if (find_type == 1)
	    size += sprintf(&where_str[size], " and ad_order_no like '%%%s%%'", find_name);
	if (find_type == 2)
	    size += sprintf(&where_str[size], " and account_name like '%%%s%%'", find_name);
    }


    //地域与病种条件
    char dept_region_str[1024] = {0};
    if (DEPT_TYPE == 2 && REGION_TYPE == 2)
	sprintf(dept_region_str, "select dept.brand_ad_id"
		" from t_brand_ad_dept dept"
		" cross join t_brand_ad_region region"
		" where dept.dept_code in (%s)"
		" and region.region_code in (%s)"
		" group by dept.brand_ad_id) t left join (",
		DEPT_ARRAY,
		REGION_ARRAY);
    else if (DEPT_TYPE == 2)
	sprintf(dept_region_str, "select dept.brand_ad_id"
		" from t_brand_ad_dept dept"
		" where dept.dept_code in (%s)"
		" group by dept.brand_ad_id) t left join (",
		DEPT_ARRAY);
    else if (REGION_TYPE == 2)
	sprintf(dept_region_str, "select region.brand_ad_id"
		" from t_brand_ad_region region"
		" where region.region_code in (%s)"
		" group by region.brand_ad_id) t left join (",
		REGION_ARRAY);

    //隐藏已失效订单
    if (BALANCE_TYPE == 2)
	size += sprintf(&where_str[size], " and (status=1 or status=3 or status=4)");

    char sql_str[8192] = {0};
    sprintf(sql_str, 
	    " select count(*)"
	    " from (select brand_ad_id,"
	    " ad_order_no,"
	    " account_id,"
	    " account_name,"
	    " dept_code,"
	    " region_type,"
	    " region_serialize,"
	    " days,"
	    " begin_time,"
	    " end_time,"
	    " money,"
	    " to_char(brand_create_time, 'YYYY-MM-DD') as b_create_time,"
	    " status,"
	    " brand_refuse_reason,"
	    " auditor_update_time,"
	    " auditor_name,"
	    " rownum as page_index"
	    " from (select %sbrand_ad_id,"
	    " ad_order_no,"
	    " account_id,"
	    " account_name,"
	    " dept_code,"
	    " region_type,"
	    " region_serialize,"
	    " days,"
	    " begin_time,"
	    " end_time,"
	    " money,"
	    " brand_create_time,"
	    " status,"
	    " brand_refuse_reason,"
	    " auditor_update_time,"
	    " auditor_name"
	    " from (%s"
	    " select brand.brand_ad_id,"
	    " brand.ad_order_no,"
	    " brand.account_id,"
	    " account.login_name as account_name,"
	    " brand.dept_code,"
	    " brand.region_type,"
	    " brand.region_serialize,"
	    " brand.end_time - brand.begin_time + 1 as days,"
	    " brand.begin_time,"
	    " brand.end_time,"
	    " brand.price as money,"
	    " brand.create_time as brand_create_time,"
	    " decode(brand.ad_state,"
	    " 0,"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.begin_time),"
	    " -1,"
	    " 4,"// --申请中"
	    " 5),"// --已过期"
	    " 1,"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.begin_time),"
	    " -1,"
	    " 3,"// --待投放"
	    " decode(sign(TRUNC(sysdate) -"
	    " brand.end_time),"
	    " 1,"
	    " 2,"// --已结束"
	    " 1)),"// --投放中"
	    " 6) as status,"// --申请拒绝"
	    " brand.refuse_reason as brand_refuse_reason,"
	    " brand.auditor_update_time,"
	    " manager.login_name as auditor_name"
	    " from t_brand_ad brand"
	    " left join t_account account on brand.account_id=account.account_id"
	    " left join t_manager manager on brand.auditor_id=manager.manager_id"
	    " where brand.row_state = 1) %s"
	    " where 1=1 %s))",

	*dept_region_str ? " t." : "",
	dept_region_str,
	*dept_region_str ? " tt on t.brand_ad_id=tt.brand_ad_id" : "",
	where_str
	    );

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}


int ad_system_sql_find_brand_schedule(ad_system_sql_t *p,
	int DEPT_TYPE,
	const char *DEPT_ARRAY,
	int REGION_TYPE,
	const char *REGION_ARRAY,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	int page_index_s,
	int page_index_e,
	int page_type)
{

    size_t size = 0;
    char where_str[409600] = {0};

    size_t size2 = 0;
    char where_str2[409600] = {0};

    char page_str[4096] = {0};
    if (page_type != 1)
	sprintf(page_str, " where page_index between %u and %u",
		page_index_s,
		page_index_e);

    if (DEPT_TYPE == 2) {
	size += sprintf(&where_str[size], " and dept.code in (%s)", DEPT_ARRAY);
	size2 += sprintf(&where_str2[size2], " and dept2.dept_code in (%s)", DEPT_ARRAY);
    }

    if (REGION_TYPE == 2) {
	size += sprintf(&where_str[size], " and region.code in (%s)", REGION_ARRAY);
	size2 += sprintf(&where_str2[size2], " and region2.region_code in (%s)", REGION_ARRAY);
    }

    char sql_str[409600] = {0};
    sprintf(sql_str, "select * from ("
	    "select dept_code, region_code, decode(tt.union_code, null, 2, 1) as status, rownum as page_index"
	    " from (select dept.code as dept_code,"
	    "             region.code as region_code,"
	    "             dept.code || region.code as union_code"
	    "        from t_category dept, t_sys_region region"
	    "       where region.code <> 'NULL' %s) t"
	    " left join (select dept2.dept_code || region2.region_code as union_code"
	    "             from t_brand_ad brand,"
	    "                  t_brand_ad_dept dept2,"
	    "                  t_brand_ad_region region2"
	    "            where brand.ad_state = 1"
	    "              and brand.row_state = 1"
	    "		   and brand.begin_time <= to_date('%s', 'YYYY-MM-DD')"
	    "		   and brand.end_time >= to_date('%s', 'YYYY-MM-DD')"
	    "              and brand.brand_ad_id = dept2.brand_ad_id"
	    "              and brand.brand_ad_id = region2.brand_ad_id"
	    " %s) tt on t.union_code = tt.union_code)"
	    " %s",

	    where_str,
	    SHOW_TIME_E,
	    SHOW_TIME_S,
	    where_str2,
	    page_str);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_find_brand_schedule_count(char *count,
	int DEPT_TYPE,
	const char *DEPT_ARRAY,
	int REGION_TYPE,
	const char *REGION_ARRAY)
{

    size_t size = 0;
    char where_str[409600] = {0};

    if (DEPT_TYPE == 2)
	size += sprintf(&where_str[size], "and dept.code in (%s)", DEPT_ARRAY);

    if (REGION_TYPE == 2)
	size += sprintf(&where_str[size], "and region.code in (%s)", REGION_ARRAY);

    char sql_str[409600] = {0};
    sprintf(sql_str,
	    "select count(*)"
	    " from t_category dept, t_sys_region region"
	    " where region.code <> 'NULL' %s",
	    where_str);

    fprintf(stderr, "-->%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取科室病种CODE
int ad_system_sql_get_disease_code(ad_system_sql_t *p)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select title, code from t_category order by code");
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_set_brand_creative(ad_system_sql_t *p, const char *brand_id, const char *auditor_id, int status, const char *REFUSE_REASON)
{

    char sql_str[1024];
    if (status == 3) {//审核拒绝

	sprintf(sql_str, "update t_brand_creative_temp t set t.auditor_id=%s, t.creative_state=2, REFUSE_REASON='%s'  where t.brand_ad_id in (%s)", 
		auditor_id, REFUSE_REASON, brand_id);
	if (_ad_system_sql_execute(p, sql_str))
	    return -1;

    } else if (status == 2) {//审核通过

	// 更新创意状态
	sprintf(sql_str, "update t_brand_creative_temp t set t.auditor_id=%s, t.creative_state=1 where t.brand_ad_id in (%s)", auditor_id, brand_id);
	fprintf(stderr, "==>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str))
	    return -1;

	// 删除(修改前表)的这个创意
	sprintf(sql_str, "delete from t_brand_creative t where t.brand_ad_id in (%s)", brand_id);
	fprintf(stderr, "==>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str)) {

	    ad_system_sql_rollback(p);
	    return -1;
	}

	// 拷贝创意至(修改前表)
	sprintf(sql_str, "insert into t_brand_creative( BRAND_CREATIVE_ID, BRAND_AD_ID, ACCOUNT_ID, TITLE, DESCRIPTION, DESTINATION_URL, DISPLAY_URL, TELEPHONE, WEB_SITE, WEB_IM, CREATIVE_STATE, AUDITOR_ID, AUDITOR_UPDATE_TIME, REFUSE_REASON, CREATE_TIME, LAST_UPDATE_TIME, ROW_STATE, GUAHAO_URL) select * from t_brand_creative_temp t where t.brand_ad_id in (%s)", brand_id);
	fprintf(stderr, "==>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str)) {

	    ad_system_sql_rollback(p);
	    return -1;
	}

	// 删除(修改前表)附件创意
	sprintf(sql_str, 
		"delete from t_brand_creative_append t where t.brand_creative_id in (select tt.brand_creative_id from t_brand_creative tt where tt.brand_ad_id in (%s))",
		brand_id);
	fprintf(stderr, "==>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str)) {

	    ad_system_sql_rollback(p);
	    return -1;
	}

	// 拷贝附加创意至(修改前表)
	sprintf(sql_str,
		"insert into t_brand_creative_append"
		"  (BRAND_CREATIVE_APPEND_ID,"
		"   BRAND_CREATIVE_ID,"
		"   IMG_PATH,"
		"   IMG_SIZE,"
		"   URL,"
		"   SEQUENCE)"
		"  select BRAND_CREATIVE_APPEND_ID,"
		"         BRAND_CREATIVE_ID,"
		"         IMG_PATH,"
		"         IMG_SIZE,"
		"         URL,"
		"         SEQUENCE"
		"    from t_brand_creative_append_temp t"
		"   where t.brand_creative_id in" 
		"         (select tt.brand_creative_id"
		"            from t_brand_creative tt"
		"           where tt.brand_ad_id in (%s))",
		brand_id);
	fprintf(stderr, "==>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str)) {

	    ad_system_sql_rollback(p);
	    return -1;
	}
    }

    ad_system_sql_commit(p);
    return 0;
}

int ad_system_sql_get_brand_creative_count(char *count,
	int find_type, 
	const char *find_name,
	int brand_id_type,
	const char *brand_id,
	int status,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t size = 0;
    char where_str[2048] = {0};

    if (brand_id_type == 2)
	size += sprintf(&where_str[size], " and t.brand_ad_id in (%s)", brand_id);

    if (find_type == 1) 
	size += sprintf(&where_str[size], " and t.title like '%%%s%%'", find_name);
    else if (find_type == 2)
	size += sprintf(&where_str[size], " and t.description like '%%%s%%'", find_name);
    else if (find_type == 3)
	size += sprintf(&where_str[size], " and (t.title like '%%%s%%' or t.description like '%%%s%%')", find_name, find_name);
    else if (find_type == 4)
	size += sprintf(&where_str[size], " and account.login_name like '%%%s%%'", find_name);

    switch(status) {

	case 1:
	    size += sprintf(&where_str[size], " and t.creative_state=0");
	    break;

	case 2:
	    size += sprintf(&where_str[size], " and t.creative_state=1");
	    break;

	case 3:
	    size += sprintf(&where_str[size], " and t.creative_state=2");
	    break;
    }

    char sql_str[40960] = {0};
    sprintf(sql_str, "select count(*) from t_brand_creative_temp t left join t_account account on t.account_id = account.account_id where 1=1 %s", where_str);

    fprintf(stderr, "-->%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_account_name(ad_system_sql_t *p, int find_type, const char *find_name)
{

    char where_str[1024] = {0};
    if (find_type == 1)
	sprintf(where_str, " and t.login_name like '%%%s%%'", find_name);
    else if (find_type == 2)
	sprintf(where_str, " and t.company_name like '%%%s%%'", find_name);
    else if (find_type == 3)
	sprintf(where_str, " and t.company_site like '%%%s%%'", find_name);

    char sql_str[40960] = {0};
    sprintf(sql_str, "select t.account_id, t.login_name, t.company_name from t_account t where row_state = 1 and rownum between 1 and 5 %s", where_str);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}



int ad_system_sql_get_brand_creative(ad_system_sql_t *p,
	int find_type, 
	const char *find_name,
	int brand_id_type,
	const char *brand_id,
	int status,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t size = 0;
    char where_str[2048] = {0};

    if (brand_id_type == 2)
	size += sprintf(&where_str[size], " and t.brand_ad_id in (%s)", brand_id);

    if (find_type == 1) 
	size += sprintf(&where_str[size], " and t.title like '%%%s%%'", find_name);
    else if (find_type == 2)
	size += sprintf(&where_str[size], " and t.description like '%%%s%%'", find_name);
    else if (find_type == 3)
	size += sprintf(&where_str[size], " and (t.title like '%%%s%%' or t.description like '%%%s%%')", find_name, find_name);
    else if (find_type == 4)
	size += sprintf(&where_str[size], " and account.login_name like '%%%s%%'", find_name);

    switch(status) {

	case 1:
	    size += sprintf(&where_str[size], " and t.creative_state=0");
	    break;

	case 2:
	    size += sprintf(&where_str[size], " and t.creative_state=1");
	    break;

	case 3:
	    size += sprintf(&where_str[size], " and t.creative_state=2");
	    break;
    }

    char sort[128] = "account.login_name";
    if (sort_type == 1) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "t.title");
		break;

	    case 2:
		sprintf(sort, "account.login_name");
		break;

	    case 3:
		sprintf(sort, "t.last_update_time");
		break;

	    case 4:
		sprintf(sort, "t.creative_state");
		break;
	}

    } else if (sort_type == 2) {

	switch(sort_column) {

	    case 1:
		sprintf(sort, "t.title desc");
		break;

	    case 2:
		sprintf(sort, "account.login_name desc");
		break;

	    case 3:
		sprintf(sort, "last_update_time desc");
		break;

	    case 4:
		sprintf(sort, "t.creative_state desc");
		break;
	}
    }

    char sql_str[40960] = {0};
    sprintf(sql_str, "select * from (select *"
	    "from (select brand_ad_id,"
	    "    brand_creative_id,"
	    "    title,"
	    "    description,"
	    "    destination_url,"
	    "    display_url,"
	    "    telephone,"
	    "    web_site,"
	    "    web_im,"
	    "    guahao_url,"
	    "    creative_state,"
	    "    refuse_reason,"
	    "    create_time,"
	    "	 last_update_time,"
	    "    account_id,"
	    "    account_name,"
	    "    auditor_name,"
	    "    rownum as page_index"
	    "    from (select t.brand_ad_id,"
	    "        t.brand_creative_id,"
	    "        t.title,"
	    "        t.description,"
	    "        t.destination_url,"
	    "        t.display_url,"
	    "        t.telephone,"
	    "        t.web_site,"
	    "t.web_im,"//"decode(t.web_im, null, account.webim, t.web_im) as web_im,"
	    "t.guahao_url,"//"decode(t.guahao_url,null, account.guahao_token, t.guahao_url) as guahao_url,"
	    "        t.creative_state,"
	    "        t.refuse_reason,"
	    "        t.create_time,"
	    "	     t.last_update_time,"
	    "	     t.account_id,"
	    "	     account.login_name as account_name,"
	    "	     manager.login_name as auditor_name"
	    "        from t_brand_creative_temp t"
	    "        left join t_account account on t.account_id=account.account_id"
	    "        left join t_manager manager on t.auditor_id = manager.manager_id"
	    "        where 1 = 1 %s"
	    "        order by %s))"
	    "        where page_index between %u and %u) TT"
	    " LEFT JOIN T_BRAND_CREATIVE_APPEND_TEMP APPEND ON TT.brand_creative_id=APPEND.BRAND_CREATIVE_ID",
	where_str,
	sort,
	page_index_s,
	page_index_e);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_brand_status(ad_system_sql_t *p, char *brand_id)
{

    char sql_str[1024];
    sprintf(sql_str, "select ad_state from T_BRAND_AD where brand_ad_id=%s", brand_id);
    if (_ad_system_sql_select(p, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(p))
	return -1;

    return ad_system_sql_get_number(p, NULL, 1);
}

int ad_system_sql_get_brand_id(ad_system_sql_t *p, char *brand_id)
{

    if (_ad_system_sql_select(p, "select SEQ_T_BRAND_AD.Nextval from dual"))
	return -1;

    if (!ad_system_sql_fetch_next(p))
	return -1;

    strcpy(brand_id, ad_system_sql_get_string(p, NULL, 1));
    return 0;
}

int ad_system_sql_add_brand_dept(ad_system_sql_t *p,
	const char *brand_id,
	const char *DEPT_CODE)
{

    char buffer[128];
    char sql_str[1024];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(DEPT_CODE, buffer, &save_ptr, ',');
    if (!str) return -1;

    while(str) {

	sprintf(sql_str, 
		"insert into t_brand_ad_dept t (BRAND_AD_DEPT_ID, BRAND_AD_ID, DEPT_ID, DEPT_CODE) values (seq_t_brand_ad_dept.nextval, %s, %s, '%s')",
		brand_id, str, str);

	if (_ad_system_sql_execute(p, sql_str))
	    return -1;

	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return 0;
}

int ad_system_sql_add_brand_region(ad_system_sql_t *p,
	const char *brand_id,
	int REGION_TYPE,
	const char *REGION_SERIALIZE)
{

    if (REGION_TYPE == 1)
	return 0;

    char buffer[128];
    char sql_str[409600];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(REGION_SERIALIZE, buffer, &save_ptr, ',');
    if (!str) return -1;

    while(str) {

	sprintf(sql_str, 
		"insert into T_BRAND_AD_REGION (BRAND_AD_REGION_ID, BRAND_AD_ID, REGION_ID, REGION_CODE) values (seq_t_brand_ad_region.nextval, %s, %s, '%s')",
		brand_id, str, str);

	fprintf(stderr, "############>%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str))
	    return -1;

	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    return 0;
}

int ad_system_sql_add_brand_r(ad_system_sql_t *p,
	const char *brand_id,
	const char *ACCOUNT_ID,
	int REGION_TYPE,
	const char *REGION_SERIALIZE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	const char *DEPT_CODE,
	const char *PRICE,
	const char *PROPOSER_ID)
{

    char REGION_SERIALIZE_STR[409600] = "null";
    if (REGION_TYPE == 2)
	sprintf(REGION_SERIALIZE_STR, REGION_SERIALIZE);

    char sql_str[409600];
    size_t size = sprintf(sql_str, "insert into T_BRAND_AD"
	    "(BRAND_AD_ID,"
	    "ACCOUNT_ID,"
	    "AD_ORDER_NO,"
	    "REGION_TYPE,"
	    "REGION_SERIALIZE,"
	    "BEGIN_TIME,"
	    "END_TIME,"
	    "DEPT_CODE,"
	    "PRICE,"
	    "AD_STATE,"
	    "PROPOSER_ID,"
	    "PROPOSER_IS_MANAGER,"
	    "CREATE_TIME,"
	    "SYNCH_STATE,"
	    "ROW_STATE)"
	    "values"
	    "(%s,"
	    "%s,"//ACCOUNT_ID
	    "sys_guid(),"
	    "%u,"//REGION_TYPE
	    "'%s',"//REGION_SERIALIZE
	    "to_date('%s', 'yyyy-mm-dd'),"//SHOW_TIME_S
	    "to_date('%s', 'yyyy-mm-dd'),"//SHOW_TIME_E
	    "'%s',"//DEPT_CODE
	    "%s,"//PRICE
	    "0,"
	    "%s,"//PROPOSER_ID
	    "1,"
	    "TRUNC(sysdate),"
	    "0,"
	    "1)",

	brand_id,
	ACCOUNT_ID,
	REGION_TYPE == 1 ? 0 : 1,
	REGION_SERIALIZE_STR,
	SHOW_TIME_S,
	SHOW_TIME_E,
	DEPT_CODE,
	PRICE,
	PROPOSER_ID);

    fprintf(stderr, "[%zd]%s\n", size, sql_str);
    fprintf(stderr, "[%zd]\n", strlen(REGION_SERIALIZE_STR));
    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    return 0;
}

int ad_system_sql_brand_pric_check(ad_system_sql_t *p, const char *account_id, const char *PRICE)
{

    char sql_str[1024];
    sprintf(sql_str, "select brand_balance from t_account t where t.account_id = %s", account_id);
    if (_ad_system_sql_select(p, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(p))
	return -1;

    double PRICE_NUM = atof(PRICE);
    fprintf(stderr, "-------------------<%f><%f>\n", ad_system_sql_get_double(p, NULL, 1), PRICE_NUM);
    if (ad_system_sql_get_double(p, NULL, 1) < PRICE_NUM)
	return -1;

    return 0;
}

int ad_system_sql_brand_inspection(ad_system_sql_t *p,
	const char *manager_id,
	const char *brand_id_str, 
	int status,
	const char *REFUSE_REASON,
	const char *ACCOUNT_ID,
	int REGION_TYPE,
	const char *REGION_SERIALIZE,
	const char *DEPT_SERIALIZE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	const char *PRICE,
	const char *PROPOSER_ID)
{

    char sql_str[4096000];

    if (status == 2) {

	//更新订单状态(审核拒绝)
	sprintf(sql_str, "update t_brand_ad t  set t.ad_state = 2, t.AUDITOR_ID='%s',t.AUDITOR_UPDATE_TIME=sysdate,t.REFUSE_REASON='%s' where brand_ad_id in (%s)", 
		manager_id, REFUSE_REASON, brand_id_str);
	fprintf(stderr, "----->%s\n", sql_str);
	if (_ad_system_sql_execute(p, sql_str))
	    return -1; 

	ad_system_sql_commit(p);
	return 0;
    }

    char brand_id[128] = {0};
    if (!brand_id_str) {

	fprintf(stderr, "brand_id=%s\n", brand_id);
	if (ad_system_sql_get_brand_id(p, brand_id))
	    return -1;

	fprintf(stderr, "brand_id=%s\n", brand_id);
	if (ad_system_sql_add_brand_r(p,
		    brand_id,
		    ACCOUNT_ID,
		    REGION_TYPE,
		    REGION_SERIALIZE,
		    SHOW_TIME_S,
		    SHOW_TIME_E,
		    DEPT_SERIALIZE,
		    PRICE,
		    PROPOSER_ID))
	    return -1;

    } else {

	strcpy(brand_id, brand_id_str);

	sprintf(sql_str, "delete from t_brand_ad_dept where BRAND_AD_ID=%s", brand_id);
	if (_ad_system_sql_execute(p, sql_str))
	    return -1;

	sprintf(sql_str, "delete from T_BRAND_AD_REGION where BRAND_AD_ID=%s", brand_id);
	if (_ad_system_sql_execute(p, sql_str)) {

	    ad_system_sql_rollback(p);
	    return -1; 
	}
    }

    if (ad_system_sql_add_brand_dept(p, brand_id, DEPT_SERIALIZE)) {

	ad_system_sql_rollback(p);
	return -1;
    }

    if (ad_system_sql_add_brand_region(p, brand_id, REGION_TYPE, REGION_SERIALIZE)) {

	ad_system_sql_rollback(p);
	return -1;
    }

    size_t size = 0;
    char where_str[409600] = {0};
    if (REGION_TYPE == 2)
	size = sprintf(where_str, "and region.region_code in (%s)", REGION_SERIALIZE);
    sprintf(&where_str[size], " and dept.dept_code in (%s)", DEPT_SERIALIZE);

    //更新订单状态(审核通过)
    sprintf(sql_str,
	    "update t_brand_ad t "
	    "set t.ad_state = 1, "
	    "t.auditor_id = %s, "
	    "t.AUDITOR_UPDATE_TIME = sysdate "
	    "where t.brand_ad_id = %s"
	    "and not exists (select * "
	    "from t_brand_ad brand, "
	    "t_brand_ad_dept dept, "
	    "t_brand_ad_region region "
	    "where brand.ad_state = 1 "
	    "and brand.row_state = 1 "
	    "and brand.begin_time <= to_date('%s', 'YYYY-MM-DD') "
	    "and brand.end_time >= to_date('%s', 'YYYY-MM-DD') "
	    "and brand.brand_ad_id = dept.brand_ad_id "
	    "and brand.brand_ad_id = region.brand_ad_id %s)",
	    manager_id,
	    brand_id,
	    SHOW_TIME_E,
	    SHOW_TIME_S,
	    where_str);

    fprintf(stderr, "%s\n", sql_str);
    if (_ad_system_sql_execute(p, sql_str)) {

	ad_system_sql_rollback(p);
	return -1; 
    }

    // 如果审核未通过(删除原有地域科室)
    if (ad_system_sql_get_brand_status(p, brand_id) != 1) {

	if (!brand_id_str) {

	    sprintf(sql_str, "delete from t_brand_ad_dept where BRAND_AD_ID=%s", brand_id);
	    if (_ad_system_sql_execute(p, sql_str)) {

		ad_system_sql_rollback(p);
		return -1;
	    }

	    sprintf(sql_str, "delete from T_BRAND_AD_REGION where BRAND_AD_ID=%s", brand_id);
	    if (_ad_system_sql_execute(p, sql_str)) {

		ad_system_sql_rollback(p);
		return -1;
	    }

	    sprintf(sql_str, "delete from t_brand_ad where BRAND_AD_ID=%s", brand_id);
	    if (_ad_system_sql_execute(p, sql_str)) {

		ad_system_sql_rollback(p);
		return -1;
	    }
	}

	return -1;
    }

    //扣费
    sprintf(sql_str,
	    "update t_account t set t.brand_balance = t.brand_balance - %s where t.account_id = %s",
	    PRICE,
	    ACCOUNT_ID);
    if (_ad_system_sql_execute(p, sql_str)) {

	ad_system_sql_rollback(p);
	return -1;
    }

    //记录扣款记录(以后报告使用)
    sprintf(sql_str,
	    "insert into T_BRAND_CONSUME "
	    "(BRAND_CONSUME_ID, BRAND_AD_ID, ACCOUNT_ID, MONEY) "
	    "select seq_t_brand_consume.nextval, t.brand_ad_id, t.account_id, t.price "
	    "from t_brand_ad t "
	    "where t.brand_ad_id=%s",
	    brand_id);
    if (_ad_system_sql_execute(p, sql_str)) {

	ad_system_sql_rollback(p);
	return -1;
    }

    ad_system_sql_commit(p);
    return 0;
}

int ad_system_sql_get_brand_unique(ad_system_sql_t *p, const char *brand_id)
{

    char sql_str[40960];
    sprintf(sql_str, "select brand.brand_ad_id,"
	    " brand.ad_order_no,"
	    " brand.account_id,"
	    " account.login_name as account_name,"
	    " brand.dept_code,"
	    " brand.region_type,"
	    " brand.region_serialize,"
	    " brand.end_time - brand.begin_time + 1 as days,"
	    " brand.begin_time,"
	    " brand.end_time,"
	    " brand.price as money,"
	    " to_char(brand.create_time, 'YYYY-HH-DD') as b_create_time,"
	    " decode(brand.ad_state,0,decode(sign(TRUNC(sysdate) - brand.begin_time),-1,4,5),1,decode(sign(sysdate - brand.begin_time),-1,3,"
	    " decode(sign(sysdate - brand.end_time),1,2,1)),6) as status,"
	    " brand.refuse_reason as brand_refuse_reason"
	    " from t_brand_ad brand"
	    " left join t_account account on brand.account_id = account.account_id"
	    " where brand.row_state = 1"
	    " and brand.brand_ad_id = %s", brand_id);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_add_money_operation(
	char *ACCOUNT_ID,
	char *MONEY,
	char *PAYMENT_TYPE,
	char *ACCOUNTING_DOCUMENT_NUMBER,
	char *ACCOUNTING_DOCUMENT_FILE,
	char *ACCOUNTING_DOCUMENT_REMARK,
	char *CREATE_BY,
	char *LAST_UPDATE_BY,
	char *BRAND_TYPE,
	char *DISCOUNT)
{

    char sql_str[4096] = {0};
    sprintf(sql_str,
	    "insert into T_RECHARGE(RECHARGE_ID,"
	    "ACCOUNT_ID,"
	    "SWIFT_NUMBER,"
	    "MONEY,"
	    "CURRENT_MONEY_BALANCE,"
	    "INVEST,"
	    "PAYMENT_TYPE,"
	    "ACCOUNTING_DOCUMENT_NUMBER,"
	    "ACCOUNTING_DOCUMENT_FILE,"
	    "FILE_STORE_ID,"
	    "ACCOUNTING_DOCUMENT_REMARK,"
	    "OPERATE_TIME,"
	    "COLLECTED_TIME,"
	    "ROW_STATE,"
	    "CREATE_TIME,"
	    "CREATE_BY,"
	    "LAST_UPDATE_TIME,"
	    "LAST_UPDATE_BY,"
	    "CREATE_BY_UNAME,"
	    "LAST_UPDATE_BY_UNAME,"
	    "SYNCH_STATE,"
	    "RECHARGE_DOCUMENT_NAME,"
	    "BRAND_TYPE,"
	    "DISCOUNT,"
	    "MULTIPLE"
	    ") values (sys_guid(),"
	    "%s,"	//ACCOUNT_ID
	    "to_char('NO.' || to_char(SYSDATE, 'YYYYMMDD') || LPAD(SEQ_T_RECHARGE_SWIFT.Nextval, 5, 0))," //SWIFT_NUMBER
	    "%s,"//MONEY
	    "(select balance + BRAND_BALANCE + %s from t_account where account_id = %s),"  //CURRENT_MONEY_BALANCE
	    "(select invest + %s from t_account where account_id = %s),"   //INVEST
	    "%s,"	//PAYMENT_TYPE
	    "'%s',"	//ACCOUNTING_DOCUMENT_NUMBER
	    "'%s',"	//ACCOUNTING_DOCUMENT_FILE
	    "null,"	//FILE_STORE_ID
	    "'%s',"	//ACCOUNTING_DOCUMENT_REMARK
	    "sysdate,"	//OPERATE_TIME
	    "sysdate,"	//COLLECTED_TIME
	    "1,"	//ROW_STATE
	    "sysdate,"	//CREATE_TIME
	    "%s,"	//CREATE_BY
	    "sysdate,"	//LAST_UPDATE_TIME
	    "%s,"	//LAST_UPDATE_BY
	    "'null',"	//CREATE_BY_UNAME
	    "'null',"	//LAST_UPDATE_BY_UNAME
	    "0,"	//SYNCH_STATE
	    "null,"	//RECHARGE_DOCUMENT_NAME
	    "%s,"	//BRAND_TYPE
	    "%s,"	//DISCOUNT
	    "%s / 100 + 1)",//MULTIPLE

	ACCOUNT_ID,
	MONEY,
	MONEY,
	ACCOUNT_ID,
	MONEY,
	ACCOUNT_ID,
	PAYMENT_TYPE,
	ACCOUNTING_DOCUMENT_NUMBER,
	ACCOUNTING_DOCUMENT_FILE,
	ACCOUNTING_DOCUMENT_REMARK,
	CREATE_BY,
	LAST_UPDATE_BY,
	BRAND_TYPE,
	DISCOUNT,
	DISCOUNT);
    fprintf(stderr, "%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    if (strcmp(BRAND_TYPE, "0") == 0) {

	if (ad_system_sql_add_money_operation_update_account(&sql_conn, ACCOUNT_ID, MONEY)) {

	    ad_system_sql_rollback(&sql_conn);
	    ad_system_sql_free(&sql_conn);
	    return -1;
	}

    } else if (strcmp(BRAND_TYPE, "1") == 0) {

	if (ad_system_sql_add_brand_money_operation_update_account(&sql_conn, ACCOUNT_ID, MONEY)) {

	    ad_system_sql_rollback(&sql_conn);
	    ad_system_sql_free(&sql_conn);
	    return -1;
	}

    } else {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_manager_dsp_list_region(const char *region_src, char *region_dest)
{

    char buffer[1024];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(region_src, buffer, &save_ptr, ',');

    size_t size = sprintf(region_dest, " and (CAMPAIGN.Region_Type = 0 ");

    while(str) {

	size += sprintf(&region_dest[size], "or CAMPAIGN.Region_Serialize like '%%%s%%' ", str);
	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
    }

    size += sprintf(&region_dest[size], ")");
    return size;
}

int ad_system_sql_get_manager_dsp_list_dept(const char *dept_src, char *dept_dest)
{

    char buffer[1024];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(dept_src, buffer, &save_ptr, ',');
    char or_str[4] = "";

    size_t size = sprintf(dept_dest, " and (");

    while(str) {

	size += sprintf(&dept_dest[size], "%s adgroup_targeting.code like '%%%s%%' ", or_str, str);
	str = lg_string_strtok(NULL, buffer, &save_ptr, ',');
	strcpy(or_str, "or");
    }

    size += sprintf(&dept_dest[size], ")");
    return size;
}


int ad_system_sql_get_manager_dsp_list(ad_system_sql_t *p,
	const char *CPC_PRICE,
	const char *BUDGET_MORE,
	const char *BUDGET_LESS,
	const char *ACCOUNT_NAME,
	const char *REGION_SERIALIZE,
	const char *DEPT_SERIALIZE,
	int STATUS,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t where_size = 0;
    char where_str[4096] = {0};

    if (ACCOUNT_NAME && *ACCOUNT_NAME)
	where_size += sprintf(&where_str[where_size], " and ACCOUNT.Login_Name like '%%%s%%' ", ACCOUNT_NAME);

    if (CPC_PRICE && *CPC_PRICE)
	where_size += sprintf(&where_str[where_size], " and ADGROUP.CPC_PRICE >= %s ", CPC_PRICE);

    if (BUDGET_MORE && *BUDGET_MORE)
	where_size += sprintf(&where_str[where_size], " and CAMPAIGN.Campaign_Budget <= %s ", BUDGET_MORE);

    if (BUDGET_LESS && *BUDGET_LESS)
	where_size += sprintf(&where_str[where_size], " and CAMPAIGN.Campaign_Budget >= %s ", BUDGET_LESS);

    if (REGION_SERIALIZE && *REGION_SERIALIZE)
	where_size += ad_system_sql_get_manager_dsp_list_region(REGION_SERIALIZE, &where_str[where_size]);

    if (DEPT_SERIALIZE && *DEPT_SERIALIZE)
	where_size += ad_system_sql_get_manager_dsp_list_dept(DEPT_SERIALIZE, &where_str[where_size]);

    char dsp_status[128] = {0};
    if (STATUS != 100) {
	if (STATUS == 3) {
	    sprintf(dsp_status, " AND STATUS!=2");
	} else {
	    sprintf(dsp_status, " AND STATUS=%d", STATUS);
	}
    }

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "LOGIN_NAME");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "CAMPAIGN_BUDGET");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "CPC_PRICE");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "TITLE");
	    break;

	case 5:
	    sort_size = sprintf(sort_str, "STATUS");
	    break;
    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");


    char sql_str[8192] = {0};
    sprintf(sql_str, 
	    "SELECT * FROM ( "
	    "SELECT CREATIVE_ID, "
	    "LOGIN_NAME, "
	    "CAMPAIGN_BUDGET, "
	    "CPC_PRICE, "
	    "TITLE, "
	    "STATUS, "
	    "ROWNUM AS RN "
	    " FROM (SELECT * FROM ("

	    "SELECT CREATIVE_ID, "
	    "LOGIN_NAME, "
	    "CAMPAIGN_BUDGET, "
	    "CPC_PRICE, "
	    "TITLE, "
	    "DECODE(TT.PAUSE, null, 2, TT.PAUSE) AS STATUS "
	    "FROM (SELECT CREATIVE.CREATIVE_ID, "
	    "ACCOUNT.LOGIN_NAME, "
	    "CAMPAIGN.CAMPAIGN_BUDGET, "
	    "ADGROUP.CPC_PRICE, "
	    "CREATIVE.TITLE "
	    "FROM T_TEXT_CREATIVE CREATIVE "
	    "LEFT JOIN T_ACCOUNT ACCOUNT ON CREATIVE.ACCOUNT_ID = ACCOUNT.ACCOUNT_ID "
	    "LEFT JOIN T_CAMPAIGN CAMPAIGN ON CREATIVE.CAMPAIGN_ID = CAMPAIGN.CAMPAIGN_ID "
	    "LEFT JOIN T_ADGROUP ADGROUP ON CREATIVE.ADGROUP_ID = ADGROUP.ADGROUP_ID "
	    "LEFT JOIN T_ADGROUP_TARGETING ADGROUP_TARGETING ON CREATIVE.ADGROUP_ID = ADGROUP_TARGETING.ADGROUP_ID "
	    "WHERE ACCOUNT.ROW_STATE = 1 "
	    "AND ACCOUNT.BALANCE > 0 "
	    "AND ACCOUNT.JOIN_DSP = 1 "
	    "AND CAMPAIGN.PAUSE = 0 "
	    "AND ADGROUP.ADGROUP_STATE = 21 "
	    "AND CREATIVE.Creative_State = 31 "
	    "AND CREATIVE.PAUSE = 0 %s) T "
	    "LEFT JOIN (SELECT FAKE_CREATIVE.TEXT_CREATIVE_ID, FAKE_CREATIVE.PAUSE "
	    "FROM T_FAKE_CREATIVE FAKE_CREATIVE "
	    "WHERE FAKE_CREATIVE.ROW_STATE = 1 "
	    "GROUP BY FAKE_CREATIVE.TEXT_CREATIVE_ID, FAKE_CREATIVE.PAUSE) TT ON T.CREATIVE_ID = TT.TEXT_CREATIVE_ID) "
	    "WHERE 1=1 %s ORDER BY %s)) "
	    "WHERE RN BETWEEN %u AND %u",
	where_str,
	dsp_status,
	sort_str,
	page_index_s,
	page_index_e);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_manager_dsp_list_count(char *count,
	const char *CPC_PRICE,
	const char *BUDGET_MORE,
	const char *BUDGET_LESS,
	const char *ACCOUNT_NAME,
	const char *REGION_SERIALIZE,
	const char *DEPT_SERIALIZE,
	int STATUS,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t where_size = 0;
    char where_str[40960] = {0};

    if (ACCOUNT_NAME && *ACCOUNT_NAME)
	where_size += sprintf(&where_str[where_size], " and ACCOUNT.Login_Name like '%%%s%%' ", ACCOUNT_NAME);

    if (CPC_PRICE && *CPC_PRICE)
	where_size += sprintf(&where_str[where_size], " and ADGROUP.CPC_PRICE >= %s ", CPC_PRICE);

    if (BUDGET_MORE && *BUDGET_MORE)
	where_size += sprintf(&where_str[where_size], " and CAMPAIGN.Campaign_Budget <= %s ", BUDGET_MORE);

    if (BUDGET_LESS && *BUDGET_LESS)
	where_size += sprintf(&where_str[where_size], " and CAMPAIGN.Campaign_Budget >= %s ", BUDGET_LESS);

    if (REGION_SERIALIZE && *REGION_SERIALIZE)
	where_size += ad_system_sql_get_manager_dsp_list_region(REGION_SERIALIZE, &where_str[where_size]);

    if (DEPT_SERIALIZE && *DEPT_SERIALIZE)
	where_size += ad_system_sql_get_manager_dsp_list_dept(DEPT_SERIALIZE, &where_str[where_size]);

    char dsp_status[128] = {0};
    if (STATUS != 100) {
	if (STATUS == 3) {
	    sprintf(dsp_status, " AND STATUS!=2");
	} else {
	    sprintf(dsp_status, " AND STATUS=%d", STATUS);
	}
    }

    char sql_str[40960] = {0};
    sprintf(sql_str, 
	    "SELECT count(*) FROM ( "
	    "SELECT CREATIVE_ID, "
	    "LOGIN_NAME, "
	    "CAMPAIGN_BUDGET, "
	    "CPC_PRICE, "
	    "TITLE, "
	    "STATUS, "
	    "ROWNUM AS RN "
	    " FROM ("
	    "SELECT CREATIVE_ID, "
	    "LOGIN_NAME, "
	    "CAMPAIGN_BUDGET, "
	    "CPC_PRICE, "
	    "TITLE, "
	    "DECODE(TT.PAUSE, null, 2, TT.PAUSE) AS STATUS "
	    "FROM (SELECT CREATIVE.CREATIVE_ID, "
	    "ACCOUNT.LOGIN_NAME, "
	    "CAMPAIGN.CAMPAIGN_BUDGET, "
	    "ADGROUP.CPC_PRICE, "
	    "CREATIVE.TITLE "
	    "FROM T_TEXT_CREATIVE CREATIVE "
	    "LEFT JOIN T_ACCOUNT ACCOUNT ON CREATIVE.ACCOUNT_ID = ACCOUNT.ACCOUNT_ID "
	    "LEFT JOIN T_CAMPAIGN CAMPAIGN ON CREATIVE.CAMPAIGN_ID = CAMPAIGN.CAMPAIGN_ID "
	    "LEFT JOIN T_ADGROUP ADGROUP ON CREATIVE.ADGROUP_ID = ADGROUP.ADGROUP_ID "
	    "LEFT JOIN T_ADGROUP_TARGETING ADGROUP_TARGETING ON CREATIVE.ADGROUP_ID = ADGROUP_TARGETING.ADGROUP_ID "
	    "WHERE ACCOUNT.ROW_STATE = 1 "
	    "AND ACCOUNT.BALANCE > 0 "
	    "AND ACCOUNT.JOIN_DSP = 1 "
	    "AND CAMPAIGN.PAUSE = 0 "
	    "AND ADGROUP.ADGROUP_STATE = 21 "
	    "AND CREATIVE.Creative_State = 31 "
	    "AND CREATIVE.PAUSE = 0 %s) T "
	    "LEFT JOIN (SELECT FAKE_CREATIVE.TEXT_CREATIVE_ID, FAKE_CREATIVE.PAUSE "
	    "FROM T_FAKE_CREATIVE FAKE_CREATIVE "
	    "WHERE FAKE_CREATIVE.ROW_STATE = 1 "
	    "GROUP BY FAKE_CREATIVE.TEXT_CREATIVE_ID, FAKE_CREATIVE.PAUSE) TT ON T.CREATIVE_ID = TT.TEXT_CREATIVE_ID)"
	    "WHERE 1=1 %s) ",
	where_str,
	dsp_status);

    fprintf(stderr, "-->%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(count, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_manager_dsp(ad_system_sql_t *p,
	const char *TEXT_CREATIVE_ID,
	const char *CREATIVE_NAME,
	const char *IMG_SIZE,
	const char *FILE_SIZE,
	const char *FILE_PATH
	)
{

    char sql_str[1024] = {0};
    sprintf(sql_str,
	    "insert into T_FAKE_CREATIVE "
	    "select seq_t_creative.nextval,"
	    "t.account_id,"	//t.account_id
	    "t.campaign_id,"	//t.campaign_id
	    "t.adgroup_id,"	//t.adgroup_id
	    "t.creative_id,"	//t.creative_id
	    "'%s',"		//CREATIVE_NAME
	    "'%s',"		//IMG_SIZE
	    "%s,"		//IMG_LENGTH
	    "'%s',"		//FILE_PATH
	    "0," 		//AUDITING_STATE   
	    "null," 		//REFUSE_REASON    
	    "0," 		//PAUSE            
	    "1," 		//ROW_STATE        
	    "0," 		//SYNCH_STATE     
	    "sysdate," 		//CREATE_TIME      
	    "sysdate" 		//LAST_UPDATE_TIME 
	    " from t_text_creative t "
	    " where t.creative_id = %s",
	    CREATIVE_NAME,
	    IMG_SIZE,
	    FILE_SIZE,
	    FILE_PATH,
	    TEXT_CREATIVE_ID);

    fprintf(stderr, "%s\n", sql_str);
    return _ad_system_sql_execute(p, sql_str);
}

int ad_system_sql_get_manager_dsp(ad_system_sql_t *p, const char *TEXT_CREATIVE_ID)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select t.creative_id, t.CREATIVE_NAME, t.img_size, t.img_length, t.file_path from t_fake_creative t where t.row_state = 1 and t.text_creative_id = %s", TEXT_CREATIVE_ID);
    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_del_manager_dsp(const char *IMG_ID)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "update t_fake_creative t set t.row_state = 0 where t.creative_id in (%s)", IMG_ID);
    fprintf(stderr, "-->%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_manager_dsp(const char *TEXT_CREATIVE_ID, int status)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "update t_fake_creative t set t.pause = %d, t.synch_state = 0 where t.text_creative_id in (%s)", status, TEXT_CREATIVE_ID);
    fprintf(stderr, "-->%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}


