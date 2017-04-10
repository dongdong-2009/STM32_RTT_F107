#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "control_client.h"
#include <board.h>
#include "checkdata.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"
#include "debug.h"
#include "datetime.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "myfile.h"
#include "mysocket.h"
#include "remote_control_protocol.h"

#include "inverter_id.h"


ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t control_client_stack[ 14336 ];
extern struct rt_thread control_client_thread;

extern rt_mutex_t record_data_lock;
char ecuid[13] = {'\0'};

#define ARRAYNUM 6
#define MAXBUFFER 4096
#define FIRST_TIME_CMD_NUM 12

typedef struct socket_config
{
	int timeout;
	int report_interval;
	int port1;
	int port2;
	char domain[32];
	char ip[16];
}Socket_Cfg;

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150,A151,
};
int (*pfun[100])(const char *recvbuffer, char *sendbuffer);
Socket_Cfg sockcfg = {'\0'};


void add_functions()
{
//  pfun[A102] = response_inverter_id; 			//�ϱ������ID
//  pfun[A103] = set_inverter_id; 				//���������ID
//  pfun[A104] = response_time_zone; 			//�ϱ�ECU����ʱ��
//	pfun[A105] = set_time_zone; 				//����ECU����ʱ��
//	pfun[A106] = response_comm_config;			//�ϱ�ECUͨѶ���ò���
//	pfun[A107] = set_comm_config;				//����ECUͨѶ���ò���
//	pfun[A108] = custom_command;				//��ECU�����Զ�������
//	pfun[A109] = set_inverter_ac_protection_5; 	//���������������������(5��)
//	pfun[A110] = set_inverter_maxpower;			//��������������
//	pfun[A111] = set_inverter_onoff;			//������������ػ�
//	pfun[A112] = clear_inverter_gfdi;			//���������GFDI
//	pfun[A113] = response_ecu_ac_protection_5;	//�ϱ�ECU��������������(5��)
//	pfun[A114] = read_inverter_ac_protection_5; //��ȡ������Ľ�����������(5��)
//	pfun[A117] = response_inverter_maxpower;	//�ϱ����������ʼ���Χ
//	pfun[A119] = set_ecu_flag;					//����ECU��EMA��ͨѶ����
//	pfun[A120] = response_ecu_ac_protection_13;	//�ϱ�ECU��������������(13��)
//	pfun[A121] = read_inverter_ac_protection_13;//��ȡ������Ľ�����������(13��)
//	pfun[A122] = set_inverter_ac_protection_13;	//����������Ľ�����������(13��)
//	pfun[A124] = read_inverter_grid_environment;//��ȡ�������������
//	pfun[A125] = set_inverter_grid_environment;	//�����������������
//	pfun[A126] = read_inverter_ird;				//��ȡ�������IRDѡ��
//	pfun[A127] = set_inverter_ird;				//�����������IRDѡ��
//	pfun[A128] = read_inverter_signal_strength;	//��ȡ��������ź�ǿ��
//	pfun[A129] = response_grid_quality;			//�ϱ�ϵͳ�ĵ�������
//	pfun[A130] = response_ecu_ac_protection_17;	//�ϱ�ECU��������������(17��)
//	pfun[A131] = read_inverter_ac_protection_17;//��ȡ������Ľ�����������(17��)
//	pfun[A132] = set_inverter_ac_protection_17;	//����������Ľ�����������(17��)
//	pfun[A134] = set_inverter_restore;			//����������Ļ�ԭ��־
//	pfun[A136] = set_inverter_update;			//�����������������־
//	pfun[A138] = set_autoflag_report;			//����ECU�Զ��ϱ�����
//	pfun[A148] = read_wrong_id;					//��ȡ�쳣��3501uid
//	pfun[A149] = response_wrong_id;				//�ϱ��쳣��3501uid
//	pfun[A150] = set_unnormal_id;				//����3501��ȷ��id
//	pfun[A151] = response_changed_id;			//�ϱ��޸ĳɹ���id
}

/* [A118] ECU��������EMA��Ҫִ�еĴ������ */
int first_time_info(const char *recvbuffer, char *sendbuffer)
{
	static int command_id = 0;
	int functions[FIRST_TIME_CMD_NUM] = {
			A102, A104, A106, A113, A114, A117,
			A120, A121, A124, A126, A130, A131,
	};

	//���ú���
	(*pfun[functions[command_id++]%100])(recvbuffer, sendbuffer);
//	debug_msg("cmd:A%d", functions[command_id - 1] + 100);
	rt_hw_us_delay(100000);

	if(command_id < FIRST_TIME_CMD_NUM)
		return 118;
	else
		return 0;
}
/* �����ļ��ж�ȡ�ļ�ֵ�Ա��浽socket���ýṹ���� */
int get_socket_config(Socket_Cfg *cfg, MyArray *array)
{
	int i;

	for(i=0; i<ARRAYNUM; i++){
		if(!strlen(array[i].name))break;
		//��ʱʱ��
		if(!strcmp(array[i].name, "Timeout")){
			cfg->timeout = atoi(array[i].value);
		}
		//��ѵʱ��
		else if(!strcmp(array[i].name, "Report_Interval")){
			cfg->report_interval = atoi(array[i].value);
		}
		//����
		else if(!strcmp(array[i].name, "Domain")){
			strncpy(cfg->domain, array[i].value, 32);
		}
		//IP��ַ
		else if(!strcmp(array[i].name, "IP")){
			strncpy(cfg->ip, array[i].value, 16);
		}
		//�˿�1
		else if(!strcmp(array[i].name, "Port1")){
			cfg->port1 = atoi(array[i].value);
		}
		//�˿�2
		else if(!strcmp(array[i].name, "Port2")){
			cfg->port2 = atoi(array[i].value);
		}
	}
	return 0;
}

/* ���ȡport1��port2 */
int randport(Socket_Cfg cfg)
{
	srand((unsigned)acquire_time());
	if(rand()%2)
		return cfg.port1;
	else
		return cfg.port2;
}


int detection_statusflag(char flag)		//���/home/record/inverstaĿ¼���Ƿ����flag�ļ�¼    ���ڷ���1�������ڷ���0
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//printf("%s\n",path);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] == flag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊflag   ���������flag  �ر��ļ�����return 1
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}		
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

//��status״̬Ϊ2�ı�־��Ϊ1
int change_statusflag1()  //�ı�ɹ�����1
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{

						if(buff[strlen(buff)-2] == '2')
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc('1',fp);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}
						
					}
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;	
}	

void delete_statusflag0()		//�������flag��־ȫ��Ϊ0��Ŀ¼
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return;
						}		
					}
					fclose(fp);
					//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
					unlink(path);
				}			
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;
}

//����ʱ����޸ı�־
int change_statusflag(char *time,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						memset(filetime,0,15);
						memcpy(filetime,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
						filetime[14] = '\0';
						if(!memcmp(time,filetime,14))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc(flag,fp);
							//printf("%s\n",filetime);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}
						
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
	
}

//��ѯһ��flagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
time����ʾ��ȡ����ʱ��
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_statusflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/inversta");
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //��ȡһ������
					{
						if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
						{
							memcpy(time,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
							memcpy(data,buff,(strlen(buff)-18));
							data[strlen(buff)-18] = '\n';
							//printf("time:%s   data:%s\n",time,data);
							rt_thread_delay(RT_TICK_PER_SECOND*1);
							while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}

							*flag = 0;
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}		
					}
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}


void delete_pro_result_flag0()		//�������flag��־ȫ��Ϊ0��Ŀ¼
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return;
						}		
					}
					fclose(fp);
					//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
					unlink(path);
				}			
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;
}

//����ʱ����޸ı�־
int change_pro_result_flag(char *item,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char fileitem[4] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						memset(fileitem,0,4);
						memcpy(fileitem,&buff[strlen(buff)-6],3);				//��ȡÿ����¼��ʱ��
						fileitem[3] = '\0';
						if(!memcmp(item,fileitem,4))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc(flag,fp);
							//printf("%s\n",filetime);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}
						
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
	
}
//��ѯһ��flagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
item����ʾ��ȡ������֡
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_pro_result_flag(char *data,char * item, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/data/proc_res");
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //��ȡһ������
					{
						if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
						{
							memcpy(item,&buff[strlen(buff)-6],3);				//��ȡÿ����¼��item
							memcpy(data,buff,(strlen(buff)-7));
							data[strlen(buff)-7] = '\n';
							//printf("time:%s   data:%s\n",time,data);
							rt_thread_delay(RT_TICK_PER_SECOND*1);
							while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}

							*flag = 0;
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
						}		
					}
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}


//���������쳣״̬
int check_inverter_abnormal_status_sent(int hour)
{
	int sockfd;
	int i, flag, num = 0;
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};

	if(get_hour() != hour)
		return 0;

	//��ѯ�Ƿ���flag=2������
	if(0 == detection_statusflag('2'))
		return 0;
	//��flag=2������,����һ����ȡEMA�Ѵ�ʱ�������
	printmsg("control_client",">>Start Check abnormal status sent");
	sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
	if(sockfd < 0) return -1;
	strcpy(send_buffer, "APS13AAA51A123AAA0");
	strcat(send_buffer, ecuid);
	strcat(send_buffer, "000000000000000000END\n");
	send_socket(sockfd, send_buffer, strlen(send_buffer));
	//����EMAӦ��
	if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
		close(sockfd);
		return 0;
	}
	//У������
	if(msg_format_check(recv_buffer) < 0){
		close(sockfd);
		return 0;
	}
	//�����յ���ʱ���,��ɾ��EMA�Ѵ������(�����Ϊ0)
	flag = msg_get_int(&recv_buffer[18], 1);
	num = 0;
	if(flag){
		num = msg_get_int(&recv_buffer[19], 2);
		for(i=0; i<num; i++){
			strncpy(datetime, &recv_buffer[21 + i*14], 14);
			change_statusflag(datetime,'0');
		}
	}

	//��flag=2�����ݸ�Ϊflag=1
	change_statusflag1();
	close(sockfd);
	
	//������б�־Ϊ0�����������
	delete_statusflag0();
	return 0;
}

/* ���ļ��в�ѯ�Ƿ����������쳣״̬ */
int exist_inverter_abnormal_status()
{
	int result = 0;
	//��ѯ�������ID�ڱ����Ƿ����
	if(1 == detection_statusflag('1')){
				result = 1;
	}
	return result;
}

/* [A123]ECU�ϱ���������쳣״̬ */
int response_inverter_abnormal_status()
{
	int result = 0;
	int  j, sockfd, flag, num, cmd_id, next_cmd_id,havaflag;
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char command[4096] = {'\0'};
	char send_buffer[1024]={'\0'};
	char save_buffer[MAXBUFFER] = {'\0'};
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//��ѯ��������
	char time[15] = {'\0'};
	FILE *fp;	
	printmsg("control_client",">>Start Response Abnormal Status");

	//����socket����
	sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
	if(sockfd < 0) return -1;
	//��������������쳣״̬
	while(search_statusflag(data,time,&havaflag,'1'))		//	��ȡһ��resendflagΪ1������
	{	
		//����һ��������쳣״̬��Ϣ
		if(send_socket(sockfd, data, strlen(data)) < 0){
			continue;
		}
		//����EMAӦ��
		if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
			close(sockfd);
			return 0;
		}
		//У������
		if(msg_format_check(recv_buffer) < 0){
			continue;
		}
		//�����ͺͽ��ܶ��ɹ�����һ��״̬�ı�־��2
		change_statusflag(time,'2');
		//�����յ���ʱ���,��ɾ��EMA�Ѵ�����
		flag = msg_get_int(&recv_buffer[18], 1);
		num = 0;
		if(flag){
			num = msg_get_int(&recv_buffer[19], 2);
			for(j=0; j<num; j++){
				strncpy(datetime, &recv_buffer[21 + j*14], 14);
				change_statusflag(datetime,'0');
			}
		}
		//�ж�Ӧ��֡�Ƿ񸽴�����
		if(strlen(recv_buffer) > (24 + 14*num)){
			memset(command, 0, sizeof(command));
			strncpy(command, &recv_buffer[24 + 14*num], sizeof(command));
			print2msg("control_client","Command", command);
			//У������
			if(msg_format_check(command) < 0)
				continue;
			//���������
			cmd_id = msg_cmd_id(command);

			if(cmd_id==118)
			{
				char da_time[20]={'\0'};
				strncpy(da_time, &recv_buffer[72],14);
				
				fp=fopen("/yuneng/A118.con","w");
				if(fp==NULL)
					return -1;
				else
					{
						fputs("1",fp);
						fclose(fp);
						
						memset(send_buffer,0x00,1024);
						msg_ACK(send_buffer, "A118", da_time, 0);
						send_socket(sockfd, send_buffer, strlen(send_buffer));
						printmsg("control_client",">>End");
						printdecmsg("control_client","socked",sockfd);
						result=1;break;
					}
			}
			//���ú���
			else if(pfun[cmd_id%100]){
				next_cmd_id = (*pfun[cmd_id%100])(command, save_buffer);
				save_process_result(cmd_id, save_buffer);
				if(next_cmd_id > 0){
					memset(command, 0, sizeof(command));
					snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0000000000000A%3d00000000000000END", next_cmd_id);
					(*pfun[next_cmd_id%100])(command, save_buffer);
					save_process_result(next_cmd_id, save_buffer);
				}
				else if(next_cmd_id < 0){
					result = -1;
				}
			}
		}
	}
	//���inversta��flag��־λΪ0�ı�־
	delete_statusflag0();
	close(sockfd);
	return result;
}

/* ��EMA����ͨѶ */
int communication_with_EMA(int next_cmd_id)
{
	int sockfd;
	int cmd_id;
	char timestamp[15] = "00000000000000";
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};
	int one_a118=0;
	
	while(1)
	{
		printmsg("control_client","Start Communication with EMA");
		sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
		if(sockfd < 0) return -1;
		if(next_cmd_id <= 0)
		{
			//ECU��EMA������������ָ��
			msg_REQ(send_buffer);
			send_socket(sockfd, send_buffer, strlen(send_buffer));
			memset(send_buffer, '\0', sizeof(send_buffer));

			//����EMA����������
			if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) < 0){
				close(sockfd);
				break;
			}
			
			//У������
			if(msg_format_check(recv_buffer) < 0){
				close(sockfd);
				continue;
			}
			
			//���������
			cmd_id = msg_cmd_id(recv_buffer);
		}
		else{
			//������һ������(�����������������,�ϱ����ú��ECU״̬)
			cmd_id = next_cmd_id;
			next_cmd_id = 0;
			memset(recv_buffer, 0, sizeof(recv_buffer));
			snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
					ecuid, cmd_id, timestamp);
		}

		//ECUע�����κ�EMAͨѶ
		if(cmd_id == 118){
			if(one_a118==0){
				one_a118=1;
				//system("rm /etc/yuneng/fill_up_data.conf");
				//system("echo '1'>>/etc/yuneng/fill_up_data.conf");
				//system("killall main.exe");
			}
			strncpy(timestamp, &recv_buffer[34], 14);
			next_cmd_id = first_time_info(recv_buffer, send_buffer);
			if(next_cmd_id == 0){
				strncpy(timestamp, "00000000000000", 14);
			}
		}
		//��������ŵ��ú���
		else if(pfun[cmd_id%100]){
			//�����ú���������Ϻ���Ҫִ���ϱ�,��᷵���ϱ������������,���򷵻�0
			next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
		}
		//EMA��������
		else if(cmd_id == 100){
			close(sockfd);
			break;
		}
		else{
			//������Ų�����,��������ʧ��Ӧ��(ÿ������Э���ʱ���λ�ò�ͳһ,����ʱ����Ǹ�����...)
			memset(send_buffer, 0, sizeof(send_buffer));
			snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END",
					ecuid, cmd_id);
		}

		//����Ϣ���͸�EMA(�Զ����㳤��,���ϻس�)
		send_socket(sockfd, send_buffer, strlen(send_buffer));
		printmsg("control_client",">>End");
		close(sockfd);

		//������ܺ�������ֵС��0,�򷵻�-1,������Զ��˳�
		if(next_cmd_id < 0){
			return -1;
		}
	}
	printmsg("control_client",">>End");
	return 0;
}

/* �ϱ�process_result���е���Ϣ */
int response_process_result()
{
	//char sendbuffer[MAXBUFFER] = {'\0'};
	int sockfd, flag;
	//int num, i;
	//int item_num[32] = {0};
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//��ѯ��������
	char item[4] = {'\0'};
	

	{
		//��ѯ����[ECU����]������
		//�����ϱ�ECU��������
		while(search_pro_result_flag(data,item,&flag,'1'))
		{
			printmsg("control_client",">>Start Response ECU Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			if(sockfd < 0) return -1;
			//����һ����¼
			if(send_socket(sockfd, data, strlen(data)) < 0){
				close(sockfd);
				continue;
			}
			//���ͳɹ��򽫱�־λ��0
			change_pro_result_flag(item,'0');
			close(sockfd);
			printmsg("control_client",">>End");
		}				
		delete_pro_result_flag0();
		
		/*
		//��ѯ[���������]�������������,��������
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "SELECT distinct(item) FROM inverter_process_result WHERE flag=1");
		if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
			close_db(db);
			return 0;
		}
		for(i=1; i<=nrow; i++)
		{
			item_num[i-1] = atoi(azResult[i]);
		}
		if(nrow)sqlite3_free_table(azResult);
		//����ƴ�Ӳ��ϱ��������������
		num = 0;
		while(item_num[num] != 0)
		{
			debug_msg("item:%d", item_num[num]);
			item = item_num[num++];
			memset(sql, '\0', sizeof(sql));
			snprintf(sql, sizeof(sql), "SELECT result FROM inverter_process_result WHERE item=%d and flag=1", item);
			if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
				close_db(db);
				return 0;
			}
			//ƴ������
			memset(sendbuffer, 0, sizeof(sendbuffer));
			sprintf(sendbuffer, "APS1300000A%03dAAA0%.12s%04d00000000000000END", item, ecuid, nrow);
			for(i=1; i<=nrow; i++)
			{
				strcat(sendbuffer, azResult[i]);
			}
			if(nrow)sqlite3_free_table(azResult);
			//��������
			debug_msg(">>Start Response Inverter Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			if(sockfd < 0) return -1;
			if(send_socket(sockfd, sendbuffer, strlen(sendbuffer)) < 0){
				close(sockfd);
				continue;
			}
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE inverter_process_result SET flag=0 WHERE item=%d", item);
			update_data(db, sql);
			close(sockfd);
		}
		close_db(db);
		*/
	}
	return 0;
}

void control_client_thread_entry(void* parameter)
{
	int result, ecu_time = 0, ecu_flag = 1;
	char buffer[16] = {'\0'};
	MyArray array[ARRAYNUM] = {'\0'};
	FILE *fp;
	delete_line("/test","/ttt","0",1);
	//��ӹ��ܺ���
  add_functions();
	
	//��ȡECU��ͨѶ����flag
	if(file_get_one(buffer, sizeof(buffer), "/yuneng/ecu_flag.con")){
		ecu_flag = atoi(buffer);
	}

	printdecmsg("control_client","ecu_flag", ecu_flag);
	
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");

	//�������ļ��л�ȡsocketͨѶ����
	if(file_get_array(array, ARRAYNUM, "/yuneng/control.con") == 0){
		get_socket_config(&sockcfg, array);
	}
	
	/* ECU��ѵ��ѭ�� */
	while(1)
	{
		//ÿ��һ��ʱ��EMAȷ��������쳣״̬�Ƿ񱻴洢
		check_inverter_abnormal_status_sent(1);

		fp=fopen("/yuneng/A118.con","r");
		if(fp!=NULL)
		{
			char c='0';
			c=fgetc(fp);
			if(c=='1')
				result = communication_with_EMA(118);
			fclose(fp);
			unlink("/yuneng/A118.con");
		}
		
		if(exist_inverter_abnormal_status() && ecu_flag){
			ecu_time =  acquire_time();
			result = response_inverter_abnormal_status();
			response_process_result();
		}
		else if(compareTime(acquire_time() ,ecu_time,60*sockcfg.report_interval)){
			ecu_time = acquire_time();
			if(ecu_flag){ //���ecu_flag = 0 ���ϱ�������
				response_process_result();
			}
			result = communication_with_EMA(0);
		}
		//����������������ѭ��
		if(result < 0){
			result = 0;
			printmsg("control_client","Quit control_client");
			continue;
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND*sockcfg.report_interval*60/3);
		
	}

}
