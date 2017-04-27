#ifndef __THREADLIST_H
#define __THREADLIST_H

#include <rtthread.h>

typedef enum THREADTYPE {
	TYPE_LED = 1,
	TYPE_LANRST = 2,
  TYPE_UPDATE = 3,
  TYPE_MAIN = 4,
  TYPE_CLIENT = 5,
  TYPE_CONTROL_CLIENT = 6
}threadType;
//ÿ���̵߳����ȼ�   ��򿪱�ʾ�̴߳�
#define THREAD_PRIORITY_INIT							10
//#define THREAD_PRIORITY_LED               20
#define THREAD_PRIORITY_IDWRITER					20
#define THREAD_PRIORITY_LAN8720_RST				21

//#define THREAD_PRIORITY_WIFI_TEST					21
//#define THREAD_PRIORITY_NET_TEST					22
//#define THREAD_PRIORITY_ZIGBEE_TEST				22

//#define THREAD_PRIORITY_UPDATE						21
#define 	THREAD_PRIORITY_IDWRITE						22
//#define THREAD_PRIORITY_MAIN	            24
//#define THREAD_PRIORITY_CONTROL_CLIENT		26
//#define THREAD_PRIORITY_CLIENT						27

//ÿ���߳��ڿ����������ʱ��,��λS
#define START_TIME_UPDATE									0
#define START_TIME_IDWRITE								5
#define START_TIME_MAIN										10
#define START_TIME_CONTROL_CLIENT					60
#define START_TIME_CLIENT									60
void tasks_new(void);//����ϵͳ��Ҫ���߳�

void restartThread(threadType type);

#endif
