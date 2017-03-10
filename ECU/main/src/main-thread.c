#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"
#include "resolve.h"
#include "variation.h"
#include "checkdata.h"
#include "rtc.h"

#define MAIN_VERSION "R-1.0.0"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 1024 ];
extern struct rt_thread main_thread;

inverter_info inverter[MAXINVERTERCOUNT];
ecu_info ecu;


int init_ecu()
{
	char *ecuid = "888888888888";		//�����޸�Ϊ��flash�л�ȡ
	rt_memcpy(ecu.id,ecuid,12);//��ȡECU  ID
	ecu.panid = 0x88;
	//��ȡECU�ŵ�
	ecu.channel = 0x10;
	rt_memset(ecu.ip, '\0', sizeof(ecu.ip));
	ecu.life_energy = 0;			//������flash�л�ȡ��ʷ������
	ecu.current_energy = 0;
	ecu.system_power = 0;
	ecu.count = 0;
	ecu.total = 0;
	ecu.flag_ten_clock_getshortaddr = 1;			//ZK
	ecu.polling_total_times=0;					//ECUһ��֮����ѵ�Ĵ�����0, ZK
	ecu.type = 1;
	ecu.zoneflag = 0;				//ʱ��
	printecuinfo(&ecu);
	return 1;
}

int init_inverter(inverter_info *inverter)
{
	int i;
	char flag_limitedid = '0';				//�޶�ID��־

	inverter_info *curinverter = inverter;

	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		rt_memset(curinverter->id, '\0', sizeof(curinverter->id));		//��������UID
		rt_memset(curinverter->tnuid, '\0', sizeof(curinverter->tnuid));			//��������ID

		curinverter->model = 0;

		curinverter->dv=0;			//��յ�ǰһ��ֱ����ѹ
		curinverter->di=0;			//��յ�ǰһ��ֱ������
		curinverter->op=0;			//��յ�ǰ������������
		curinverter->gf=0;			//��յ���Ƶ��
		curinverter->it=0;			//���������¶�
		curinverter->gv=0;			//��յ�����ѹ
		curinverter->dvb=0;			//B·��յ�ǰһ��ֱ����ѹ
		curinverter->dib=0;			//B·��յ�ǰһ��ֱ������
		curinverter->opb=0;			//B·��յ�ǰ������������
		curinverter->gvb=0;
		curinverter->dvc=0;
		curinverter->dic=0;
		curinverter->opc=0;
		curinverter->gvc=0;
		curinverter->dvd=0;
		curinverter->did=0;
		curinverter->opd=0;
		curinverter->gvd=0;



		curinverter->curgeneration = 0;	//����������ǰһ�ַ�����
		curinverter->curgenerationb = 0;	//B·��յ�ǰһ�ַ�����

		curinverter->preaccgen = 0;
		curinverter->preaccgenb = 0;
		curinverter->curaccgen = 0;
		curinverter->curaccgenb = 0;
		curinverter->preacctime = 0;
		curinverter->curacctime = 0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//��������״̬
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));		//��������״̬
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));		//B·��������״̬

		curinverter->dataflag = 0;		//��һ�������ݵı�־��λ
	//	curinverter->bindflag=0;		//���������־λ����0
		curinverter->no_getdata_num=0;	//ZK,���������ȡ�����Ĵ���
		curinverter->disconnect_times=0;		//û���������ͨ���ϵĴ�����0, ZK
		curinverter->signalstrength=0;			//�ź�ǿ�ȳ�ʼ��Ϊ0

		curinverter->updating=0;
		curinverter->raduis=0;
	}

	get_ecu_type();		//��ȡECU�ͺ�

	flag_limitedid = '1';		//������flash�л�ȡ�޶�ID��־


	if ('1' == flag_limitedid) {
		while(1) {
			//bind_inverters(); //�������
			//ecu.total = get_id_from_db(inverter); //��ȡ���������
			if (ecu.total > 0) {
				break; //ֱ���������������0ʱ�˳�ѭ��
			} else {
				//display_input_id(); //��ʾ�û����������ID
				rt_thread_delay(5*RT_TICK_PER_SECOND);
			}
		}
		flag_limitedid = '0';
	}
	else {
		while(1) {
			//ecu.total = get_id_from_db(inverter);
			if (ecu.total > 0) {
				break; //ֱ���������������0ʱ�˳�ѭ��
			} else {
				//display_input_id(); //��ʾ�û����������ID
				rt_thread_delay(5*RT_TICK_PER_SECOND);
			}
		}
	}

	return 1;
}


int init_all(inverter_info *inverter)
{

	openzigbee();
	init_ecu();
	init_inverter(inverter);

	return 0;
}

int reset_inverter(inverter_info *inverter)
{
	int i;
	inverter_info *curinverter = inverter;

	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		curinverter->dataflag = 0;

		curinverter->dv=0;
		curinverter->di=0;
		curinverter->op=0;
		curinverter->gf=0;
		curinverter->it=0;
		curinverter->gv=0;

		curinverter->dvb=0;
		curinverter->dib=0;
		curinverter->opb=0;

		curinverter->curgeneration = 0;
		curinverter->curgenerationb = 0;
		curinverter->status_send_flag=0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//????????
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));
	}

	return 1;
}



void main_thread_entry(void* parameter)
{
	int thistime=0, durabletime=65535, reportinterval=300;					//thistime:��������������͹㲥Ҫ���ݵ�ʱ��;durabletime:ECU�����������Ҫ���ݵĳ���ʱ��
	//char broadcast_hour_minute[3]={'\0'};									//����������͹㲥����ʱ��ʱ��
	//int cur_time_hour;														//��ǰ��ʱ��Сʱ


	rt_kprintf("\nmain.exe %s\n", MAIN_VERSION);
	printmsg("Start-------------------------------------------------");

	init_all(inverter);   //��ʼ�����������
	
	while(1)
	{
		if((durabletime-thistime) >= reportinterval){
		//if((durabletime-thistime) >= 60){
			thistime = time(RT_NULL);
		
		}
		
	}
	
}
