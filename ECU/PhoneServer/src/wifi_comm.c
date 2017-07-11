#include "wifi_comm.h"
#include "string.h"
#include "variation.h"
#include "usr_wifi232.h"
#include "stdio.h"
#include "version.h"

extern ecu_info ecu;
static char SendData[4096] = {'\0'};

//�������ĳ���
unsigned short packetlen(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//�����յ�������
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id)
{
	//APS
	if(strncmp(RecvData, "APS", 3))
		return -1;
	//�汾�� 
	//����
	*Data_Len = packetlen((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*10 + (RecvData[10]-'0');
	return 0;
}

//01 COMMAND_BASEINFO 					//��ȡ������Ϣ����
void APP_Response_BaseInfo(char *ID,stBaseInfo baseInfo)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS11000001%s",baseInfo.ECUID);
	packlength = 23;
	SendData[packlength++] = baseInfo.LastSystemPower/256;
	SendData[packlength++] = baseInfo.LastSystemPower%256;
	SendData[packlength++] = baseInfo.GenerationCurrentDay/256;
	SendData[packlength++] = baseInfo.GenerationCurrentDay%256;
	memcpy(&SendData[packlength],baseInfo.LastToEMA,7);
	packlength += 7;
	SendData[packlength++] = baseInfo.InvertersNum/256;
	SendData[packlength++] = baseInfo.InvertersNum%256;
	SendData[packlength++] = baseInfo.LastInvertersNum/256;
	SendData[packlength++] = baseInfo.LastInvertersNum%256;
	SendData[packlength++] = '0';
	SendData[packlength++] = '0';
	SendData[packlength++] = '9';
	sprintf(&SendData[packlength],"%s_%s_%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	packlength += 9;
	SendData[packlength++] = '0';
	SendData[packlength++] = '0';
	SendData[packlength++] = '9';
	sprintf(&SendData[packlength],"Etc/GMT+8");
	packlength += 9;		
	memcpy(&SendData[packlength],baseInfo.MacAddress,6);
	packlength += 6;
	memcpy(&SendData[packlength],baseInfo.WifiMac,6);
	packlength += 6;
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	SendData[packlength++] = '\n';
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	
	SendToSocketA(SendData ,packlength,ID);
}

//02 COMMAND_POWERGENERATION		//�����������������  mapping :: 0x00 ƥ��  0x01 ��ƥ��
void APP_Response_PowerGeneration(char mapping,char *ID,inverter_info *inverter,int VaildNum)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130201\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƥ��ɹ� 
	if(ecu.had_data_broadcast_time[0] == '\0')
	{
		sprintf(SendData,"APS1100130202\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100000200");
	packlength = 13;
	
	SendData[packlength++] = VaildNum/256;
	SendData[packlength++] = VaildNum%256;
	memcpy(&SendData[packlength],ecu.had_data_broadcast_time,14);
	packlength += 14;
	
	for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)
	{
		//UID
		SendData[packlength++] = ((curinverter->id[0]-'0') << 4) + (curinverter->id[1]-'0');
		SendData[packlength++] = ((curinverter->id[2]-'0') << 4) + (curinverter->id[3]-'0');
		SendData[packlength++] = ((curinverter->id[4]-'0') << 4) + (curinverter->id[5]-'0');
		SendData[packlength++] = ((curinverter->id[6]-'0') << 4) + (curinverter->id[7]-'0');
		SendData[packlength++] = ((curinverter->id[8]-'0') << 4) + (curinverter->id[9]-'0');
		SendData[packlength++] = ((curinverter->id[10]-'0') << 4)+ (curinverter->id[11]-'0');
		
		//����Ƶ��
		SendData[packlength++] = (int)(curinverter->gf * 10) / 256;
		SendData[packlength++] = (int)(curinverter->gf * 10) % 256;

		//�����¶�
		SendData[packlength++] = (curinverter->it + 100) /256;
		SendData[packlength++] = (curinverter->it + 100) %256;		
		
		//���������  A 
		SendData[packlength++] = curinverter->op / 256;
		SendData[packlength++] = curinverter->op % 256;;
			
		//������ѹ    A
		SendData[packlength++] = curinverter->gv / 256;
		SendData[packlength++] = curinverter->gv % 256;
		
		//���������  B 
		SendData[packlength++] = curinverter->opb / 256;
		SendData[packlength++] = curinverter->opb % 256;
			
		//������ѹ    B
		SendData[packlength++] = curinverter->gvb / 256;
		SendData[packlength++] = curinverter->gvb % 256;
		
	}
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	SendData[packlength++] = '\n';
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	
	SendToSocketA(SendData ,packlength,ID);
}

//03 COMMAND_POWERCURVE					//������������   mapping :: 0x00 ƥ��  0x01 ��ƥ��   data ��ʾ����
void APP_Response_PowerCurve(char mapping,char *ID,char * date)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	//ƴ����Ҫ���͵ı���
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130301\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	
	
	SendToSocketA(SendData ,packlength,ID);
}

//04 COMMAND_GENERATIONCURVE		//��������������    mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_GenerationCurve(char mapping,char *ID,char request_type)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//05 COMMAND_REGISTERID 				//�����IDע������
void APP_Response_RegisterID(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//06 COMMAND_SETTIME						//ʱ����������
void APP_Response_SetTime(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//07 COMMAND_SETWIREDNETWORK		//����������������
void APP_Response_SetWiredNetwork(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//08 COMMAND_SETWIFI 						//����������������
void APP_Response_SetWifi(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//09 COMMAND_SEARCHWIFISTATUS		//������������״̬����
void APP_Response_SearchWifiStatus(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);
}

//10 COMMAND_SETWIFIPASSWD			//AP������������
void APP_Response_SetWifiPasswd(char mapping,char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130401\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���

	SendToSocketA(SendData ,packlength,ID);	
}

