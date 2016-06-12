
#ifndef	__AD_SYSTEM_PACKET_H__
#define	__AD_SYSTEM_PACKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ngx_http.h>

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
#define	PACKET_CODE_BUDGET_ERR				1022//预算过低
#define	PACKET_CODE_REGION_CODE_ERR			1023//地域CODE设置错误(计划包含2级区域)
#define	PACKET_CODE_BRAND_ERR				1024//品牌广告冲突
#define	PACKET_CODE_PRICE_ERR				1025//余额不足
#define	PACKET_CODE_BRAND_CREATIVE_ERR			1026//品牌广告创意未设置
#define	PACKET_CODE_BRAND_REGION_LEN_ERR		1027//品牌广告地域CODE过多

#define PACKET_CODE_NOT_KEY                             9995//没有这个ID
#define	PACKET_CODE_NOT_DATA				9996//没有数据
#define	PACKET_CODE_PARAMETER_ERR			9997//报文参数错误
#define	PACKET_CODE_FORMAT_ERR				9998//报文格式错误
#define	PACKET_CODE_ERR					9999//服务器内部错误

#define	PACKET_HEAD "server="
#define	PACKET_HEAD_LEN 7
#define	PACKET_BUFF_SIZE 409600

#ifdef __cplusplus
extern "C" { 
#endif

    extern int ad_system_packet_parse(ngx_http_request_t *r);
    extern int ad_system_http_send(ngx_http_request_t *r, ngx_chain_t *buffer, size_t size);
    extern ngx_chain_t *ad_system_buffer_to_ngx_chain(ngx_http_request_t *r, char *p_buffer, size_t size);
    extern int ad_system_http_send_buffer(ngx_http_request_t *r, char *buffer, size_t size);
    extern void ad_system_chain_add(ngx_chain_t **head, ngx_chain_t **next, ngx_chain_t *p);
    extern void ad_system_chain_add_r(ngx_http_request_t *r, ngx_chain_t **head, ngx_chain_t **next, char *buffer, size_t size);
    extern char *ngx_ad_system_get_ipaddr(ngx_http_request_t *r);
    extern int ngx_ad_system_get_session(ngx_http_request_t *r);
    extern int ngx_ad_system_set_session(ngx_http_request_t *r, char *session_id);

#ifdef __cplusplus
}
#endif

#endif



