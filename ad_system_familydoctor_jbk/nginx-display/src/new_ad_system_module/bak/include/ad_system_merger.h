
#ifndef	__AD_SYSTEM_MERGER_H__
#define	__AD_SYSTEM_MERGER_H__

#include "lg_list.h"
#include "ad_system_sql.h"
#include "ad_system_json_packet.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	lg_list_t list;

	char CAMPAIGN_ID[128];
	char CAMPAIGN_NAME[256];

	int CAMPAIGN_STATE;
	double CAMPAIGN_BUDGET;

	int SCHEDULE_TYPE;
	char SCHEDULE_SERIALIZE[2048];

	int REGION_TYPE;
	char REGION_SERIALIZE[1024];

	int TIME_TYPE;
	char BEGIN_TIME[128];
	char END_TIME [128];

	int CAMPAIGN_IMPRESSION;//展现
	int CAMPAIGN_CLICK;//点击
	double CAMPAIGN_COST;//消费  
	double CAMPAIGN_CTR;//点击率
	double CAMPAIGN_CPC;//平均点击价格
	double CAMPAIGN_CPM;//千次展现成本

    } ad_system_merger_campaign_t; //计划

    typedef struct {

	lg_list_t list;

	char CAMPAIGN_ID[128];
	char CAMPAIGN_NAME[256];

	char GROUP_ID[128];
	char GROUP_NAME[256];

	int GROUP_STATE;

	int BIDDING_MODEL;//出价状态
	double GROUP_CPC_PRICE;//CPC出价
	double GROUP_CPM_PRICE;//CPM出价

	int GROUP_TARGETING_TYPE; //1浏览定向 2搜索定向
	char GROUP_DISEASE[1024];

	int GROUP_IMPRESSION;//展现次数
	int GROUP_CLICK;//点击次数
	double GROUP_COST;//总消费  
	double GROUP_CTR;//点击率
	double GROUP_CPC;//平均点击价格
	double GROUP_CPM;//千次展现成本

    } ad_system_merger_group_t; //组

    typedef struct {

	lg_list_t list;

	char CAMPAIGN_ID[128];
	char CAMPAIGN_NAME[256];

	char GROUP_ID[128];
	char GROUP_NAME[256];

	char CREATIVE_ID[128];
	char CREATIVE_NAME[256];

	int CREATIVE_STATE;//1有效 2不宜推广 3暂停 4审核中 5审核拒绝
	char CREATIVE_REFUSE_REASON[256];//审核拒绝说明
	
	int CREATIVE_DSP_STATE;//0有效 1审核中 2审核拒绝
	char CREATIVE_DSP_REFUSE_REASON[256];//审核拒绝说明

	char CREATIVE_IMG_SIZE[128];
	char CREATIVE_IMG_SIZE_IMGLENGTH[128];
	char CREATIVE_FILE_PATH[256];

	char CREATIVE_DESTINATION_URL[1024];
	char CREATIVE_DISPLAY_URL[1024];

	int CREATIVE_IMPRESSION;//展现次数
	int CREATIVE_CLICK;//点击次数
	double CREATIVE_COST;//总消费  
	double CREATIVE_CTR;//点击率
	double CREATIVE_CPC;//平均点击价格
	double CREATIVE_CPM;//千次展现成本

    } ad_system_merger_creative_img_t; //图片创意

    typedef struct {

	lg_list_t list;

	char CAMPAIGN_ID[128];
	char CAMPAIGN_NAME[256];

	char GROUP_ID[128];
	char GROUP_NAME[256];

	char CREATIVE_ID[128];
	char CREATIVE_NAME[256];

	int CREATIVE_STATE;//1有效 2不宜推广 3暂停 4审核中 5审核拒绝
	char CREATIVE_REFUSE_REASON[1024];

	char CREATIVE_DESCRIPTION[1024];
	char CREATIVE_DESTINATION_URL[1024];
	char CREATIVE_DISPLAY_URL[1024];
	int CREATIVE_APPEND_TYPE;
	char CREATIVE_APPEND_ID[128];

	char MOBILE_DESTINATION_URL[1024];
	char MOBILE_DISPLAY_URL[1024];

	int CREATIVE_IMPRESSION;//展现次数
	int CREATIVE_CLICK;//点击次数
	double CREATIVE_COST;//总消费  
	double CREATIVE_CTR;//点击率
	double CREATIVE_CPC;//平均点击价格
	double CREATIVE_CPM;//千次展现成本

    } ad_system_merger_creative_text_t; //文字创意

    typedef struct {

	int CAMPAIGN_ENABLED;
	double CAMPAIGN_BUDGET;
	int CAMPAIGN_IMPRESSION;
	int CAMPAIGN_CLICK;
	double CAMPAIGN_CTR;
	double CAMPAIGN_CPC;
	double CAMPAIGN_COST;
	double CAMPAIGN_CPM;
	int CAMPAIGN_COUNT;

    } ad_system_merger_campaign_info_t; //计划汇总

    typedef struct {

	int GROUP_ENABLED;
	double GROUP_PRICE;
	int GROUP_IMPRESSION;
	int GROUP_CLICK;
	double GROUP_CTR;
	double GROUP_CPC;
	double GROUP_COST;
	double GROUP_CPM;
	int GROUP_COUNT;

    } ad_system_merger_group_info_t;//组汇总

    typedef struct {

	int CREATIVE_ENABLED;
	int CREATIVE_IMPRESSION;
	int CREATIVE_CLICK;
	double CREATIVE_CTR;
	double CREATIVE_CPC;
	double CREATIVE_COST;
	double CREATIVE_CPM;
	int CREATIVE_COUNT;

    } ad_system_merger_creative_info_t;//创意汇总

    extern void ad_system_merger_free(lg_list_t *head);

    extern int ad_system_merger_campaign(
	    ad_system_sql_t *sql_conn, 
	    lg_list_t *mongo_head, 
	    ad_system_packet_json_data_t *json,
	    lg_list_t *manager_head,
	    ad_system_merger_campaign_info_t *info);

    extern int ad_system_merger_group(
	    ad_system_sql_t *sql_conn, 
	    lg_list_t *mongo_head,
	    ad_system_packet_json_data_t *json,
	    lg_list_t *manager_head,
	    ad_system_merger_group_info_t *info);

    extern int ad_system_merger_creative_text(
	    ad_system_sql_t *sql_conn, 
	    lg_list_t *mongo_head, 
	    ad_system_packet_json_data_t *json,
	    lg_list_t *merger_head,
	    ad_system_merger_creative_info_t *info);

    extern int ad_system_merger_creative_img(
	    ad_system_sql_t *sql_conn, 
	    lg_list_t *mongo_head,
	    ad_system_packet_json_data_t *json,
	    lg_list_t *merger_head,
	    ad_system_merger_creative_info_t *info);

    extern int ad_system_merger_schedule(const char *sysdate, const char *schedule);

#ifdef __cplusplus
}
#endif

#endif


