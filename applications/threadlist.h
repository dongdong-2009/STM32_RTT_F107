#ifndef __THREADLIST_H
#define __THREADLIST_H
/*****************************************************************************/
/* File      : threadlist.h                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <rtthread.h>
#include "arch/sys_arch.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_USE 
//restartThread parameter
typedef enum THREADTYPE {
	TYPE_LED = 1,
	TYPE_LANRST = 2,
  TYPE_UPDATE = 3,
	TYPE_IDWRITE = 4,
  TYPE_MAIN = 5,
  TYPE_CLIENT = 6,
  TYPE_CONTROL_CLIENT = 7,
	TYPE_NTP = 8
}threadType;

typedef struct IPConfig
{
	IP_t IPAddr;
	IP_t MSKAddr;
	IP_t GWAddr;
	IP_t DNS1Addr;
	IP_t DNS2Addr;
		
} IPConfig_t;


//Thread Priority
//Init device thread priority
#define THREAD_PRIORITY_INIT							10
//LAN8720A Monitor thread priority
#define THREAD_PRIORITY_LAN8720_RST				11
//LED thread priority
#define THREAD_PRIORITY_LED               12

//phone Server thread priority
#define THREAD_PRIORITY_PHONE_SERVER			19

//Update thread priority
#define THREAD_PRIORITY_UPDATE						20
//NTP thread priority
//#define THREAD_PRIORITY_NTP								21
//ID Writethread priority
#define THREAD_PRIORITY_IDWRITE						22
//DRM Connect thread priority
#define THREAD_PRIORITY_DRM								23
//data collection thread priority
#define THREAD_PRIORITY_MAIN	            24
//control client thread priority
#define THREAD_PRIORITY_CONTROL_CLIENT		26
//data uploading thread priority
#define THREAD_PRIORITY_CLIENT						27



//thread start time
#define START_TIME_UPDATE									15
#define START_TIME_NTP										10
#define START_TIME_IDWRITE								5
#define START_TIME_MAIN										0			//90
#define START_TIME_CONTROL_CLIENT					20			//120
#define START_TIME_CLIENT									60			//180
#define START_TIME_PHONE_SERVER						20

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//����ϵͳ��Ҫ���߳�
void tasks_new(void);
//��λ�߳�
void restartThread(threadType type);

#endif
