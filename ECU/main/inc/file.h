#ifndef __FILE_H__
#define __FILE_H__
#include "variation.h"


void get_ecuid(char *ecuid);
int get_ecu_type(void);
unsigned short get_panid(void);
char get_channel(void);
float get_lifetime_power(void);	
void update_life_energy(float lifetime_power);

void updateID(void);


int splitString(char *data,char splitdata[20][13]);

int get_id_from_file(inverter_info *firstinverter);

int save_process_result(int item, char *result);	//���ñ�������,���ʵ���ɺ�ѽ�������/home/data/proc_resĿ¼�£�control_client�ѽ�����͸�EMA

void save_record(char sendbuff[], char *date_time);			//ECU���ͼ�¼��EMA��ͬʱ,����Ҳ����һ��

int save_status(char *result, char *date_time);	//���ñ������������ʵ���ɺ󣬰ѽ�����浽�ļ��У�control_client�ѽ�����͸�EMA


#endif /*__FILE_H__*/
