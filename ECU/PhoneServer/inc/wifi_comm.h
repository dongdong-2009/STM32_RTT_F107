#ifndef __WIFI_COMM_H__
#define __WIFI_COMM_H__

typedef enum
{ 
    COMMAND_BASEINFO         	= 1,		//��ȡ������Ϣ����
    COMMAND_POWERGENERATION  	= 2,		//�����������������
    COMMAND_POWERCURVE      	= 3,		//������������
    COMMAND_GENERATIONCURVE  	= 4,		//��������������
		COMMAND_REGISTERID 				= 5,		//�����IDע������
		COMMAND_SETTIME			 			= 6,		//ʱ����������
		COMMAND_SETWIREDNETWORK		= 7,		//����������������
		COMMAND_SETWIFI 					= 8,		//����������������
		COMMAND_SEARCHWIFISTATUS	= 9,		//������������״̬����
		COMMAND_SETWIFIPASSWD			= 10,		//AP������������
	
} eCommandID;// receive state machin


typedef struct
{
	char ECUID[13];											//ECU ID
	unsigned int LifttimeEnergy;				//ECU ��ʷ������
	unsigned short LastSystemPower;			//ECU ��ǰϵͳ����
	unsigned short GenerationCurrentDay;//ECU ���췢����
	char LastToEMA[8];									//ECU ���һ������EMA��ʱ��
	unsigned short InvertersNum;				//ECU ���������
	unsigned short LastInvertersNum;		//ECU ��ǰ���ӵ����������
	unsigned char Length;								//ECU �汾�ų���
	char Version[20];										//ECU �汾
	unsigned char TimeZoneLength;				//ECU ʱ������
	char TimeZone[20];									//ECU ʱ��
	char MacAddress[7];									//ECU ����Mac��ַ
	char WifiMac[7];										//ECU ����Mac��ַ
} stBaseInfo;


unsigned short packetlen(unsigned char *packet);

int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id);





#endif /*__WIFI_COMM_H__*/
