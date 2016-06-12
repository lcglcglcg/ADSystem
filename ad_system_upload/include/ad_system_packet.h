
#ifndef	__AD_SYSTEM_PACKET_H__
#define	__AD_SYSTEM_PACKET_H__

#define	PACKET_CODE_OK					0//成功
#define	PACKET_CODE_USER_LOGIN_CHECK_OK			1000//用户已登录
#define	PACKET_CODE_USER_LOGIN_CHECK_ERR		1001//用户未登录
#define	PACKET_CODE_USER_LOGIN_ERR			1002//用户名/密码错误
#define PACKET_CODE_USER_LOGIN_IMG_ERR			1003//验证码错误
#define	PACKET_CODE_USER_PASS_PROTECT_AUTHEN_ERR	1012//验证密保错误
#define	PACKET_CODE_NAME_REPEAT				1013//名字重复
#define	PACKET_CODE_IMG_UPLOAD_FILE_ERR			1014//上传文件格式错误
#define	PACKET_CODE_IMG_UPLOAD_FILE_SIZE_ERR		1015//上传文件尺寸错误
#define	PACKET_CODE_NOT_CAMPAIGN_ID			1016//计划ID不存在
#define	PACKET_CODE_NOT_GROUT_ID			1017//组ID不存在
#define	PACKET_CODE_IMG_ID_ERR				1018//图片ID 错误
#define	PACKET_CODE_PIRCE_ERR				1019//至少需要一个出价
#define	PACKET_CODE_USER_FROZEN				1020//账户冻结
#define	PACKET_CODE_PRICE_EXPENSIVE			1021//出价大于预算

#define PACKET_CODE_NOT_KEY                             9995//没有这个ID
#define	PACKET_CODE_NOT_DATA				9996//没有数据
#define	PACKET_CODE_PARAMETER_ERR			9997//报文参数错误
#define	PACKET_CODE_FORMAT_ERR				9998//报文格式错误
#define	PACKET_CODE_ERR					9999//服务器内部错误

#define	HTTP_HEAD "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\n\r\n"
#define	HTTP_JSON_HEAD "HTTP/1.1 200 OK\r\n\r\n{"
#define	HTTP_IMAGE "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: %u\r\nContent-Type: image/png\r\nSet-Cookie: key=%s; Path=/action\r\n\r\n"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	const char *PASS_OLD;
	const char *PASS_NEW;

	int FIND_TYPE;
	char FIND_NAME[256];
	const char *_FIND_NAME;

	int PAGE_COUNT;
	int PAGE_INDEX;
	int page_index_s;
	int page_index_e;

	int DATE_TYPE;
	int DISPLAY_STATUS;

	int SORT_COLUMN;
	int SORT_TYPE;

	int FIND_NAME_TYPE;
	int FIND_STATUS;
	int UPDATE_TYPE;
	int STATUS;

	int TIME_TYPE;
	const char *BEGIN_TIME;
	const char *END_TIME;

	const char *CAMPAIGN_ID;
	const char *CAMPAIGN_NAME;
	const char *CAMPAIGN_BUDGET;
	int CAMPAIGN_STATUS;

	int REGION_TYPE;
	const char *REGION_SERIALIZE;
	int SCHEDULE_TYPE;
	const char *SCHEDULE_SERIALIZE;

	const char *GROUP_ID;
	int GROUP_NAME_TYPE;
	const char *GROUP_NAME;
	int GROUP_PRICE_TYPE;
	const char *GROUP_CPC_PRICE;
	const char *GROUP_CPM_PRICE;
	int GROUP_TARGETING_TYPE;
	const char *GROUP_DISEASE;
	const char *GROUP_KEYWORD;

	const char *CREATIVE_ID;
	const char *CREATIVE_NAME;
	int CREATIVE_STATUS;

	const char *CREATIVE_DESCRIPTION;
	const char *MOBILE_DESTINATION_URL;
	const char *MOBILE_DISPLAY_URL;


	const char *CREATIVE_DESTINATION_URL;
	const char *CREATIVE_DISPLAY_URL;
	const char *CREATIVE_IMG_ID;

	const char *CREATIVE_APPEND_ID;
	int APPEND_STATUS;
	const char *APPEND_CONTENT;
	int CREATIVE_APPEND_TYPE;

	const char *ID;
	int ID_TYPE;
	int BATCH_TYPE;

	const char *REPORT_ID;
	int REPORT_SPECIES;
	int REPORT_TYPE;
	const char *REPORT_NAME;
	const char *TIME_START;
	const char *TIME_END;
	int REPORT_BELONG;

	int MONEY_TYPE;

	const char *COMPANY_NAME;
	const char *COMPANY_SITE;
	const char *INDUSTRY;
	const char *PROVINCE_ID;
	const char *CITY_ID;
	const char *ADDRESS;
	const char *POSTCODE;
	const char *CONTACT;
	const char *TELEPHONE;
	const char *FAX;
	const char *MOBILE;
	const char *EMAIL;
	const char *WEBIM;
	
	const char *MEDIA_TYPE;
	const char *KEY;

    } ad_system_packet_json_data_t;

#ifdef __cplusplus
}
#endif

#endif



