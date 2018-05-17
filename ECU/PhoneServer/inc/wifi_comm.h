#ifndef __WIFI_COMM_H__
#define __WIFI_COMM_H__

#include "variation.h"
#include "arch/sys_arch.h"
#include "phoneServer.h"

typedef struct
{
    char ECUID[13];											//ECU ID
    unsigned int LifttimeEnergy;				//ECU ��ʷ������
    unsigned int LastSystemPower;			//ECU ��ǰϵͳ����
    unsigned int GenerationCurrentDay;//ECU ���췢����
    char LastToEMA[8];									//ECU ���һ������EMA��ʱ��
    unsigned short InvertersNum;				//ECU ���������
    unsigned short LastInvertersNum;		//ECU ��ǰ���ӵ����������
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
void APP_Response_BaseInfo(stBaseInfo baseInfo);
//02	�����������������
void APP_Response_PowerGeneration(char mapping,inverter_info *inverter,int VaildNum);
//03	������������
void APP_Response_PowerCurve(char mapping,char * date);
//04	��������������
void APP_Response_GenerationCurve(char mapping,char request_type);
//05	�����IDע������
void APP_Response_RegisterID(char mapping);
//06	ʱ����������
void APP_Response_SetTime(char mapping);
//07	����������������
void APP_Response_SetWiredNetwork(char mapping);
//08 	��ȡECUӲ����Ϣ
void APP_Response_GetECUHardwareStatus(char mapping);
//10	AP������������
void APP_Response_SetWifiPasswd(char mapping);
//11	AP������������
void APP_Response_GetIDInfo(char mapping,inverter_info *inverter);
//12	AP������������
void APP_Response_GetTime(char mapping,char *Time);
//13	FlashSize �ж�
void APP_Response_FlashSize(char mapping,unsigned int Flashsize);
//14	��ȡ������������
void APP_Response_GetWiredNetwork(char mapping,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr);
//15 	�����ŵ�
void APP_Response_SetChannel(unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL);
//18 	��ȡ�̵�ַ
void APP_Response_GetShortAddrInfo(char mapping,inverter_info *inverter);
//20 �����Ӧ
void APP_Response_GetECUAPInfo(char mapping,unsigned char connectStatus,char *info);
//21 �����Ӧ
void APP_Response_SetECUAPInfo(unsigned char result);
//22 �����Ӧ
void APP_Response_GetECUAPList(char mapping,char *list);
//23 �����Ӧ
void APP_Response_GetFunctionStatusInfo(char mapping);
//24 �����Ӧ
void APP_Response_ServerInfo(char mapping,ECUServerInfo_t *serverInfo);
//25 �����Ӧ
void APP_Response_InverterMaxPower(char mapping,int cmd,inverter_info *inverter,const char *recvbuffer,int length);
//26 �����Ӧ
void APP_Response_InverterOnOff(char mapping,int cmd,inverter_info *inverter,const char *recvbuffer,int length);
//27 �����Ӧ
void APP_Response_InverterGFDI(char mapping,int cmd,inverter_info *inverter,const char *recvbuffer,int length);
//28 �����Ӧ
void APP_Response_InverterIRD(char mapping,int cmd,inverter_info *inverter,const char *recvbuffer,int length);
// 30 �����Ӧ
void APP_Response_RSSI(char mapping,inverter_info *inverter);
// 31 �����Ӧ
void APP_Response_ClearEnergy(char mapping);
// 32�����Ӧ
void APP_Response_AlarmEvent(char mapping,const char *Date,const char *serial);
#endif /*__WIFI_COMM_H__*/
