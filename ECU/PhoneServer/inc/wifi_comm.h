#ifndef __WIFI_COMM_H__
#define __WIFI_COMM_H__

#include "variation.h"
#include "arch/sys_arch.h"

typedef struct
{
	char ECUID[13];											//ECU ID
	unsigned int LifttimeEnergy;				//ECU ��ʷ������
	unsigned int LastSystemPower;			//ECU ��ǰϵͳ����
	unsigned int GenerationCurrentDay;//ECU ���췢����
	char LastToEMA[8];									//ECU ���һ������EMA��ʱ��
	unsigned short InvertersNum;				//ECU ���������
	unsigned short LastInvertersNum;		//ECU ��ǰ���ӵ����������
	unsigned char Length;								//ECU �汾�ų���
	char Version[20];										//ECU �汾
	unsigned char TimeZoneLength;				//ECU ʱ������
	char TimeZone[20];									//ECU ʱ��
	char MacAddress[7];									//ECU ����Mac��ַ
	char WifiMac[7];										//ECU ����Mac��ַ
	char Channel[3];										//�ŵ�
} stBaseInfo;

unsigned short packetlen(unsigned char *packet);

int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id);
int phone_add_inverter(int num,const char *uidstring);
//01	��ȡ������Ϣ����
void APP_Response_BaseInfo(unsigned char *ID,stBaseInfo baseInfo);
//02	�����������������
void APP_Response_PowerGeneration(char mapping,unsigned char *ID,inverter_info *inverter,int VaildNum);
//03	������������
void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date);
//04	��������������
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char request_type);
//05	�����IDע������
void APP_Response_RegisterID(char mapping,unsigned char *ID);
//06	ʱ����������
void APP_Response_SetTime(char mapping,unsigned char *ID);
//07	����������������
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID);
//08 	��ȡECUӲ����Ϣ
void APP_Response_GetECUHardwareStatus(char mapping,unsigned char *ID);
//10	AP������������
void APP_Response_SetWifiPasswd(char mapping,unsigned char *ID);
//11	AP������������
void APP_Response_GetIDInfo(char mapping,unsigned char *ID,inverter_info *inverter);
//12	AP������������
void APP_Response_GetTime(char mapping,unsigned char *ID,char *Time);
//13	FlashSize �ж�
void APP_Response_FlashSize(char mapping,unsigned char *ID,unsigned int Flashsize);
//14	��ȡ������������
void APP_Response_GetWiredNetwork(char mapping,unsigned char *ID,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr);
//15 	�����ŵ�
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL);
//18 	��ȡ�̵�ַ
void APP_Response_GetShortAddrInfo(char mapping,unsigned char *ID,inverter_info *inverter);

#endif /*__WIFI_COMM_H__*/
