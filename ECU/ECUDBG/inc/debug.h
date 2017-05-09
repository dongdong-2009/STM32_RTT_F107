#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "variation.h"

typedef enum DEBUG {
  ECU_DBG_UPDATE = 0,
	ECU_DBG_IDWRITE = 1,
  ECU_DBG_MAIN = 2,
  ECU_DBG_CLIENT = 3,
  ECU_DBG_CONTROL_CLIENT = 4,
	ECU_DBG_NTP = 5,
	ECU_DBG_OTHER = 6,
	
}DebugType;

#define ECU_DBG_OFF           		0x0
#define ECU_DBG_ON           			0x1

#define ECU_DEBUG									ECU_DBG_ON

#ifndef ECU_DEBUG
#define ECU_DEBUG									ECU_DBG_ON
#endif 

//�꿪�أ��򿪹ر��̴߳�ӡ��Ϣ
#define ECU_DEBUG_UPDATE					ECU_DBG_ON
#define ECU_DEBUG_IDWRITE					ECU_DBG_ON
#define ECU_DEBUG_MAIN						ECU_DBG_ON
#define ECU_DEBUG_CLIENT					ECU_DBG_ON
#define ECU_DEBUG_CONTROL_CLIENT	ECU_DBG_ON
#define ECU_DEBUG_NTP							ECU_DBG_ON
#define ECU_DEBUG_OTHER						ECU_DBG_ON


extern void printmsg(DebugType type,char *msg);		//��ӡ�ַ���
extern void print2msg(DebugType type,char *msg1, char *msg2);		//��ӡ�ַ���
extern void printdecmsg(DebugType type,char *msg, int data);		//��ӡ��������
extern void printhexdatamsg(DebugType type,char *msg, int data);		//��ӡ16��������,ZK
extern void printfloatmsg(DebugType type,char *msg, float data);		//��ӡʵ��
extern void printhexmsg(DebugType type,char *msg, char *data, int size);		//��ӡʮ����������
extern void printecuinfo(ecu_info *ecu);
extern void printinverterinfo(inverter_info *inverter);

#endif /*__DEBUG_H__*/
