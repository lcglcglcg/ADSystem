
#include <ocilib.h>
#include <ocilib_internal.h>
#include <arpa/inet.h>
#include "ad_system_sql.h"
#include "lg_string.h"

/****************/
/*oracle oci API*/
/****************/

void ad_system_sql_err_handler(OCI_Error *err)
{
    fprintf(stderr, OCI_ErrorGetString(err));
}

static OCI_ConnPool *ad_system_sql_pool = NULL;
int ad_system_sql_init(lg_list_t *config_file_head)
{

    char *oracle_name = NULL;
    if (!(oracle_name = lg_config_file_find(config_file_head, "oracle_name")))
	return -1;;

    char *oracle_pass = NULL;
    if (!(oracle_pass = lg_config_file_find(config_file_head, "oracle_pass")))
	return -1;;

    char *oracle_host = NULL;
    if (!(oracle_host = lg_config_file_find(config_file_head, "oracle_host")))
	return -1;;

    char *oracle_pool_str = NULL;
    if (!(oracle_pool_str = lg_config_file_find(config_file_head, "oracle_pool")))
	return -1;;

    int oracle_pool = 0;
    if (!(oracle_pool = atoi(oracle_pool_str)))
	return -1;;

    fprintf(stdout, "ORACLE_NAME=%s\n", oracle_name);
    fprintf(stdout, "ORACLE_PASS=%s\n", oracle_pass);
    fprintf(stdout, "ORACLE_HOST=%s\n", oracle_host);
    fprintf(stdout, "ORACLE_POOL=%d\n", oracle_pool);

    setenv("NLS_LANG","AMERICAN_AMERICA.UTF8",1);
    if (!OCI_Initialize(ad_system_sql_err_handler, NULL, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
	return -1;; 

    if (!(ad_system_sql_pool = OCI_PoolCreate(
		    oracle_host, 
		    oracle_name, 
		    oracle_pass, 
		    OCI_POOL_CONNECTION, 
		    OCI_SESSION_DEFAULT, 
		    0,   
		    oracle_pool, 
		    1))) 
	return -1;

    return 0;
}

int ad_system_sql_connect(ad_system_sql_t *p)
{

    OCI_Connection *cn = NULL;
    if (!(cn = OCI_PoolGetConnection(ad_system_sql_pool, NULL)))
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

int ad_system_sql_select(ad_system_sql_t *p, char *sql_str)
{

    memset(p, 0, sizeof(ad_system_sql_t));

    OCI_Connection *cn = NULL;
    if (!(cn = OCI_PoolGetConnection(ad_system_sql_pool, NULL)))
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
    const char *str = OCI_GetString(p->rs, index);
    if (str) return str;
    return "";
}

int ad_system_sql_execute(ad_system_sql_t *p, char *sql_str)
{

    memset(p, 0, sizeof(ad_system_sql_t));

    OCI_Connection *cn = NULL;
    if (!(cn = OCI_PoolGetConnection(ad_system_sql_pool, NULL)))
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
    OCI_ConnectionFree(p->cn);
}

/****************/
/*   业务逻辑   */
/****************/

//登录验证
int ad_system_sql_login(ad_system_sql_t *p, const char *account, const char *password)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select "
	    "to_char(ACCOUNT_ID),"
	    "to_char(ROW_STATE) "
	    "from T_ACCOUNT "
	    "where LOGIN_NAME='%s' and LOGIN_PASSWORD_MD5='%s' and ROW_STATE != 0",
	    account, password);
    return ad_system_sql_select(p, sql_str);
}

//更新登录地址
int ad_system_sql_set_login(const char *account_id, char *addr)
{

    char sql_str[1024];
    sprintf(sql_str, "update T_ACCOUNT set LATE_LOGIN_IP='%s', "
	    "LATE_LOGIN_ADDRESS=(select province || city from t_sys_ip where %u >= ip_long_begin and %u <= ip_long_end),"
	    "LATE_LOGIN_TIME=sysdate where ACCOUNT_ID=%s",
	    addr,
	    inet_network(addr),
	    inet_network(addr),
	    account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//获取标题信息
int ad_system_sql_get_title_info(ad_system_sql_t *p, char *account_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select "
	    "LOGIN_NAME,"
	    "COMPANY_NAME,"
	    "BALANCE + BRAND_BALANCE,"
	    "INVEST,"
	    "INVEST - BALANCE - BRAND_BALANCE,"
	    "to_char(CREATE_TIME, 'yyyy-mm-dd'),"
	    "domain,"
	    "GUAHAO_TOKEN,"
	    "WEBIM,"
	    "PLACE_REGION "
	    "from T_ACCOUNT "
	    "where ACCOUNT_ID = %s",
	    account_id);
    return ad_system_sql_select(p, sql_str);
}

//首页->获取公告
int ad_system_sql_get_announcement(ad_system_sql_t *p, int page_index_s, int page_index_e, const char *title)
{
    char where_str[1024] = {0};
    if (title) sprintf(where_str, "and announcement_title like '%%%s%%'", title);

    char sql_str[1024] = {0};
    sprintf(sql_str, "select "
	    "to_char(send_time, 'yyyy-mm-dd'),"
	    "announcement_title,"
	    "content "
	    "from (select rownum as page_index, send_time, announcement_title, content from t_announcement "
	    "where trunc(sysdate) >= trunc(SEND_TIME) %s "
	    "order by send_time desc) "
	    "where page_index between %u and %u",

	    where_str, 
	    page_index_s, 
	    page_index_e);
    return ad_system_sql_select(p, sql_str);
}

//首页->获取公告->数量
int ad_system_sql_get_announcement_count(ad_system_sql_t *p, const char *title)
{
    char where_str[1024] = {0};
    if (title) sprintf(where_str, "and announcement_title like '%%%s%%'", title);

    char sql_str[1024] = {0};
    sprintf(sql_str, "select count(*) from t_announcement where 1=1 %s", where_str);
    return ad_system_sql_select(p, sql_str);
}

//首页->曲线图->出价
int ad_system_sql_get_budget(ad_system_sql_t *p, char *account_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, 
	    "select "
	    "to_char(t.display_style),"
	    "to_char(sum(t.campaign_budget)) "
	    "from t_campaign t "
	    "where t.account_id = %s "
	    "group by t.display_style",
	    account_id);
    return ad_system_sql_select(p, sql_str);
}

//账户信息->基本信息
int ad_system_sql_get_user_info(ad_system_sql_t *p, char *account_id)
{
    if (!p || !account_id)
	return -1;

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "LOGIN_NAME,"
	    "COMPANY_NAME,"
	    "COMPANY_SITE,"
	    "INDUSTRY,"
	    "API_TOKEN,"
	    "PROVINCE_ID,"
	    "CITY_ID,"
	    "ADDRESS,"
	    "to_char(POSTCODE),"
	    "CONTACT,"
	    "TELEPHONE,"
	    "FAX,"
	    "MOBILE,"
	    "EMAIL,"
	    "WEBIM,"
	    "to_char(LATE_LOGIN_TIME, 'yyyy-mm-dd HH24:MI:SS'),"
	    "LATE_LOGIN_IP,"
	    "LATE_LOGIN_ADDRESS,"
	    "PLACE_REGION "
	    "from T_ACCOUNT "
	    "where ACCOUNT_ID = %s",
	account_id);

    fprintf(stdout, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

//账户信息->修改信息
int ad_system_sql_set_user_info(ad_system_sql_t *p, char *account_id,
	const char *company_name,
	const char *company_site,
	const char *industry,
	const char *province_id,
	const char *city_id,
	const char *address,
	const char *postcode,
	const char *contact,
	const char *telephone,
	const char *fax,
	const char *mobile,
	const char *email,
	const char *webim)
{

    char comma[4] = {0};
    char sql_str[40960] = {0};
    size_t sql_size = sprintf(sql_str, "update t_account set ");
    if (company_name) {
	sql_size += sprintf(&sql_str[sql_size], "%s company_name ='%s' ", comma,        company_name);
	comma[0] = ',';
    }
    if (company_site) {
	sql_size += sprintf(&sql_str[sql_size], "%s company_site ='%s' ", comma,        company_site);
	comma[0] = ',';
    }
    if (industry) {
	sql_size += sprintf(&sql_str[sql_size], "%s industry ='%s' ", comma,            industry);
	comma[0] = ','; 
    }    
    if (province_id) {
	sql_size += sprintf(&sql_str[sql_size], "%s province_id ='%s' ", comma,         province_id);
	comma[0] = ','; 
    }    
    if (city_id) {
	sql_size += sprintf(&sql_str[sql_size], "%s city_id ='%s' ", comma,             city_id);
	comma[0] = ','; 
    }    
    if (address) {
	sql_size += sprintf(&sql_str[sql_size], "%s address ='%s' ", comma,             address);
	comma[0] = ','; 
    }    
    if (postcode) {
	sql_size += sprintf(&sql_str[sql_size], "%s postcode ='%s' ", comma,            postcode);
	comma[0] = ',';
    }
    if (contact) {
	sql_size += sprintf(&sql_str[sql_size], "%s contact ='%s' ", comma,             contact);
	comma[0] = ',';
    }
    if (telephone) {
	sql_size += sprintf(&sql_str[sql_size], "%s telephone ='%s' ", comma,           telephone);
	comma[0] = ',';
    }
    if (fax) {
	sql_size += sprintf(&sql_str[sql_size], "%s fax ='%s' ", comma,                 fax);
	comma[0] = ',';
    }
    if (mobile) {
	sql_size += sprintf(&sql_str[sql_size], "%s mobile ='%s' ", comma,              mobile);
	comma[0] = ',';
    }
    if (email) {
	sql_size += sprintf(&sql_str[sql_size], "%s email ='%s' ", comma,               email);
	comma[0] = ',';
    }
    if (webim) {
	sql_size += sprintf(&sql_str[sql_size], "%s webim ='%s' ", comma,               webim);
	comma[0] = ',';
    }

    if (comma[0] == ',') {

	sql_size += sprintf(&sql_str[sql_size], ",SYNCH_STATE=0 ");
	sql_size += sprintf(&sql_str[sql_size], "where account_id=%s", account_id);

    } else
	return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//账户信息->获取密码
int ad_system_sql_get_user_pass(ad_system_sql_t *p, char *account_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select "
	    "LOGIN_PASSWORD_MD5 "
	    "from T_ACCOUNT "
	    "where ACCOUNT_ID = %s",
	    account_id);
    return ad_system_sql_select(p, sql_str);
}

//账户信息->修改密码
int ad_system_sql_set_user_pass(char *account_id, const char *pass_new)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "update T_ACCOUNT "
	    "set LOGIN_PASSWORD='%s',"
	    "LOGIN_PASSWORD_MD5=MD5('%s') "
	    "where ACCOUNT_ID = %s",
	    pass_new,
	    pass_new,
	    account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
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

//推广->获取计划
int ad_system_sql_get_campaign(ad_system_sql_t *p,
	char *account_id, 
	int display_status,
	int find_type,
	int find_name_type,
	const char *find_name)
{

    char where_str[1024] = {0};
    if (find_type & 2) {

	if (find_name_type == 1)
	    sprintf(where_str, "and CAMPAIGN_NAME='%s'", find_name);

	if (find_name_type == 2)
	    sprintf(where_str, "and CAMPAIGN_NAME like '%%%s%%'", find_name);
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "CAMPAIGN_ID,"
	    "CAMPAIGN_NAME,"
	    "CAMPAIGN_STATE,"
	    "decode(PAUSE, 0, 1, 1, 2, 1) as PAUSE,"
	    "CAMPAIGN_BUDGET,"
	    "decode(SCHEDULE_TYPE, 0, 1, 1, 2, 1) as SCHEDULE_TYPE,"
	    "SCHEDULE_SERIALIZE,"
	    "to_char(sysdate, 'DYHH24') as SCHEDULE_SYS_TIME,"
	    "decode(REGION_TYPE, 0, 1, 1, 2, 1) as REGION_TYPE,"
	    "REGION_SERIALIZE,"
	    "to_char(BEGIN_TIME, 'yyyy-mm-dd HH24:MI:SS') as BEGIN_TIME,"
	    "to_char(END_TIME, 'yyyy-mm-dd HH24:MI:SS') as END_TIME,"
	    "decode(sign(trunc(sysdate) - trunc(begin_time)), 1, 1, 0, 1, 2) as sys_sub_begin_time,"
	    "decode(sign(trunc(sysdate) - trunc(end_time)), 1, 2, 0, 1, 1) as sys_sub_end_time "
	    "from T_CAMPAIGN "
	    "where ACCOUNT_ID = %s "
	    "and DISPLAY_STYLE = %u %s",

	    account_id,
	    display_status,
	    where_str);

    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_group_price(const char *campaign_id, char *cpc, char *cpm)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select max(t.cpc_price), max(t.cpm_price) from t_adgroup t where t.campaign_id='%s'", campaign_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(cpc, ad_system_sql_get_string(&sql_conn, NULL, 1));
    strcpy(cpm, ad_system_sql_get_string(&sql_conn, NULL, 2));
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->设置计划
int ad_system_sql_set_campaign(char *account_id,
	const char *campaign_id,
	int update_type,
	int status,
	const char *dudget,
	int region_type,
	const char *region_serialize,
	int schedule_type,
	const char *schedule_serialize,
	int time_type,
	const char *begin_time,
	const char *end_time,
	const char *campign_name)
{

    char comma[4] = {0};
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_CAMPAIGN set ");

    if (update_type & 1) {

	if (status == 1) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=0");
	    comma[0] = ',';

	} else if (status == 2) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=1");
	    comma[0] = ',';
	}
    }

    if (update_type & 2) {

	sql_size += sprintf(&sql_str[sql_size], "%scampaign_budget=%s,"
		"campaign_state  = decode(campaign_state, 13,"
		"decode(sign(%s - budget_cost), 0, campaign_state, -1, campaign_state, 11),"
		"campaign_state) ",
		comma,
		dudget,
		dudget);
	comma[0] = ',';
    }

    if (update_type & 4) {

	if (region_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], "%sREGION_TYPE=0,REGION_SERIALIZE=null", comma);
	else if (region_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], "%sREGION_TYPE=1,REGION_SERIALIZE='%s'", comma, region_serialize);

	comma[0] = ',';
    }

    if (update_type & 8) {

	if (schedule_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], "%sSCHEDULE_TYPE=0,SCHEDULE_SERIALIZE=null", comma);
	else if (schedule_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], "%sSCHEDULE_TYPE=1,SCHEDULE_SERIALIZE='%s'", comma, schedule_serialize);

	comma[0] = ',';
    }

    if (update_type & 16) {

	if (time_type & 1) {

	    sql_size += sprintf(&sql_str[sql_size], "%sEND_TIME=null", comma);
	    comma[0] = ',';

	} else if (time_type & 2) {

	    sql_size += sprintf(&sql_str[sql_size], "%sEND_TIME=to_date('%s 23:59:59', 'YYYY-MM-DD HH24:MI:SS')", comma, end_time);
	    comma[0] = ',';
	}

	if (time_type & 4) {

	    sql_size += sprintf(&sql_str[sql_size], "%sBEGIN_TIME=to_date('%s', 'yyyy-mm-dd')", comma, begin_time);
	    comma[0] = ',';
	}
    }

    if (update_type & 32) {

	sql_size += sprintf(&sql_str[sql_size], "%sCAMPAIGN_NAME='%s'", comma, campign_name);
	comma[0] = ',';
    }

    if (comma[0] == ',')
	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where ACCOUNT_ID=%s and CAMPAIGN_ID='%s'",
		account_id, campaign_id);
    else
	return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取组
int ad_system_sql_get_group(ad_system_sql_t *p,
	const char *account_id, 
	int display_status, 
	int find_type,
	const char *campaign_id, 
	int group_name_type, 
	const char *group_name)
{
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price,"
	    "decode(adgroup.adgroup_state, 21, 1, 22, 2, 1) as group_state,"
	    "decode(campaign.pause, 0, 1, 1, 2, 1) as campaign_pause,"
	    "campaign.campaign_state,"
	    "decode(campaign.schedule_type, 0, 1, 1, 2, 1) as campaign_schedule_type,"
	    "to_char(sysdate, 'DYHH24') as campaign_schedule_sys_time,"
	    "campaign.schedule_serialize as campaign_schedule_serialize,"
	    "decode(sign(trunc(sysdate) - trunc(begin_time)), 1, 1, 0, 1, 2) as sys_sub_begin_time,"
	    "decode(sign(trunc(sysdate) - trunc(end_time)), 1, 2, 0, 1, 1) as sys_sub_end_time,"
	    "decode(adgroup.targeting_type, 2, 1, 3, 2, 1) as targeting_type,"
	    "group_targeting.code as disease_code "
	    "from t_adgroup adgroup "
	    "left join t_campaign campaign on adgroup.campaign_id = campaign.campaign_id "
	    "left join t_adgroup_targeting group_targeting on adgroup.adgroup_id = group_targeting.adgroup_id "
	    "where adgroup.account_id=%s and adgroup.display_style=%u",
	account_id, 
	display_status);

    if (find_type & 2) {

	if (group_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and adgroup.adgroup_name='%s'", group_name);

	else if (group_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and adgroup.adgroup_name like '%%%s%%'", group_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and adgroup.campaign_id='%s'", campaign_id);
    }

    return ad_system_sql_select(p, sql_str);
}

//推广->获取创意预算
int ad_system_sql_get_campaign_budget(const char *account_id, const char *campaign_id, const char *group_id, char *budget, char *bidding_model)
{

    char sql_str[4096] = {0};
    if (campaign_id)
	sprintf(sql_str, "select t.campaign_budget from t_campaign t where t.account_id=%s and t.campaign_id='%s'", account_id, campaign_id);
    else
	sprintf(sql_str, "select "
		"campaign.campaign_budget,"
		"adgroup.bidding_model "
		"from t_adgroup adgroup "
		"left join t_campaign campaign on adgroup.CAMPAIGN_ID=campaign.campaign_id "
		"where adgroup.account_id=%s and adgroup.adgroup_id='%s'",
		account_id, group_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(budget, ad_system_sql_get_string(&sql_conn, NULL, 1));
    if (group_id) strcpy(bidding_model, ad_system_sql_get_string(&sql_conn, NULL, 2));
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取预算与组出价
int ad_system_sql_get_budget_and_bidding(ad_system_sql_t *p, char *account_id, const char *group_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "campaign.campaign_budget,"
	    "adgroup.bidding_model,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price "
	    "from t_adgroup adgroup "
	    "left join t_campaign campaign on adgroup.CAMPAIGN_ID=campaign.campaign_id "
	    "where adgroup.account_id=%s and adgroup.adgroup_id='%s'",
	    account_id, group_id);

    return ad_system_sql_select(p, sql_str);
}

//推广->设置组
int ad_system_sql_set_group(
	const char *account_id,
	const char *group_id,
	int update_type,
	int status,
	const char *cpc_price,
	const char *cpm_price,
	int bidding_model,
	int targeting_type,
	const char *group_name)
{

    char comma[4] = {0};
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_ADGROUP set ");

    if (update_type & 1) {

	if (status == 1) {

	    sql_size += sprintf(&sql_str[sql_size], "ADGROUP_STATE=21");
	    comma[0] = ',';

	} else if (status == 2) {

	    sql_size += sprintf(&sql_str[sql_size], "ADGROUP_STATE=22");
	    comma[0] = ',';
	}
    }

    if (update_type & 2) {

	sql_size += sprintf(&sql_str[sql_size], "%sCPC_PRICE=%s", comma, cpc_price);
	comma[0] = ',';
    }

    if (update_type & 4) {

	if (targeting_type == 1) {

	    sql_size += sprintf(&sql_str[sql_size], "%sTARGETING_TYPE=2", comma);
	    comma[0] = ',';

	} else if (targeting_type == 2) {

	    sql_size += sprintf(&sql_str[sql_size], "%sTARGETING_TYPE=3", comma);
	    comma[0] = ',';
	}
    }

    if (update_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], "%sADGROUP_NAME='%s'", comma, group_name);
	comma[0] = ',';
    }

    if (update_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], "%sCPM_PRICE=%s", comma, cpm_price);
	comma[0] = ',';
    }

    if (update_type & 2 || update_type & 16) {

	if (update_type & 16) {

	    if (strcmp(cpm_price, "null") == 0)
		bidding_model &= ~1;
	    else
		bidding_model |= 1;
	}

	if (update_type & 2) {

	    if (strcmp(cpc_price, "null") == 0)
		bidding_model &= ~2;
	    else
		bidding_model |= 2;
	}

	sql_size += sprintf(&sql_str[sql_size], ",bidding_model=%u", bidding_model);

#if 0
	int GROUP_PRICE_TYPE = 0;
	if (update_type & 2)
	    GROUP_PRICE_TYPE = 1;

	if (update_type & 16)
	    GROUP_PRICE_TYPE += 2;

	if (GROUP_PRICE_TYPE == 1) {

	    if (strcmp(cpm_price, "null") == 0)
		sql_size += sprintf(&sql_str[sql_size], ",bidding_model = BITAND(bidding_model, (0 - 1) - 1)");
	    else
		sql_size += sprintf(&sql_str[sql_size], ",bidding_model = (bidding_model + 1) - BITAND(bidding_model, 1)");

	} else if (GROUP_PRICE_TYPE == 2) {

	    if (strcmp(cpc_price, "null") == 0) 
		sql_size += sprintf(&sql_str[sql_size], ",bidding_model = BITAND(bidding_model, (0 - 2) - 1)");
	    else
		sql_size += sprintf(&sql_str[sql_size], ",bidding_model = (bidding_model + 2) - BITAND(bidding_model, 2)");

	} else if (GROUP_PRICE_TYPE == 3) {

	    if (strcmp(cpm_price, "null") == 0) 
		sql_size += sprintf(&sql_str[sql_size], 
			",bidding_model = BITAND((bidding_model + 2) - BITAND(bidding_model, 2),(0 - 1) - 1)");
	    else if (strcmp(cpc_price, "null") == 0)
		sql_size += sprintf(&sql_str[sql_size], 
			",bidding_model = BITAND((bidding_model + 1) - BITAND(bidding_model, 1),(0 - 2) - 1)");
	    else
		sql_size += sprintf(&sql_str[sql_size], ",bidding_model = 3");
	}
#endif
    }

    if (comma[0] == ',')
	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where ACCOUNT_ID=%s and ADGROUP_ID='%s'",
		account_id, group_id);
    else
	return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->设置组附加属性
int ad_system_sql_set_group_targeting(
	const char *account_id,
	const char *group_id,
	int update_type,
	int targeting_type,
	const char *disease,
	const char *keyword)
{

    char comma[4] = " ";
    char sql_str[409600] = {0};
    size_t sql_size = sprintf(sql_str, "update T_ADGROUP_TARGETING set ");

    if (update_type & 4 && targeting_type == 1) {

	sql_size += sprintf(&sql_str[sql_size], "CODE='%s'", disease);
	comma[0] = ',';

    } else {

	return 0;
    }

    //else if (update_type & 4 && targeting_type == 2)
    //return ad_system_sql_get_group_set_keywoed(group_id, keyword);

    if (comma[0] == ',')
	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate where ACCOUNT_ID=%s and ADGROUP_ID='%s'", 
		account_id, group_id);
    else
	return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取文字创意
int ad_system_sql_get_creative_text(ad_system_sql_t *p,
	const char *account_id, 
	int find_type,
	const char *campaign_id, 
	const char *group_id, 
	int find_name_type, 
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id as group_id,"
	    "adgroup.adgroup_name as group_name,"
	    "creative.creative_id,"
	    "creative.title as creative_name,"
	    "decode(creative.pause, 0, 1, 1, 2, 1) as creative_pause,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.description1 as creative_description,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    "creative.append_id,"
	    "creative.mobile_destination_url,"
	    "creative.mobile_display_url "
	    "from t_text_creative creative "
	    "left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id "
	    "left join t_campaign campaign on creative.campaign_id = campaign.campaign_id "
	    "where creative.account_id = %s ",
	    account_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.TITLE='%s'", find_name);
	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.TITLE like '%%%s%%'", find_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.CAMPAIGN_ID='%s'", campaign_id);
    }

    if (find_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.ADGROUP_ID='%s'", group_id);
    }

    return ad_system_sql_select(p, sql_str);
}

#if 0
//推广->获取图片创意
int ad_system_sql_get_img_creative(ad_system_sql_t *p,
	const char *account_id, 
	int find_type,
	const char *campaign_id, 
	const char *group_id, 
	int find_name_type, 
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "creative.creative_id,"
	    "creative.creative_name,"
	    "decode(creative.pause, 0, 1, 1, 2, 1) as creative_pause,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.img_size,"
	    "creative.img_size_imglength,"
	    "creative.file_path,"
	    "creative.destination_url,"
	    "creative.display_url "
	    "from t_creative creative "
	    "left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id "
	    "left join t_campaign campaign on creative.campaign_id = campaign.campaign_id "
	    "where creative.account_id = %s ",
	    account_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name = '%s'", find_name);
	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name like '%%%s%%'", find_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.CAMPAIGN_ID='%s'", campaign_id);
    }

    if (find_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.ADGROUP_ID='%s'", group_id);
    }

    return ad_system_sql_select(p, sql_str);
}
#endif

//推广->创建图片创意
int ad_system_sql_add_creative_img(
	const char *account_id,
	const char *campaign_id,
	const char *group_id,
	const char *creative_name,
	const char *img_size,
	const char *img_buffer_size,
	const char *img_path,
	const char *creative_destination_url,
	const char *creative_display_url)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_CREATIVE(CREATIVE_ID,ACCOUNT_ID,CAMPAIGN_ID,ADGROUP_ID,CREATIVE_NAME,IMG_SIZE,IMG_SIZE_IMGLENGTH,FILE_PATH,DESTINATION_URL,DISPLAY_URL,CREATIVE_STATE,PAUSE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE) VALUES (SEQ_T_CREATIVE.nextval, %s, '%s', '%s', '%s', '%s', %s, '%s', '%s', '%s', 34, 0, sysdate, sysdate, 0, 0, 1)",
	    account_id,
	    campaign_id,
	    group_id,
	    creative_name,
	    img_size,
	    img_buffer_size,
	    img_path,
	    creative_destination_url,
	    creative_display_url);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->修改图片创意
int ad_system_sql_set_creative_img(
	const char *account_id,
	const char *creative_id,
	int creative_status)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_CREATIVE set ");

    if (creative_status == 1) {

	sql_size += sprintf(&sql_str[sql_size], "PAUSE=0");

    } else if (creative_status == 2) {

	sql_size += sprintf(&sql_str[sql_size], "PAUSE=1");
    }

    sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where ACCOUNT_ID=%s and CREATIVE_ID='%s'",
	    account_id, creative_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->修改文字创意
int ad_system_sql_set_creative_text(const char *creative_id, int status)
#if 0
    const char *account_id,
    const char *creative_id,
    int update_type,
    int status,
    const char *creative_name,
    const char *descriptionpriec,
    const char *destination_url,
    const char *creative_display_url,
    int creative_append_type,
    const char *creative_append_id,
    const char *mobile_destination_url,
    const char *mobile_display_url)
#endif
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE set ");

    if (status == 1)
	sql_size += sprintf(&sql_str[sql_size], "PAUSE=0");

    else if (status == 2)
	sql_size += sprintf(&sql_str[sql_size], "PAUSE=1");

    else
	return -1;

    sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where CREATIVE_ID='%s'", creative_id);

#if 0
    char comma[4] = " ";
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE set ");
    int CREATIVE_STATE = 0;

    if (update_type & 1) {

	if (status == 1) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=0");
	    comma[0] = ',';

	} else if (status == 2) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=1");
	    comma[0] = ',';
	}
    }

    if (update_type & 2) {

	sql_size += sprintf(&sql_str[sql_size], "%sTITLE='%s'", comma, creative_name);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 4) {

	sql_size += sprintf(&sql_str[sql_size], "%sDESCRIPTION1='%s'", comma, descriptionpriec);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], "%sDESTINATION_URL='%s'", comma, destination_url);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], "%sDISPLAY_URL='%s'", comma, creative_display_url);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (creative_append_type == 1) {

	sql_size += sprintf(&sql_str[sql_size], "%sAPPEND_ID=null", comma);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 32) {

	sql_size += sprintf(&sql_str[sql_size], "%sAPPEND_ID='%s'", comma, creative_append_id);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 64) {

	sql_size += sprintf(&sql_str[sql_size], "%sMOBILE_DESTINATION_URL='%s'", comma, mobile_destination_url);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (update_type & 128) {

	sql_size += sprintf(&sql_str[sql_size], "%sMOBILE_DISPLAY_URL='%s'", comma, mobile_display_url);
	comma[0] = ',';
	CREATIVE_STATE = 1;
    }

    if (comma[0] == ',') {

	if (CREATIVE_STATE)
	    sql_size += sprintf(&sql_str[sql_size], ",CREATIVE_STATE=34");

	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where ACCOUNT_ID=%s and CREATIVE_ID='%s'", 
		account_id, creative_id);

    } else {

	return -1;
    }
#endif

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取附加创意
int ad_system_sql_get_creative_append(ad_system_sql_t *p,
	const char *account_id, 
	int find_type,
	int sort_type,
	int sort_column,
	const char *find_name, 
	int status)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "append_id,decode(pause,1,3,decode(creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1)) as creative_append_status,"
	    "content,"
	    "refuse_reason "
	    "from t_text_creative_append where row_state=1 and account_id=%s", account_id);

    if (find_type & 2)
	sql_size += sprintf(&sql_str[sql_size], " and content like '%%%s%%'", find_name);

    if (find_type & 4) {

	if (status != 100) {

	    if (status == 1) {
		sql_size += sprintf(&sql_str[sql_size], " and CREATIVE_STATE=31 and PAUSE=0");

	    } else if (status == 2) {
		sql_size += sprintf(&sql_str[sql_size], " and CREATIVE_STATE=32 and PAUSE=0");

	    } else if (status == 3) {
		sql_size += sprintf(&sql_str[sql_size], " and PAUSE=1");

	    } else if (status == 4) {
		sql_size += sprintf(&sql_str[sql_size], " and CREATIVE_STATE=34 and PAUSE=0");

	    } else if (status == 5) {
		sql_size += sprintf(&sql_str[sql_size], " and CREATIVE_STATE=35 and PAUSE=0");

	    } else {

		return -1;
	    }
	}
    }

    if (sort_column == 1) {

	if (sort_type == 1) {
	    sql_size += sprintf(&sql_str[sql_size], " order by content");

	} else if (sort_type == 2) {
	    sql_size += sprintf(&sql_str[sql_size], " order by content desc");
	}

    } else if (sort_column == 2) {

	if (sort_type == 1) {
	    sql_size += sprintf(&sql_str[sql_size], " order by creative_append_status");

	} else if (sort_type == 2) {
	    sql_size += sprintf(&sql_str[sql_size], " order by creative_append_status desc");
	}
    }

    return ad_system_sql_select(p, sql_str);
}

//推广->修改附加创意
int ad_system_sql_set_creative_append(
	const char *account_id,
	const char *creative_append_id,
	int update_type,
	int status,
	const char *contend)
{

    char comma[4] = {0};
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "update T_TEXT_CREATIVE_APPEND set ");

    if (update_type & 1) {

	if (status == 1) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=0");
	    comma[0] = ',';

	} else if (status == 2) {

	    sql_size += sprintf(&sql_str[sql_size], "PAUSE=1");
	    comma[0] = ',';
	}
    }

    if (update_type & 2) {

	sql_size += sprintf(&sql_str[sql_size], "%sCONTENT='%s',CREATIVE_STATE=34", comma, contend);
	comma[0] = ',';
    }

    if (comma[0] == ',')
	sql_size += sprintf(&sql_str[sql_size], ",LAST_UPDATE_TIME=sysdate,SYNCH_STATE=0 where ACCOUNT_ID=%s and APPEND_ID='%s'", 
		account_id, creative_append_id);
    else return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//获取创意ID 与组ID
int ad_system_sql_get_campaign_id_from_group_or_creative(char *campaign_id, char *group_id, char *creative_id)
{

    char sql_str[1024] = {0};

    if (group_id) 
	sprintf(sql_str, "select campaign_id from t_adgroup where adgroup_id='%s'", group_id);
    if (creative_id) 
	sprintf(sql_str, "select campaign_id, adgroup_id from t_creative where creative_id='%s'", creative_id);

    fprintf(stderr, "%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!(ad_system_sql_fetch_next(&sql_conn)))
	return -1;

    sprintf(campaign_id, "%s", ad_system_sql_get_string(&sql_conn, NULL, 1));
    if (creative_id)
	sprintf(group_id, "%s", ad_system_sql_get_string(&sql_conn, NULL, 2));

    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->添加文字创意
int ad_system_sql_add_creative_text(const char *account_id,
	const char *campaign_id,
	const char *group_id,
	const char *creative_name,
	const char *descriptionpriec,
	const char *destination_url,
	const char *creative_display_url,
	int creative_append_type,
	const char *creative_append_id,
	const char *mobile_destination_url,
	const char *mobile_display_url)
{

    char creative_append_id_str[1024] = "null";
    if (creative_append_type == 2)
	sprintf(creative_append_id_str, "'%s'", creative_append_id);

    char mobile_destination_url_str[1024] = "null";
    if (*mobile_destination_url)
	sprintf(mobile_destination_url_str, "'%s'", mobile_destination_url);

    char mobile_display_url_str[1024] = "null";
    if (*mobile_display_url)
	sprintf(mobile_display_url_str, "'%s'", mobile_destination_url);

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_TEXT_CREATIVE (CREATIVE_ID,ACCOUNT_ID,CAMPAIGN_ID,ADGROUP_ID,TITLE,DESCRIPTION1,DESTINATION_URL,DISPLAY_URL,CREATIVE_STATE,PAUSE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE,APPEND_ID,mobile_destination_url,mobile_display_url) VALUES (SEQ_T_TEXT_CREATIVE.Nextval,%s,'%s','%s','%s','%s','%s','%s',34,0,sysdate,sysdate,0,0,1,%s,%s,%s)",
	    account_id,
	    campaign_id,
	    group_id,
	    creative_name,
	    descriptionpriec,
	    destination_url,
	    creative_display_url,
	    creative_append_id_str,
	    mobile_destination_url_str,
	    mobile_display_url_str);

    fprintf(stderr, "-->%s\n", sql_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_copy_creative_text(
	int update_update,
	const char *creative_id,
	const char *creative_name,
	const char *descriptionpriec,
	const char *destination_url,
	const char *creative_display_url,
	int creative_append_type,
	const char *creative_append_id,
	const char *mobile_destination_url,
	const char *mobile_display_url)
{

    char TITLE[1024] = "TITLE";
    if (update_update & 2)
	sprintf(TITLE, "'%s'", creative_name);

    char DESCRIPTION1[1024] = "DESCRIPTION1";
    if (update_update & 4)
	sprintf(DESCRIPTION1, "'%s'", descriptionpriec);

    char DESTINATION_URL[1024] = "DESTINATION_URL";
    if (update_update & 8)
	sprintf(DESTINATION_URL, "'%s'", destination_url);

    char DISPLAY_URL[1024] = "DISPLAY_URL";
    if (update_update & 16)
	sprintf(DISPLAY_URL, "'%s'", creative_display_url);

    char APPEND_ID[1024] = "APPEND_ID";
    if (update_update & 32) {

	if (creative_append_type == 1)
	    sprintf(APPEND_ID, "null");
	else
	    sprintf(APPEND_ID, "'%s'", creative_append_id);
    }

    char MOBILE_DESTINATION_URL[1024] = "MOBILE_DESTINATION_URL";
    if (update_update & 64)
	sprintf(MOBILE_DESTINATION_URL, "'%s'", mobile_destination_url);

    char MOBILE_DISPLAY_URL[1024] = "MOBILE_DISPLAY_URL";
    if (update_update & 128)
	sprintf(MOBILE_DISPLAY_URL, "'%s'", mobile_display_url);

    char sql_str[4096] = {0};
    sprintf(sql_str,
	    "INSERT INTO T_TEXT_CREATIVE "
	    "(CREATIVE_ID,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "TITLE,"
	    "DESCRIPTION1,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "CREATIVE_STATE,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE,"
	    "APPEND_ID,"
	    "MOBILE_DESTINATION_URL,"
	    "MOBILE_DISPLAY_URL) "
	    "SELECT SEQ_T_TEXT_CREATIVE.NEXTVAL,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "%s," //TITLE,"
	    "%s," //DESCRIPTION1,"
	    "%s," //DESTINATION_URL,"
	    "%s," //DISPLAY_URL,"
	    "34," //CREATIVE_STATE,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "0," //SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE,"
	    "%s," //APPEND_ID,"
	    "%s," //MOBILE_DESTINATION_URL,"
	    "%s " //MOBILE_DISPLAY_URL
	    "FROM T_TEXT_CREATIVE T "
	    "WHERE T.CREATIVE_ID = '%s'",

	TITLE,
	DESCRIPTION1,
	DESTINATION_URL,
	DISPLAY_URL,
	APPEND_ID,
	MOBILE_DESTINATION_URL,
	MOBILE_DISPLAY_URL,
	creative_id
	    );

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}


//推广->添加附加创意
int ad_system_sql_add_creative_append(const char *account_id, const char *append_contend)
{

    char sql_str[4096] = {0};
    sprintf(sql_str,"INSERT INTO T_TEXT_CREATIVE_APPEND (APPEND_ID,ACCOUNT_ID,CONTENT,CREATIVE_STATE,PAUSE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE) VALUES (seq_t_text_creative_append.nextval,%s,'%s', 34, 0, sysdate, sysdate, 0, 0, 1)", account_id, append_contend);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->验证组重命名
int ad_system_sql_group_name_check(const char *campaign_id, const int display_status, const char *group_name)
{

    char sql_str[1024] = {0};
    sprintf(sql_str,
	    "select ADGROUP_ID from T_ADGROUP where DISPLAY_STYLE=%u and CAMPAIGN_ID='%s'and ADGROUP_NAME='%s'",
	    display_status,
	    campaign_id,
	    group_name);

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

//推广->获取新的组ID
int ad_system_sql_get_group_id(char *group_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select SEQ_T_ADGROUP.NEXTVAL from dual");

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(group_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->添加组
int ad_system_sql_add_group(
	const char *account_id,
	int display_status,
	const char *campaign_id,
	const char *group_id,
	const char *group_name,
	int group_price_type,
	const char *group_cpc_price,
	const char *group_cpm_price,
	int targeting_type)
{

    if (targeting_type == 1)
	targeting_type = 2;
    else if (targeting_type == 2)
	targeting_type = 3;

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_ADGROUP(ADGROUP_ID,ACCOUNT_ID,CAMPAIGN_ID,ADGROUP_NAME,CPC_PRICE,CPM_PRICE,TARGETING_TYPE,ADGROUP_STATE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE,BIDDING_MODEL,DISPLAY_STYLE) VALUES ('%s',%s,'%s','%s',%s,%s,%u,21,sysdate,sysdate,0,1,1,%u,%u)",
	    group_id,
	    account_id,
	    campaign_id,
	    group_name,
	    group_price_type&2?group_cpc_price:"null",
	    group_price_type&1?group_cpm_price:"null",
	    targeting_type,
	    group_price_type,
	    display_status);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->添加组附属属性
int ad_system_sql_add_group_targeting(
	const char *account_id,
	const char *campaign_id,
	const char *group_id,
	int targeting_type,
	const char *disease,
	const char *keyword)
{

    char disease_str[1024] = "null";
    if (targeting_type == 1)
	sprintf(disease_str, "'%s'", disease);

    char sql_str[409600] = {0};
    sprintf(sql_str, "INSERT INTO T_ADGROUP_TARGETING (ADGROUP_TARGETING_ID,ACCOUNT_ID,CAMPAIGN_ID,ADGROUP_ID,CREATE_TIME,LAST_UPDATE_TIME,CODE,KEYWORD) VALUES (SEQ_T_ADGROUP_TARGETING.nextval,%s,'%s','%s',sysdate,sysdate,%s, null)",
	    account_id,
	    campaign_id,
	    group_id,
	    disease_str);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取新的计划ID
int ad_system_sql_get_campaign_id(char *campaign_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select SEQ_T_CAMPAIGN.NEXTVAL from dual");

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(campaign_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->验证计划重命名
int ad_system_sql_campaign_name_check(const char *account_id, const char *campaign_name, int display_style)
{

    char sql_str[1024] = {0};
    sprintf(sql_str,
	    "select campaign_id from t_campaign where account_id = %s and campaign_name ='%s' and display_style=%d",
	    account_id,
	    campaign_name,
	    display_style);

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

//推广->添加计划
int ad_system_sql_add_campaign(
	const char *account_id,
	int display_status,
	const char *campaign_id,
	const char *campaign_name,
	const char *dudget,
	int region_type,
	const char *region_serialize,
	int schedule_type,
	const char *schedule_serialize,
	int time_type,
	const char *begin_time,
	const char *end_time)
{

    char begin_time_sql[256] = "null";
    sprintf(begin_time_sql, "to_date('%s', 'YYYY-MM-DD')", begin_time);

    char end_time_sql[256] = "null";
    if (time_type == 2)
	sprintf(end_time_sql, "to_date('%s', 'YYYY-MM-DD')", end_time);


    char region_serialize_sql[1024] = "null";
    if (region_type == 1)
	region_type = 0;
    else {

	region_type = 1;
	sprintf(region_serialize_sql, "'%s'", region_serialize);
    }

    char schedule_serialize_sql[1024] = "null";
    if (schedule_type == 1)
	schedule_type = 0;
    else {

	schedule_type = 1;
	sprintf(schedule_serialize_sql, "'%s'", schedule_serialize);
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_CAMPAIGN (CAMPAIGN_ID,ACCOUNT_ID,CAMPAIGN_NAME,CAMPAIGN_BUDGET,REGION_TYPE,REGION_SERIALIZE,BEGIN_TIME,END_TIME,SCHEDULE_TYPE,SCHEDULE_SERIALIZE,CAMPAIGN_STATE,PAUSE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE,BIDDING_MODEL,DISPLAY_STYLE) VALUES ('%s', %s, '%s', %s, %u, %s, %s, %s, %u, %s, 11, 0, sysdate, sysdate, 0, 1, 1, 1,%u)",
	    campaign_id,
	    account_id,
	    campaign_name,
	    dudget,
	    region_type,
	    region_serialize_sql,
	    begin_time_sql,
	    end_time_sql,
	    schedule_type,
	    schedule_serialize_sql,
	    display_status);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int _ad_system_sql_del_creative_img(ad_system_sql_t *p,
	const char *campaign_id, 
	const char *group_id, 
	const char *creative_id)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "insert into t_deleted_creative(creative_id,account_id,campaign_id,adgroup_id,creative_name,img_size,img_size_imglength,file_path,file_store_id,destination_url,display_url,ectr,creative_state,refuse_reason,pause,has_update_temp,quality,impression,click,cost,dialog,appointment,arrival,create_time,last_update_time,auditor_id,deleted_time,synch_state) select creative_id,account_id,campaign_id,adgroup_id,creative_name,img_size,img_size_imglength,file_path,file_store_id,destination_url,display_url,ectr,creative_state,refuse_reason,pause,has_update_temp,quality,impression,click,cost,dialog,appointment,arrival,create_time,last_update_time,auditor_id,sysdate,0 from t_creative t where ");

    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else if (creative_id)
	sql_size += sprintf(&sql_str[sql_size], " t.creative_id='%s'", creative_id);
    else
	return -1;

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    sql_size = sprintf(sql_str, "delete from t_creative t where ");
    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else if (creative_id)
	sql_size += sprintf(&sql_str[sql_size], " t.creative_id='%s'", creative_id);

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    return 0;
}

int _ad_system_sql_del_creative_text(ad_system_sql_t *p, 
	const char *campaign_id, 
	const char *group_id, 
	const char *creative_id)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "insert into t_deleted_text_creative t_del (t_del.creative_id,t_del.account_id,t_del.campaign_id,t_del.adgroup_id,t_del.title,t_del.description1,t_del.description2,t_del.destination_url,t_del.display_url,t_del.ectr,t_del.creative_state,t_del.refuse_reason,t_del.pause,t_del.quality,t_del.impression,t_del.click,t_del.cost,t_del.dialog,t_del.appointment,t_del.arrival,t_del.create_time,t_del.last_update_time,t_del.auditor_id,t_del.append_id,t_del.deleted_time,t_del.synch_state) select t.creative_id,t.account_id,t.campaign_id,t.adgroup_id,t.title,t.description1,t.description2,t.destination_url,t.display_url,t.ectr,t.creative_state,t.refuse_reason,t.pause,t.quality,t.impression,t.click,t.cost,t.dialog,t.appointment,t.arrival,t.create_time,t.last_update_time,t.auditor_id,t.append_id,sysdate,0 from t_text_creative t where ");

    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else if (creative_id)
	sql_size += sprintf(&sql_str[sql_size], " t.creative_id='%s'", creative_id);
    else
	return -1;

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    sql_size = sprintf(sql_str, "delete from t_text_creative t where ");
    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else if (creative_id)
	sql_size += sprintf(&sql_str[sql_size], " t.creative_id='%s'", creative_id);
    else
	return -1;

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    return 0;
}

int _ad_system_sql_del_group(ad_system_sql_t *p,
	const char *campaign_id, 
	const char *group_id)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "insert into t_deleted_adgroup t_del (t_del.adgroup_id,t_del.account_id,t_del.campaign_id,t_del.adgroup_name,t_del.cpc_price,t_del.cpm_price,t_del.targeting_type,t_del.adgroup_state,t_del.impression,t_del.click,t_del.cost,t_del.dialog,t_del.appointment,t_del.arrival,t_del.create_time,t_del.last_update_time,t_del.deleted_time,t_del.synch_state) select t.adgroup_id,t.account_id,t.campaign_id,t.adgroup_name,t.cpc_price,t.cpm_price,t.targeting_type,t.adgroup_state,t.impression,t.click,t.cost,t.dialog,t.appointment,t.arrival,t.create_time,t.last_update_time,sysdate,0 from t_adgroup t where ");
    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else
	return -1;

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    sql_size = sprintf(sql_str, "delete from t_adgroup t where ");
    if (campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " t.campaign_id='%s'", campaign_id);
    else if (group_id)
	sql_size += sprintf(&sql_str[sql_size], " t.adgroup_id='%s'", group_id);
    else
	return -1;

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    return 0;
}

int _ad_system_sql_del_campaign(ad_system_sql_t *p, const char *campaign_id)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "insert into t_deleted_campaign t_del (t_del.campaign_id,t_del.account_id,t_del.campaign_name,t_del.campaign_budget,t_del.region_type,t_del.region_serialize,t_del.begin_time,t_del.end_time,t_del.schedule_type,t_del.schedule_serialize,t_del.maximp,t_del.campaign_state,t_del.pause,t_del.impression,t_del.click,t_del.cost,t_del.dialog,t_del.appointment,t_del.arrival,t_del.create_time,t_del.last_update_time,t_del.deleted_time,t_del.synch_state,t_del.bidding_model) select t.campaign_id,t.account_id,t.campaign_name,t.campaign_budget,t.region_type,t.region_serialize,t.begin_time,t.end_time,t.schedule_type,t.schedule_serialize,t.maximp,t.campaign_state,t.pause,t.impression,t.click,t.cost,t.dialog,t.appointment,t.arrival,t.create_time,t.last_update_time,sysdate,0,t.bidding_model from t_campaign t where t.campaign_id='%s'", campaign_id);

    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    sql_size = sprintf(sql_str, "delete from t_campaign t where t.campaign_id='%s'", campaign_id);
    if (_ad_system_sql_execute(p, sql_str))
	return -1;

    return 0;
}

//删除计划
int ad_system_sql_del_campaign(const char *campaign_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    if (_ad_system_sql_del_creative_img(&sql_conn, campaign_id, NULL, NULL)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (_ad_system_sql_del_creative_text(&sql_conn, campaign_id, NULL, NULL)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (_ad_system_sql_del_group(&sql_conn, campaign_id, NULL)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (_ad_system_sql_del_campaign(&sql_conn, campaign_id)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//删除组
int ad_system_sql_del_group(const char *group_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    if (_ad_system_sql_del_creative_img(&sql_conn, NULL, group_id, NULL)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (_ad_system_sql_del_creative_text(&sql_conn, NULL, group_id, NULL)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (_ad_system_sql_del_group(&sql_conn, NULL, group_id)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//删除图片创意
int ad_system_sql_del_creative_img(const char *creative_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    if (_ad_system_sql_del_creative_img(&sql_conn, NULL, NULL, creative_id)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//删除文字创意
int ad_system_sql_del_creative_text(const char *creative_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    if (_ad_system_sql_del_creative_text(&sql_conn, NULL, NULL, creative_id)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//删除附加创意
int ad_system_sql_del_creative_append(const char *append_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "update t_text_creative_append set row_state=0,SYNCH_STATE=0 where append_id='%s'", append_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//财务->充值记录
int ad_system_sql_get_money_recharge(ad_system_sql_t *p,
	const char *account_id,
	int find_type,
	int money_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end,
	int sort_type,
	int sort_column,
	int page_index_s,
	int page_index_e)
{

    size_t sort_size = 0;
    char sort_str[1024] = {0};
    switch(sort_column){

	case 1:
	    sort_size = sprintf(sort_str, "swift_number");
	    break;

	case 2:
	    sort_size = sprintf(sort_str, "money");
	    break;

	case 3:
	    sort_size = sprintf(sort_str, "payment_type");
	    break;

	case 4:
	    sort_size = sprintf(sort_str, "invest");
	    break;

	case 5:
	    sort_size = sprintf(sort_str, "current_money_balance");
	    break;

	case 6:
	    sort_size = sprintf(sort_str, "create_time");
	    break;
    }

    if (sort_type == 2)
	sprintf(&sort_str[sort_size], " desc");

    size_t size = 0;
    char where_str[1024] = {0};
    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and swift_number like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and accounting_document_number like '%%%s%%'", find_name);
		break;
	}
    }

    if (money_type != 100)
	size += sprintf(&where_str[size], "and payment_type=%u", money_type);

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and create_time > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and create_time < to_date('%s 23:59:59', 'yyyy-mm-dd hh24:mi:ss') ", time_end);
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "swift_number,"
	    "money,"
	    "payment_type,"
	    "accounting_document_number,"
	    "accounting_document_file,"
	    "invest,"
	    "current_money_balance,"
	    "create_time "
	    "from (select "
	    "rownum as page_index,"
	    "nvl(to_char(swift_number), 'null') as swift_number,"
	    "nvl(to_char(money), 'null') as money,"
	    "nvl(to_char(current_money_balance), 'null') as current_money_balance,"
	    "nvl(to_char(invest), 'null') as invest,"
	    "nvl(to_char(payment_type), 'null') as payment_type,"
	    "nvl(to_char(accounting_document_number), 'null') as accounting_document_number,"
	    "nvl(to_char(accounting_document_file), 'null') as accounting_document_file,"
	    "nvl(to_char(accounting_document_remark), 'null') as accounting_document_remark,"
	    "nvl(to_char(create_time, 'yyyy-mm-dd'), 'null') as create_time "
	    "from t_recharge "
	    "where BRAND_TYPE=0 and account_id=%s  %s order by %s) t "
	    "where t.page_index between %u and %u",

	account_id,
	where_str,
	sort_str,
	page_index_s,
	page_index_e);

    return ad_system_sql_select(p, sql_str);
}

//财务->充值记录数量
int ad_system_sql_get_money_recharge_count(ad_system_sql_t *p,
	const char *account_id,
	int find_type,
	int money_type,
	int find_name_type,
	const char *find_name,
	const char *time_start,
	const char *time_end)
{


    size_t size = 0;
    char where_str[1024] = {0};
    if (find_type & 1) {

	switch(find_name_type) {

	    case 1:
		size += sprintf(&where_str[size], "and swift_number like '%%%s%%' ", find_name);
		break;

	    case 2:
		size += sprintf(&where_str[size], "and accounting_document_number like '%%%s%%' ", find_name);
		break;
	}
    }

    if (money_type != 100)
	size += sprintf(&where_str[size], "and payment_type=%u", money_type);

    if (find_type & 2) {

	size += sprintf(&where_str[size], "and create_time > to_date('%s', 'yyyy-mm-dd') ", time_start);
	size += sprintf(&where_str[size], "and create_time < to_date('%s 23:59:59', 'yyyy-mm-dd hh24:mi:ss') ", time_end);
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "select count(*) from t_recharge where account_id = %s %s", account_id, where_str);
    return ad_system_sql_select(p, sql_str);
}

//推广->附加创意获取(单个)
int ad_system_sql_get_creative_append_unique(ad_system_sql_t *p, const char *creative_append_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "append_id,"
	    "decode(pause,1,3,decode(creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1)) as creative_append_status,"
	    "content,"
	    "refuse_reason "
	    "from t_text_creative_append "
	    "where row_state=1 and append_id='%s'",
	    creative_append_id);

    return ad_system_sql_select(p, sql_str);
}

//推广->获取组(单个)
int ad_system_sql_get_group_unique(ad_system_sql_t *p,	const char *group_id)
{
    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price,"
	    "decode(adgroup.adgroup_state, 21, 1, 22, 2, 1) as group_state,"
	    "decode(campaign.pause, 0, 1, 1, 2, 1) as campaign_pause,"
	    "campaign.campaign_state,"
	    "decode(campaign.schedule_type, 0, 1, 1, 2, 1) as campaign_schedule_type,"
	    "to_char(sysdate, 'DYHH24') as campaign_schedule_sys_time,"
	    "campaign.schedule_serialize as campaign_schedule_serialize,"
	    "decode(sign(trunc(sysdate) - trunc(begin_time)), 1, 1, 0, 1, 2) as sys_sub_begin_time,"
	    "decode(sign(trunc(sysdate) - trunc(end_time)), 1, 2, 0, 1, 1) as sys_sub_end_time,"
	    "decode(adgroup.targeting_type, 2, 1, 3, 2, 1) as targeting_type,"
	    "group_targeting.code as disease_code "
	    "from t_adgroup adgroup "
	    "left join t_campaign campaign on adgroup.campaign_id = campaign.campaign_id "
	    "left join t_adgroup_targeting group_targeting on adgroup.adgroup_id = group_targeting.adgroup_id "
	    "where adgroup.adgroup_id='%s'",
	group_id);

    return ad_system_sql_select(p, sql_str);
}


//推广->获取组数量
int ad_system_sql_get_group_count(ad_system_sql_t *p,
	const char *account_id, 
	int display_status, 
	int find_type,
	const char *campaign_id, 
	int group_name_type, 
	const char *group_name,
	char *count)
{
    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select count(*) "
	    "from t_adgroup adgroup "
	    "where adgroup.account_id=%s and adgroup.display_style=%u",
	    account_id, 
	    display_status);

    if (find_type & 2) {

	if (group_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and adgroup.adgroup_name='%s'", group_name);

	else if (group_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and adgroup.adgroup_name like '%%%s%%'", group_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and adgroup.campaign_id='%s'", campaign_id);
    }

    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_group_set_keywoed(const char *group_id, const char *keyword)
{ 

    OCI_Connection *cn = NULL;
    if (!(cn = OCI_PoolGetConnection(ad_system_sql_pool, NULL)))
	return -1;

    OCI_Statement* stmt = OCI_StatementCreate(cn);
    OCI_Lob *clob = OCI_LobCreate(cn, OCI_CLOB);

    OCI_Prepare(stmt, "UPDATE T_ADGROUP_TARGETING SET KEYWORD = :clob WHERE ADGROUP_ID = :id");

    OCI_BindLob(stmt, ":clob", clob);
    OCI_BindString(stmt, ":id", (char *)group_id, OCI_StringUTF8Length(group_id));
    OCI_LobWrite(clob, (char *)keyword, OCI_StringUTF8Length(keyword));

    OCI_Execute(stmt);
    OCI_Commit(cn);

    OCI_ConnectionFree(cn);

    return 0;
}

//推广->获取计划UNIQUE
int ad_system_sql_get_campaign_unique(ad_system_sql_t *p, const char *campaign_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select "
	    "CAMPAIGN_ID,"
	    "CAMPAIGN_NAME,"
	    "CAMPAIGN_STATE,"
	    "decode(PAUSE, 0, 1, 1, 2, 1) as PAUSE,"
	    "CAMPAIGN_BUDGET,"
	    "decode(SCHEDULE_TYPE, 0, 1, 1, 2, 1) as SCHEDULE_TYPE,"
	    "SCHEDULE_SERIALIZE,"
	    "to_char(sysdate, 'DYHH24') as SCHEDULE_SYS_TIME,"
	    "decode(REGION_TYPE, 0, 1, 1, 2, 1) as REGION_TYPE,"
	    "REGION_SERIALIZE,"
	    "to_char(BEGIN_TIME, 'yyyy-mm-dd HH24:MI:SS') as BEGIN_TIME,"
	    "to_char(END_TIME, 'yyyy-mm-dd HH24:MI:SS') as END_TIME,"
	    "decode(sign(trunc(sysdate) - trunc(begin_time)), 1, 1, 0, 1, 2) as sys_sub_begin_time,"
	    "decode(sign(trunc(sysdate) - trunc(end_time)), 1, 2, 0, 1, 1) as sys_sub_end_time "
	    "from T_CAMPAIGN "
	    "where campaign_id = %s",
	    campaign_id);

    return ad_system_sql_select(p, sql_str);
}


//推广->获取文字创意已删除
int ad_system_sql_get_creative_del_text(ad_system_sql_t *p,
	const char *account_id, 
	int find_type,
	const char *campaign_id, 
	const char *group_id, 
	int find_name_type, 
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, 
	    "select "
	    "nvl(campaign.campaign_id, del_campaign.campaign_id) as campaign_id,"
	    "nvl(campaign.campaign_name, del_campaign.campaign_name) as campaign_name,"
	    "nvl(adgroup.adgroup_id, del_adgroup.adgroup_id) as adgroup_id,"
	    "nvl(adgroup.adgroup_name, del_adgroup.adgroup_name) as adgroup_name,"
	    "creative.creative_id,"
	    "creative.title as creative_name,"
	    "decode(creative.pause, 0, 1, 1, 2, 1) as creative_pause,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.description1 as creative_description,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    "creative.append_id,"
	    "creative.mobile_destination_url,"
	    "creative.mobile_display_url "
	    "from t_deleted_text_creative creative "
	    "left join t_campaign campaign on creative.campaign_id = campaign.campaign_id "
	    "left join t_deleted_campaign del_campaign on creative.campaign_id = del_campaign.campaign_id "
	    "left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id "
	    "left join t_deleted_adgroup del_adgroup on creative.adgroup_id = del_adgroup.adgroup_id "
	    "where creative.account_id = %s ",
	account_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.TITLE='%s'", find_name);
	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.TITLE like '%%%s%%'", find_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.CAMPAIGN_ID='%s'", campaign_id);
    }

    if (find_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.ADGROUP_ID='%s'", group_id);
    }

    return ad_system_sql_select(p, sql_str);
}

//推广->获取图片创意(已删除)
int ad_system_sql_get_creative_del_img(ad_system_sql_t *p,
	const char *account_id,
	int find_type,
	const char *campaign_id, 
	const char *group_id, 
	int find_name_type,
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "nvl(campaign.campaign_id, del_campaign.campaign_id) as campaign_id,"
	    "nvl(campaign.campaign_name, del_campaign.campaign_name) as campaign_name,"
	    "nvl(adgroup.adgroup_id, del_adgroup.adgroup_id) as adgroup_id,"
	    "nvl(adgroup.adgroup_name, del_adgroup.adgroup_name) as adgroup_name,"

	    "creative.creative_id,"
	    "creative.creative_name,"
	    "decode(creative.pause, 0, 1, 1, 2, 1) as creative_pause,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.img_size,"
	    "creative.img_size_imglength,"
	    "creative.file_path,"
	    "creative.destination_url,"
	    "creative.display_url "
	    "from t_deleted_creative creative "

	    "left join t_campaign campaign on creative.campaign_id = campaign.campaign_id "
	    "left join t_deleted_campaign del_campaign on creative.campaign_id = del_campaign.campaign_id "
	    "left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id "
	    "left join t_deleted_adgroup del_adgroup on creative.adgroup_id = del_adgroup.adgroup_id "
	    "where creative.account_id = %s ",
	account_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name = '%s'", find_name);
	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name like '%%%s%%'", find_name);
    }

    if (find_type & 8) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.CAMPAIGN_ID='%s'", campaign_id);
    }

    if (find_type & 16) {

	sql_size += sprintf(&sql_str[sql_size], " and creative.ADGROUP_ID='%s'", group_id);
    }

    return ad_system_sql_select(p, sql_str);
}

//推广->组->关键词总数
int ad_system_sql_get_group_keyword_count(ad_system_sql_t *p, const char *account_id,
	int display_status,
	const char *campaign_id,
	const char *group_id,
	int find_type, 
	int find_name_type,
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, 
	    "select count(*) "
	    "from t_keyword keyword "
	    "left join t_adgroup adgroup on keyword.adgroup_id = adgroup.adgroup_id "
	    "where keyword.account_id=%s and adgroup.display_style=%u ",
	    account_id,
	    display_status);

    if (group_id && *group_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.adgroup_id='%s'", group_id);
    else if (campaign_id && *campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.campaign_id ='%s'", campaign_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword = '%s'", find_name);

	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword like '%%%s%%'", find_name);
    }

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

//推广->组->关键词
int ad_system_sql_get_group_keyword(ad_system_sql_t *p, const char *account_id,
	int page_index_s,
	int page_index_e,
	int display_status,
	const char *campaign_id,
	const char *group_id, 
	int find_type, 
	int find_name_type,
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str,
	    "select * from ("
	    "select rownum as page_index,"
	    "keyword.keyword_id,"
	    "keyword.keyword,"
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price "
	    "from t_keyword keyword "
	    "left join t_adgroup adgroup on keyword.adgroup_id = adgroup.adgroup_id "
	    "left join t_campaign campaign on keyword.campaign_id =campaign.campaign_id "
	    "where keyword.account_id=%s and adgroup.display_style=%u ",
	    account_id,
	    display_status);

    if (group_id && *group_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.adgroup_id='%s'", group_id);
    else if (campaign_id && *campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.campaign_id ='%s'", campaign_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword = '%s'", find_name);

	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword like '%%%s%%'", find_name);
    }

    sprintf(&sql_str[sql_size], ") t where t.page_index between %u and %u",
	    page_index_s,
	    page_index_e);

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

//推广->组->删除关键词
int ad_system_sql_del_group_keyword(const char  *keyword_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    char sql_str[4096] = {0};
    sprintf(sql_str, "insert into t_deleted_keyword select * from t_keyword t where t.keyword_id = %s", keyword_id);

    if (_ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    sprintf(sql_str, "delete from t_keyword t where t.keyword_id ='%s'", keyword_id);
    if (_ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->组->添加关键词
int ad_system_sql_add_group_keyword(ad_system_sql_t *sql_conn, const char *account_id, const char *group_id, const char  *keyword)
{

    char sql_str[4096];
    sprintf(sql_str, "insert into t_keyword values (seq_t_keyword.nextval, %s, (select campaign_id from t_adgroup where adgroup_id = '%s'), '%s', '%s', sysdate, sysdate, 1, 1)",
	    account_id,
	    group_id,
	    group_id,
	    keyword);

    if (_ad_system_sql_execute(sql_conn, sql_str))
	return -1;

    return 0;
}

int ad_system_sql_group_synch_state(const char *account_id, const char *group_id, const char *keyword_id)
{

    char sql_str[1024];
    if (group_id)
	sprintf(sql_str, "update t_adgroup set synch_state = 0 where adgroup_id = '%s'", group_id);
    else
	sprintf(sql_str, "update t_adgroup set synch_state = 0 where adgroup_id = (select adgroup_id from t_keyword where keyword_id=%s)", keyword_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->左侧列表
int ad_system_oci_get_sperad_list(ad_system_sql_t *p, char *account_id, int display_status)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "campaign.pause,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "adgroup.adgroup_state "
	    "from t_campaign campaign "
	    "left join t_adgroup adgroup on campaign.campaign_id = adgroup.campaign_id "
	    "where campaign.account_id=%s "
	    "and campaign.DISPLAY_STYLE=%u "
	    "order by NLSSORT(campaign.campaign_name, 'NLS_SORT=SCHINESE_PINYIN_M')",
	    account_id, display_status);

    return ad_system_sql_select(p, sql_str);
}


//推广->组->关键词报告
int ad_system_sql_get_group_keyword_report(ad_system_sql_t *p, const char *account_id,
	int display_status,
	const char *campaign_id,
	const char *group_id, 
	int find_type, 
	int find_name_type,
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str,
	    "select "
	    "keyword.keyword_id,"
	    "keyword.keyword,"
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "adgroup.cpc_price,"
	    "adgroup.cpm_price "
	    "from t_keyword keyword "
	    "left join t_adgroup adgroup on keyword.adgroup_id = adgroup.adgroup_id "
	    "left join t_campaign campaign on keyword.campaign_id =campaign.campaign_id "
	    "where keyword.account_id=%s and adgroup.display_style=%u ",
	    account_id,
	    display_status);

    if (group_id && *group_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.adgroup_id='%s'", group_id);
    else if (campaign_id && *campaign_id)
	sql_size += sprintf(&sql_str[sql_size], " and keyword.campaign_id ='%s'", campaign_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword = '%s'", find_name);

	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and keyword.keyword like '%%%s%%'", find_name);
    }

    return ad_system_sql_select(p, sql_str);
}

//批量修改组出价
int ad_system_sql_ad_system_batch_operation_group_price(const char *group_id, int price_type, const char *cpc_price, const char *cpm_price)
{

    char sql_str[4096] = {0};
    size_t size = sprintf(sql_str, "update t_adgroup t set ");

    size += sprintf(&sql_str[size], "t.bidding_model = (t.bidding_model + %d) - bitand(t.bidding_model, %d) ", price_type, price_type);

    if (price_type & 1)
	size += sprintf(&sql_str[size], ",t.cpc_price=%s ", cpc_price);
    if (price_type & 2)
	size += sprintf(&sql_str[size], ",t.cpm_price=%s ", cpm_price);

    sprintf(&sql_str[size], ",t.synch_state = 0 where t.adgroup_id = '%s'", group_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//复制创意检测
int ad_system_sql_copy_creative_check(
	const char *account_id,
	const char *campaign_id,
	const char *group_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select t.account_id from t_adgroup t where t.account_id = %s and t.campaign_id = '%s' and t.adgroup_id = '%s'",
	    account_id,
	    campaign_id,
	    group_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_free(&sql_conn);
    return 0;
}

//复制创意文字
int ad_system_sql_copy_creative_text_r(
	const char *creative_id,
	const char *account_id,
	const char *campaign_id,
	const char *group_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str,
	    "INSERT INTO T_TEXT_CREATIVE"
	    "(CREATIVE_ID,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "TITLE,"
	    "DESCRIPTION1,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "CREATIVE_STATE,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE,"
	    "APPEND_ID,"
	    "MOBILE_DESTINATION_URL,"
	    "MOBILE_DISPLAY_URL)"
	    "SELECT SEQ_T_TEXT_CREATIVE.NEXTVAL,"
	    "%s,"//ACCOUNT_ID
	    "'%s',"//CAMPAIGN_ID
	    "'%s',"//ADGROUP_ID
	    "TITLE,"
	    "DESCRIPTION1,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "34,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "0,  "
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE,"
	    "APPEND_ID,"
	    "MOBILE_DESTINATION_URL,"
	    "MOBILE_DISPLAY_URL "
	    "FROM T_TEXT_CREATIVE T "
	    "WHERE T.CREATIVE_ID = '%s'",

	account_id,
	campaign_id,
	group_id,
	creative_id);

    fprintf(stderr, "%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_copy_creative_img_r(
	const char *creative_id,
	const char *account_id,
	const char *campaign_id,
	const char *group_id)
{

    char sql_str[4096] = {0};
    sprintf(sql_str,
	    "INSERT INTO T_CREATIVE"
	    "(CREATIVE_ID,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "CREATIVE_NAME,"
	    "IMG_SIZE,"
	    "IMG_SIZE_IMGLENGTH,"
	    "FILE_PATH,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "CREATIVE_STATE,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE)"
	    "select SEQ_T_CREATIVE.nextval,"
	    "%s,"//ACCOUNT_ID,"
	    "%s,"//CAMPAIGN_ID,"
	    "%s,"//ADGROUP_ID,"
	    "CREATIVE_NAME,"
	    "IMG_SIZE,"
	    "IMG_SIZE_IMGLENGTH,"
	    "FILE_PATH,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "34, "
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "0,  "
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE "
	    "from T_CREATIVE src "
	    "where src.creative_id = '%s'",

	account_id,
	campaign_id,
	group_id,
	creative_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

//推广->获取图片创意
int ad_system_sql_get_img_creative(ad_system_sql_t *p,
	const char *account_id, 
	int find_type,
	int find_status,
	const char *campaign_id, 
	const char *group_id, 
	int find_name_type, 
	const char *find_name)
{

    char sql_str[4096] = {0};
    size_t sql_size = sprintf(sql_str, "select "
	    "campaign.campaign_id,"
	    "campaign.campaign_name,"
	    "adgroup.adgroup_id,"
	    "adgroup.adgroup_name,"
	    "creative.creative_id,"
	    "creative.creative_name,"
	    "decode(creative.pause, 0, 1, 1, 2, 1) as creative_pause,"
	    //"decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "decode(creative.creative_state, 31, 1, 32, 2, 33, 3, 34, 4, 35, 5, 1) as creative_state,"
	    "creative.refuse_reason,"
	    "creative.img_size,"
	    "creative.img_size_imglength,"
	    "creative.file_path,"
	    "creative.destination_url,"
	    "creative.display_url,"
	    //"dsp_baidu_creative.auditing_state,"
	    "nvl(dsp_baidu_creative.auditing_state, 3) as dsp_auditing_state,"
	    "dsp_baidu_creative.refuse_reason "
	    "from t_creative creative "
	    "left join t_adgroup adgroup on creative.adgroup_id = adgroup.adgroup_id "
	    "left join t_campaign campaign on creative.campaign_id = campaign.campaign_id "
	    "left join t_dsp_baidu_creative dsp_baidu_creative on creative.creative_id = dsp_baidu_creative.creative_id "
	    "where creative.account_id = %s ",
	account_id);

    if (find_type & 2) {

	if (find_name_type == 1)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name = '%s'", find_name);
	else if (find_name_type == 2)
	    sql_size += sprintf(&sql_str[sql_size], " and creative.creative_name like '%%%s%%'", find_name);
    }

    if (find_type & 8)
	sql_size += sprintf(&sql_str[sql_size], " and creative.CAMPAIGN_ID='%s'", campaign_id);

    if (find_type & 16)
	sql_size += sprintf(&sql_str[sql_size], " and creative.ADGROUP_ID='%s'", group_id);

    if (find_status == 10)
	sql_size += sprintf(&sql_str[sql_size], " and dsp_baidu_creative.auditing_state = 0");
    else if (find_status == 11)
	sql_size += sprintf(&sql_str[sql_size], " and dsp_baidu_creative.auditing_state = 1");
    else if (find_status == 12)
	sql_size += sprintf(&sql_str[sql_size], " and dsp_baidu_creative.auditing_state = 2");

    fprintf(stderr, "%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

//推广->复制图片创意
int ad_system_sql_copy_creative_img(
	int update_type,
	const char *creative_id,
	const char *creative_name,
	const char *creative_img_size,
	const char *creative_img_buffer_size,
	const char *creative_img_path,
	const char *creative_destination_url,
	const char *creative_display_url)
{

    char CREATIVE_NAME[1024] = "CREATIVE_NAME";
    char IMG_SIZE[128] = "IMG_SIZE";
    char IMG_SIZE_IMGLENGTH[128] = "IMG_SIZE_IMGLENGTH";
    char FILE_PATH[128] = "FILE_PATH";
    char DESTINATION_URL[1024] = "DESTINATION_URL";
    char DISPLAY_URL[1024] = "DISPLAY_URL";

    if (update_type & 16 && creative_name)
	sprintf(CREATIVE_NAME, "'%s'", creative_name);
    if (update_type & 16 && creative_img_size)
	sprintf(IMG_SIZE, "'%s'", creative_img_size);
    if (update_type & 16 && creative_img_buffer_size)
	sprintf(IMG_SIZE_IMGLENGTH, "'%s'", creative_img_buffer_size);
    if (update_type & 16 && creative_img_path)
	sprintf(FILE_PATH, "'%s'", creative_img_path);

    if (update_type & 2)
	sprintf(CREATIVE_NAME, "'%s'", creative_name);

    if (update_type & 4)
	sprintf(DESTINATION_URL, "'%s'", creative_destination_url);

    if (update_type & 8)
	sprintf(DISPLAY_URL, "'%s'", creative_display_url);

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_CREATIVE "
	    "(CREATIVE_ID,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "CREATIVE_NAME,"
	    "IMG_SIZE,"
	    "IMG_SIZE_IMGLENGTH,"
	    "FILE_PATH,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "CREATIVE_STATE,"
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "SYNCH_STATE,"
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE) "
	    "select SEQ_T_CREATIVE.nextval,"
	    "ACCOUNT_ID,"
	    "CAMPAIGN_ID,"
	    "ADGROUP_ID,"
	    "%s,"//CREATIVE_NAME
	    "%s,"//IMG_SIZE
	    "%s,"//IMG_SIZE_IMGLENGTH
	    "%s,"//FILE_PATH
	    "%s,"//DESTINATION_URL
	    "%s,"//DISPLAY_URL
	    "34,"//CREATIVE_STATE
	    "PAUSE,"
	    "CREATE_TIME,"
	    "LAST_UPDATE_TIME,"
	    "0,"//SYNCH_STATE
	    "SYNCN_INITIALIZED,"
	    "ROW_STATE "
	    "from T_CREATIVE src "
	    "where src.creative_id = '%s'",

	CREATIVE_NAME,
	IMG_SIZE,
	IMG_SIZE_IMGLENGTH,
	FILE_PATH,
	DESTINATION_URL,
	DISPLAY_URL,
	creative_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_brand_count(char *count,
	const char *account_id,
	int brand_status,
	int CREATIVE_STATUS,
	int DEPT_TYPE, 
	const char *DEPT_ARRAY, 
	int REGION_TYPE, 
	const char *REGION_ARRAY,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	int SORT_TYPE,
	int SORT_COLUMN,
	int page_index_s,
	int page_index_e)
{

    size_t size = 0;
    char dept_region_where_str[1024] = {0};
    if (DEPT_TYPE == 2)
	size += sprintf(&dept_region_where_str[size], " and dept.dept_code in (%s)", DEPT_ARRAY);
    if (REGION_TYPE == 2)
	size += sprintf(&dept_region_where_str[size], " and region.region_code in (%s)", REGION_ARRAY);

    size = 0;
    char where_str[1024] = {0};
    switch(brand_status) {

	case 1:
	    size += sprintf(&where_str[size], " and brand_status=1");
	    break;
	case 2:
	    size += sprintf(&where_str[size], " and brand_status=2");
	    break;
	case 3:
	    size += sprintf(&where_str[size], " and brand_status=3");
	    break;
	case 4:
	    size += sprintf(&where_str[size], " and (brand_status=4 or brand_status=5 or brand_status=6)");
	    break;
    }

    size += sprintf(&where_str[size], " and begin_time <= to_date('%s', 'YYYY-MM-DD')", SHOW_TIME_E);
    size += sprintf(&where_str[size], " and end_time >= to_date('%s', 'YYYY-MM-DD')", SHOW_TIME_S);

    char creative_state[128] = {0};
    switch(CREATIVE_STATUS) {

	case 1:
	    sprintf(creative_state, " and creative_state = 1");
	    break;
	case 2:
	    sprintf(creative_state, " and creative_state = 2");
	    break;
	case 3:
	    sprintf(creative_state, " and creative_state = 3");
	    break;
	case 4:
	    sprintf(creative_state, " and creative_state = 4");
	    break;
    }

    char sql_str[8192] = {0};
    sprintf(sql_str, 
	    "select count(*) from (select *"
	    "      from (select dept.brand_ad_id"
	    "          from t_brand_ad_dept dept"
	    "          cross join t_brand_ad_region region"
	    "          where 1 = 1 %s"
	    "          group by dept.brand_ad_id) t"
	    "      left join (select brand.brand_ad_id,"
	    "                       brand.ad_order_no,"
	    "                       brand.dept_code,"
	    "                       brand.region_type,"
	    "                       brand.region_serialize,"
	    "                       brand.end_time - brand.begin_time as days,"
	    "                       brand.begin_time,"
	    "                       brand.end_time,"
	    "                       brand.price as money,"
	    "                       to_char(brand.create_time, 'YYYY-HH-DD') as brand_create_time,"
	    "                         decode(brand.ad_state,"
	    "                                0,"
	    "                                decode(sign(TRUNC(sysdate) -"
	    "                                            brand.begin_time),"
	    "                                       -1,"
	    "                                       4,"// --申请中"
	    "                                       5),"// --已过期"
	    "                                1,"
	    "                                decode(sign(TRUNC(sysdate) -"
	    "                                            brand.begin_time),"
	    "                                       -1,"
	    "                                       3,"// --待投放"
	    "                                       decode(sign(TRUNC(sysdate) -"
	    "                                                   brand.end_time),"
	    "                                              1,"
	    "                                              2,"// --已结束"
	    "                                              1)),"// --投放中"
	    "                                6) as brand_status,"// --申请拒绝"
	    "                       brand.refuse_reason as brand_refuse_reason"
	    "                  from t_brand_ad brand where brand.account_id = %s and brand.row_state = 1) tt on t.brand_ad_id = tt.brand_ad_id"
	    "     left join t_brand_creative_temp creative on t.brand_ad_id = creative.brand_ad_id"
	    "     where 1 = 1 %s)"
	    " where 1=1 %s",

	dept_region_where_str,
	account_id,
	where_str,
	creative_state);

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




int ad_system_sql_get_brand_id(char *brand_id)
{

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, "select SEQ_T_BRAND_AD.Nextval from dual"))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(brand_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_brand_dept(ad_system_sql_t *p,
	const char *brand_id,
	const char *DEPT_CODE)
{

    char sql_str[1024];

    char buffer[128];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(DEPT_CODE, buffer, &save_ptr, ',');
    if (!str) return -1;

    while(str) {

	sprintf(sql_str, 
		"insert into t_brand_ad_dept t (BRAND_AD_DEPT_ID, BRAND_AD_ID, DEPT_ID, DEPT_CODE) values (seq_t_brand_ad_dept.nextval, %s, %s, '%s')",
		brand_id, str, str);
	fprintf(stderr, "-->%s\n", sql_str);
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

    char sql_str[1024];

    char buffer[128];
    char *save_ptr = NULL;
    char *str = lg_string_strtok(REGION_SERIALIZE, buffer, &save_ptr, ',');
    if (!str) return -1;

    while(str) {

	sprintf(sql_str, 
		"insert into T_BRAND_AD_REGION (BRAND_AD_REGION_ID, BRAND_AD_ID, REGION_ID, REGION_CODE) values (seq_t_brand_ad_region.nextval, %s, %s, '%s')",
		brand_id, str, str);
	fprintf(stderr, "-->%s\n", sql_str);
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

    char REGION_SERIALIZE_STR[4096] = "null";
    if (REGION_TYPE == 2)
	sprintf(REGION_SERIALIZE_STR, "'%s'", REGION_SERIALIZE);

    char sql_str[8192] = {0};
    sprintf(sql_str, "insert into T_BRAND_AD"
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
	    "%s,"//REGION_SERIALIZE
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

    fprintf(stderr, "-->%s\n", sql_str);
    if (_ad_system_sql_execute(p, sql_str))
	return -1;
    return 0;
}


int ad_system_sql_add_brand(
	const char *ACCOUNT_ID,
	int REGION_TYPE,
	const char *REGION_SERIALIZE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	const char *DEPT_CODE,
	const char *PRICE,
	const char *PROPOSER_ID)
{

    char brand_id[128] = {0};
    if (ad_system_sql_get_brand_id(brand_id))
	return -1;

    ad_system_sql_t sql_conn;
    if (ad_system_sql_connect(&sql_conn))
	return -1;

    if (ad_system_sql_add_brand_dept(&sql_conn, brand_id, DEPT_CODE)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (ad_system_sql_add_brand_region(&sql_conn, brand_id, REGION_TYPE, REGION_SERIALIZE)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    if (ad_system_sql_add_brand_r(&sql_conn,
		brand_id,
		ACCOUNT_ID,
		REGION_TYPE,
		REGION_SERIALIZE,
		SHOW_TIME_S,
		SHOW_TIME_E,
		DEPT_CODE,
		PRICE,
		PROPOSER_ID)) {

	ad_system_sql_rollback(&sql_conn);
	ad_system_sql_free(&sql_conn);
	return -1;
    }

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_brand_creative(ad_system_sql_t *p, int MOD_STATUS, const char *brand_id)
{

    char sql_str[4096] = {0};

    if (MOD_STATUS == 1) {
	sprintf(sql_str, 
		"select t.brand_ad_id, "
		"t.brand_creative_id, "
		"t.title, "
		"t.description, "
		"t.destination_url, "
		"t.display_url, "
		"t.telephone, "
		"t.web_site, "
		"t.web_im,"//"decode(t.web_im, null, account.webim, t.web_im),"
		"t.guahao_url,"//"decode(t.guahao_url, null, account.guahao_token, t.guahao_url),"
		"t.creative_state, "
		"t.refuse_reason, "
		"t.create_time, "
		"t.last_update_time, "
		"decode(creative.brand_ad_id, null, 1, 2) as INSPECTION_STATUS, "
		"append.sequence, "
		"append.img_path, "
		"append.url "
		"from t_brand_creative_temp t "
		"left join t_brand_creative_append_temp append on t.brand_creative_id = "
		"append.brand_creative_id "
		"left join t_brand_creative creative on t.brand_ad_id = creative.brand_ad_id "
		"left join t_account account on t.account_id = account.account_id "
		"where t.brand_ad_id = %s",
	    brand_id);
    } else {

	sprintf(sql_str, 
		"select t.brand_ad_id,"
		"t.brand_creative_id,"
		"t.title,"
		"t.description,"
		"t.destination_url,"
		"t.display_url,"
		"t.telephone,"
		"t.web_site,"
		"t.web_im,"//"decode(t.web_im, null, account.webim, t.web_im),"
		"t.guahao_url,"//"decode(t.guahao_url, null, account.guahao_token, t.guahao_url),"
		"t.creative_state,"
		"t.refuse_reason,"
		"t.create_time,"
		"t.last_update_time,"
		"2,"
		"append.sequence,"
		"append.img_path,"
		"append.url "
		"from t_brand_creative t "
		"left join t_brand_creative_append append on t.brand_creative_id=append.brand_creative_id "
		"left join t_account account on t.account_id = account.account_id "
		"where t.brand_ad_id = %s",
	    brand_id);
    }

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_add_brand_creative(ad_system_sql_t *p,
	int insert_type,
	const char *BRAND_ID,
	const char *ACCOUNT_ID,
	const char *TITLE,
	const char *DESCRIPTION,
	const char *DESTINATION_URL,
	const char *DISPLAY_URL,
	const char *TELEPHONE,
	const char *WEB_SITE,
	const char *WEB_IM,
	const char *GUAHAO_URL)
{

    char WEB_IM_STR[1024] = "null";
    char GUAHAO_URL_STR[1024] = "null";

    if (insert_type & 1)
	sprintf(WEB_IM_STR, "'%s'", WEB_IM);
    if (insert_type & 2)
	sprintf(GUAHAO_URL_STR, "'%s'", GUAHAO_URL);

    char sql_str[40960];
    sprintf(sql_str,
	    "insert into T_BRAND_CREATIVE_TEMP"
	    "(BRAND_CREATIVE_ID,"
	    "BRAND_AD_ID,"
	    "ACCOUNT_ID,"
	    "TITLE,"
	    "DESCRIPTION,"
	    "DESTINATION_URL,"
	    "DISPLAY_URL,"
	    "TELEPHONE,"
	    "WEB_SITE,"
	    "WEB_IM,"
	    "CREATIVE_STATE,"
	    "GUAHAO_URL"
	    ")values("
	    "seq_t_brand_creative.nextval,"
	    "%s,"//BRAND_AD_ID
	    "%s,"//ACCOUNT_ID
	    "'%s',"//TITLE
	    "'%s',"//DESCRIPTION
	    "'%s',"//DESTINATION_URL
	    "'DISPLAY_URL',"//DISPLAY_URL
	    "'%s',"//TELEPHONE
	    "'%s',"//WEB_SITE
	    "%s,"//WEB_IM
	    "0,"//CREATIVE_STATE
	    "%s)",//GUAHAO_URL

	BRAND_ID,
	ACCOUNT_ID,
	TITLE,
	DESCRIPTION,
	DESTINATION_URL,
	//DISPLAY_URL,
	TELEPHONE,
	WEB_SITE,
	WEB_IM_STR,
	GUAHAO_URL_STR);

    fprintf(stderr, "%s\n", sql_str);
    if (_ad_system_sql_execute(p, sql_str))
	return -1; 

    return 0;
}

int ad_system_sql_set_brand_creative(int update_type,
	const char *BRAND_CREATIVE_ID,
	const char *ACCOUNT_ID,
	const char *TITLE,
	const char *DESCRIPTION,
	const char *DESTINATION_URL,
	const char *DISPLAY_URL,
	const char *TELEPHONE,
	const char *WEB_SITE,
	const char *WEB_IM,
	const char *GUAHAO_URL)
{

    size_t size = 0;
    char sql_str[4096] = {0};
    size += sprintf(&sql_str[size], "update T_BRAND_CREATIVE_TEMP t set ");

    char comma[4] = "";

    if (update_type & 1) {
	size += sprintf(&sql_str[size], "%st.TITLE = '%s'", comma, TITLE);
	comma[0] = ',';
    }

    if (update_type & 2) {
	size += sprintf(&sql_str[size], "%st.DESCRIPTION= '%s'", comma, DESCRIPTION);
	comma[0] = ',';
    }

    if (update_type & 4) {
	size += sprintf(&sql_str[size], "%st.DESTINATION_URL= '%s'", comma, DESTINATION_URL);
	comma[0] = ',';
    }

    if (update_type & 8) {
	size += sprintf(&sql_str[size], "%st.DISPLAY_URL= '%s'", comma, DISPLAY_URL);
	comma[0] = ',';
    }

    if (update_type & 16) {
	size += sprintf(&sql_str[size], "%st.TELEPHONE= '%s'", comma, TELEPHONE);
	comma[0] = ',';
    }

    if (update_type & 32) {
	size += sprintf(&sql_str[size], "%st.WEB_SITE= '%s'", comma, WEB_SITE);
	comma[0] = ',';
    }

    if (update_type & 64) {
	size += sprintf(&sql_str[size], "%st.WEB_IM= '%s'", comma, WEB_IM);
	comma[0] = ',';
    }

    if (update_type & 128) {
	size += sprintf(&sql_str[size], "%st.GUAHAO_URL= '%s'", comma, GUAHAO_URL);
	comma[0] = ',';
    }

    sprintf(&sql_str[size], ",t.CREATIVE_STATE = 0 where t.BRAND_CREATIVE_ID=%s", BRAND_CREATIVE_ID);

    fprintf(stderr, "%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_add_brand_creative_append(ad_system_sql_t *p,
	const char *brand_creative_id,
	const char *img_path, 
	const char *img_size, 
	const char *img_url, 
	int img_sequence)
{

    char sql_str[4096] = {0};
    sprintf(sql_str, 
	    "insert into t_brand_creative_append_temp"
	    "(BRAND_CREATIVE_APPEND_ID,"
	    "BRAND_CREATIVE_ID,"
	    "IMG_PATH,"
	    "IMG_SIZE,"
	    "URL,"
	    "SEQUENCE)"
	    "values"
	    "(seq_t_brand_creative_append.nextval,"
	    "(select t.brand_creative_id from t_brand_creative_temp t where t.brand_ad_id = %s),"
	    "'%s',"//IMG_PATH,
	    "'%s',"//IMG_SIZE,
	    "'%s',"//URL,
	    "%d)",//SEQUENCE

	    brand_creative_id,
	    img_path,
	    img_size,
	    img_url,
	    img_sequence);

    fprintf(stderr, "%s\n", sql_str);
    if (_ad_system_sql_execute(p, sql_str))
	return -1; 

    return 0;
}

int ad_system_sql_set_brand_creative_state(const char *brand_creative_id)
{

    char sql_str[1024];
    sprintf(sql_str, "update T_BRAND_CREATIVE_TEMP t set t.CREATIVE_STATE = 0 where t.BRAND_CREATIVE_ID=%s", brand_creative_id);

    fprintf(stdout, "%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_set_brand_creative_append(int status, const char *brand_creative_id, const char *img_path, const char *img_size, const char *img_url, int img_sequence)
{

    char sql_str[4096] = {0};
    if (status == 0) {

	sprintf(sql_str,
		"update t_brand_creative_append_temp t "
		"set t.url = '%s' "
		"where t.sequence = %d"
		"and t.brand_creative_id = %s",
		img_url, img_sequence, brand_creative_id);

    } else if (status == 1) {

	sprintf(sql_str,
		"update t_brand_creative_append_temp t "
		"set t.img_path = '%s', t.img_size = '%s' "
		"where t.sequence = %d"
		"and t.brand_creative_id = %s",
		img_path, img_size, img_sequence, brand_creative_id);
    }

    fprintf(stdout, "%s\n", sql_str);
    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);

    return ad_system_sql_set_brand_creative_state(brand_creative_id);
}

int ad_system_sql_del_brand(const char *brand_id_list)
{

    char sql_str[4096];
    sprintf(sql_str, "update t_brand_ad t set t.row_state = 0 where t.brand_ad_id in (%s)", brand_id_list);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1; 

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_account_domain(const char *account_id, char *domain)
{

    char sql_str[40960] = {0};
    sprintf(sql_str, "select domain from t_account t where account_id = %s", account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(domain, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_brand(ad_system_sql_t *p,
	const char *account_id,
	int brand_status,
	int CREATIVE_STATUS,
	int DEPT_TYPE, 
	const char *DEPT_ARRAY, 
	int REGION_TYPE, 
	const char *REGION_ARRAY,
	int SHOW_TIME_TYPE,
	const char *SHOW_TIME_S,
	const char *SHOW_TIME_E,
	int SORT_TYPE,
	int SORT_COLUMN,
	int page_index_s,
	int page_index_e)
{

    //地域与病种条件
    char dept_region_str[4096] = {0};
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


    size_t size = 0;
    char where_str[1024] = {0};
    switch(brand_status) {

	case 1:
	    size += sprintf(&where_str[size], " and brand_status=1");
	    break;
	case 2:
	    size += sprintf(&where_str[size], " and brand_status=2");
	    break;
	case 3:
	    size += sprintf(&where_str[size], " and brand_status=3");
	    break;
	case 4:
	    size += sprintf(&where_str[size], " and (brand_status=4 or brand_status=5 or brand_status=6)");
	    break;
    }

    if (SHOW_TIME_TYPE == 2) {
	size += sprintf(&where_str[size], " and begin_time <= to_date('%s', 'YYYY-MM-DD')", SHOW_TIME_E);
	size += sprintf(&where_str[size], " and end_time >= to_date('%s', 'YYYY-MM-DD')", SHOW_TIME_S);
    }

    char creative_state[128] = {0};
    switch(CREATIVE_STATUS) {

	case 1:
	    sprintf(creative_state, " and creative_state = 1");
	    break;
	case 2:
	    sprintf(creative_state, " and creative_state = 2");
	    break;
	case 3:
	    sprintf(creative_state, " and creative_state = 3");
	    break;
	case 4:
	    sprintf(creative_state, " and creative_state = 4");
	    break;
    }

    //1订单编号 2投放科室 3投放地域 4投放时长 5开始日期 6结束日期 7广告消费 8创建时间 9创意状态 10订单状态
    size = 0;
    char sort_str[512] = {0};
    switch(SORT_COLUMN){

	case 1:
	    size += sprintf(&sort_str[size], "ad_order_no");
	    break;
	case 2:
	    size += sprintf(&sort_str[size], "dept_code");
	    break;
	case 3:
	    size += sprintf(&sort_str[size], "region_serialize");
	    break;
	case 4:
	    size += sprintf(&sort_str[size], "days");
	    break;
	case 5:
	    size += sprintf(&sort_str[size], "begin_time");
	    break;
	case 6:
	    size += sprintf(&sort_str[size], "end_time");
	    break;
	case 7:
	    size += sprintf(&sort_str[size], "money");
	    break;
	case 8:
	    size += sprintf(&sort_str[size], "brand_create_time");
	    break;
	case 9:
	    size += sprintf(&sort_str[size], "creative_state");
	    break;
	case 10:
	    size += sprintf(&sort_str[size], "brand_status");
	    break;
    }

    if (SORT_TYPE == 2)
	sprintf(&sort_str[size], " desc");

    char sql_str[8192] = {0};
    sprintf(sql_str, 
	    "select *"
	    "from (select brand_ad_id,"
	    "     ad_order_no,"
	    "     dept_code,"
	    "     region_type,"
	    "     region_serialize,"
	    "     days,"
	    "     begin_time,"
	    "     end_time,"
	    "     money,"
	    "     brand_create_time,"
	    "     brand_status,"
	    "     brand_refuse_reason,"
	    "     creative_state,"
	    "     creative_refuse_reason,"
	    "     inspection_status,"
	    "     rownum as page_index"
	    "   from (select t.brand_ad_id,"
	    "        ad_order_no,"
	    "        dept_code,"
	    "        region_type,"
	    "        region_serialize,"
	    "        days,"
	    "        begin_time,"
	    "        end_time,"
	    "        money,"
	    "        brand_create_time,"
	    "        brand_status,"
	    "        brand_refuse_reason,"
	    "        decode(creative.creative_state, 0, 1, 1, 2, 2, 3, 4) as creative_state,"
	    "        creative.refuse_reason as creative_refuse_reason,"
	    "        decode(creative_r.brand_ad_id, null, 1, 2) as inspection_status"
	    "      from (%s select brand.brand_ad_id,"
	    "                       brand.ad_order_no,"
	    "                       brand.dept_code,"
	    "                       brand.region_type,"
	    "                       brand.region_serialize,"
	    "                       brand.end_time - brand.begin_time + 1 as days,"
	    "                       brand.begin_time,"
	    "                       brand.end_time,"
	    "                       brand.price as money,"
	    "                       to_char(brand.create_time, 'YYYY-MM-DD') as brand_create_time,"
	    "                         decode(brand.ad_state,"
	    "                                0,"
	    "                                decode(sign(TRUNC(sysdate) -"
	    "                                            brand.begin_time),"
	    "                                       -1,"
	    "                                       4,"// --申请中"
	    "                                       5),"// --已过期"
	    "                                1,"
	    "                                decode(sign(TRUNC(sysdate) -"
	    "                                            brand.begin_time),"
	    "                                       -1,"
	    "                                       3,"// --待投放"
	    "                                       decode(sign(TRUNC(sysdate) -"
	    "                                                   brand.end_time),"
	    "                                              1,"
	    "                                              2,"// --已结束"
	    "                                              1)),"// --投放中"
	    "                                6) as brand_status,"// --申请拒绝"
	    "                       brand.refuse_reason as brand_refuse_reason"
	    "                  from t_brand_ad brand where brand.account_id = %s and brand.row_state = 1) %s"
	    "     left join t_brand_creative_temp creative on t.brand_ad_id = creative.brand_ad_id"
	    "     left join t_brand_creative creative_r on t.brand_ad_id = creative_r.brand_ad_id"
	    "     where 1 = 1 %s"
	    "     order by %s)"
	    " where 1=1 %s)"
	    " where page_index between %u and %u",

	dept_region_str,
	account_id,
	*dept_region_str ? " tt on t.brand_ad_id=tt.brand_ad_id" : " t",
	where_str,
	sort_str,
	creative_state,
	page_index_s,
	page_index_e
	    );

    fprintf(stderr, "-->%s\n", sql_str);
    return ad_system_sql_select(p, sql_str);
}

int ad_system_sql_get_brand_creative_id(char *brand_creative_id)
{

    char sql_str[1024] = {0};
    sprintf(sql_str, "select seq_t_brand_creative.nextval from dual");

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    strcpy(brand_creative_id, ad_system_sql_get_string(&sql_conn, NULL, 1));
    ad_system_sql_free(&sql_conn);
    return 0;
}

int ad_system_sql_get_disease_code_type(char *account_id, int *disease_code_type)
{

    char sql_str[1024];
    sprintf(sql_str, "select disease_code_type from t_account where account_id=%s", account_id);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_select(&sql_conn, sql_str))
	return -1;

    if (!ad_system_sql_fetch_next(&sql_conn)) {

	ad_system_sql_free(&sql_conn);
	return -1;
    }

    *disease_code_type = ad_system_sql_get_number(&sql_conn, NULL, 1);
    ad_system_sql_free(&sql_conn);
    return 0;
}













//推广->添加计划
int ad_system_sql_add_campaign(
	const char *account_id,
	int display_status,
	const char *campaign_id,
	const char *campaign_name,
	const char *dudget,
	int region_type,
	const char *region_serialize,
	int schedule_type,
	const char *schedule_serialize,
	int time_type,
	const char *begin_time,
	const char *end_time)
{

    char begin_time_sql[256] = "null";
    sprintf(begin_time_sql, "to_date('%s', 'YYYY-MM-DD')", begin_time);

    char end_time_sql[256] = "null";
    if (time_type == 2)
	sprintf(end_time_sql, "to_date('%s', 'YYYY-MM-DD')", end_time);


    char region_serialize_sql[1024] = "null";
    if (region_type == 1)
	region_type = 0;
    else {

	region_type = 1;
	sprintf(region_serialize_sql, "'%s'", region_serialize);
    }

    char schedule_serialize_sql[1024] = "null";
    if (schedule_type == 1)
	schedule_type = 0;
    else {

	schedule_type = 1;
	sprintf(schedule_serialize_sql, "'%s'", schedule_serialize);
    }

    char sql_str[4096] = {0};
    sprintf(sql_str, "INSERT INTO T_CAMPAIGN (CAMPAIGN_ID,ACCOUNT_ID,CAMPAIGN_NAME,CAMPAIGN_BUDGET,REGION_TYPE,REGION_SERIALIZE,BEGIN_TIME,END_TIME,SCHEDULE_TYPE,SCHEDULE_SERIALIZE,CAMPAIGN_STATE,PAUSE,CREATE_TIME,LAST_UPDATE_TIME,SYNCH_STATE,SYNCN_INITIALIZED,ROW_STATE,BIDDING_MODEL,DISPLAY_STYLE) VALUES ('%s', %s, '%s', %s, %u, %s, %s, %s, %u, %s, 11, 0, sysdate, sysdate, 0, 1, 1, 1,%u)",
	    campaign_id,
	    account_id,
	    campaign_name,
	    dudget,
	    region_type,
	    region_serialize_sql,
	    begin_time_sql,
	    end_time_sql,
	    schedule_type,
	    schedule_serialize_sql,
	    display_status);

    ad_system_sql_t sql_conn;
    if (ad_system_sql_execute(&sql_conn, sql_str))
	return -1;

    ad_system_sql_commit(&sql_conn);
    ad_system_sql_free(&sql_conn);
    return 0;
}

