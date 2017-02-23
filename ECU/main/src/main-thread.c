#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"
#include "resolve.h"

#define MAIN_VERSION "R-1.0.0"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 1024 ];
extern struct rt_thread main_thread;

inverter_info inverter[MAXINVERTERCOUNT];
int init_all(inverter_info *inverter)
{

	openzigbee();
	//init_ecu();
	//init_inverter(inverter);

	return 0;
}


void main_thread_entry(void* parameter)
{
	//int thistime=0, durabletime=65535, reportinterval=300;					//thistime:��������������͹㲥Ҫ���ݵ�ʱ��;durabletime:ECU�����������Ҫ���ݵĳ���ʱ��
	//char broadcast_hour_minute[3]={'\0'};									//����������͹㲥����ʱ��ʱ��
	//int cur_time_hour;														//��ǰ��ʱ��Сʱ


	rt_kprintf("\nmain.exe %s\n", MAIN_VERSION);
	printmsg("Start-------------------------------------------------");
  
	init_all(inverter);   //��ʼ�����������
	while(1)
	{
	}
}
