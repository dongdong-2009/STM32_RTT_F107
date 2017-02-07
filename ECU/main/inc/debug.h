#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "variation.h"



extern void printmsg(char *msg);		//��ӡ�ַ���
extern void print2msg(char *msg1, char *msg2);		//��ӡ�ַ���
extern void printdecmsg(char *msg, int data);		//��ӡ��������
extern void printhexdatamsg(char *msg, int data);		//��ӡ16��������,ZK
extern void printfloatmsg(char *msg, float data);		//��ӡʵ��
extern void printhexmsg(char *msg, char *data, int size);		//��ӡʮ����������
extern void printecuinfo(ecu_info *ecu);
extern void printinverterinfo(inverter_info *inverter);

#endif /*__DEBUG_H__*/
