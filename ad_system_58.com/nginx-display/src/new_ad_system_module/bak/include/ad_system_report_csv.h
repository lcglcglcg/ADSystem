
#ifndef	__AD_SYSTEM_REPORT_CSV_H__
#define	__AD_SYSTEM_REPORT_CSV_H__

#include "lg_list.h"

extern int ad_system_report_standard_csv(int report_type, char *report_path, int bidding_model, lg_list_t *head);
extern int ad_system_report_complex_csv(int report_type, char *report_path, int bidding_model, lg_list_t *head);
extern int ad_system_report_csv_comperssion(char *report_id, char *report_path);

#endif


