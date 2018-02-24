#include "wifi_comm.h"
#include "string.h"
#include "variation.h"
#include "stdio.h"
#include "version.h"
#include "file.h"
#include "rtc.h"
#include "usart5.h"
#include "stdlib.h"
#include "threadlist.h"

extern ecu_info ecu;
static char SendData[MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER] = {'\0'};
extern unsigned char rateOfProgress;

int phone_add_inverter(int num,const char *uidstring)
{
	int i = 0;
	char buff[25] = { '\0' };
	char *allbuff = NULL;
	allbuff = malloc(2500);
	memset(allbuff,0x00,2500);
	for(i = 0; i < num; i++)
	{
		memset(buff,'\0',25);
		sprintf(buff,"%02x%02x%02x%02x%02x%02x,,,,,,\n",uidstring[0+i*6],uidstring[1+i*6],uidstring[2+i*6],uidstring[3+i*6],uidstring[4+i*6],uidstring[5+i*6]);
		memcpy(&allbuff[0+19*i],buff,19);
	}
	
	
	echo("/home/data/id",allbuff);
	echo("/yuneng/limiteid.con","1");
	free(allbuff);
	allbuff = NULL;
	return 0;
}

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
	*Command_Id = (RecvData[9]-'0')*1000 + (RecvData[10]-'0')*100 + (RecvData[11]-'0')*10 + (RecvData[12]-'0');
	return 0;
}

//01 COMMAND_BASEINFO 					//��ȡ������Ϣ����
void APP_Response_BaseInfo(stBaseInfo baseInfo)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100000001%s",baseInfo.ECUID);
	packlength = 25;

	SendData[packlength++] = '0';
	SendData[packlength++] = '1';
	
	SendData[packlength++] = (baseInfo.LifttimeEnergy/16777216)%256;
	SendData[packlength++] = (baseInfo.LifttimeEnergy/65536)%256;
	SendData[packlength++] = (baseInfo.LifttimeEnergy/256)%256;
	SendData[packlength++] =  baseInfo.LifttimeEnergy%256;

	SendData[packlength++] = (baseInfo.LastSystemPower/16777216)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/65536)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/256)%256;
	SendData[packlength++] = baseInfo.LastSystemPower%256;

	SendData[packlength++] = (baseInfo.GenerationCurrentDay/16777216)%256;
	SendData[packlength++] = (baseInfo.GenerationCurrentDay/65536)%256;
	SendData[packlength++] = (baseInfo.GenerationCurrentDay/256)%256;
	SendData[packlength++] = baseInfo.GenerationCurrentDay%256;
	
	
	memcpy(&SendData[packlength],baseInfo.LastToEMA,7);
	packlength += 7;
	
	SendData[packlength++] = baseInfo.InvertersNum/256;
	SendData[packlength++] = baseInfo.InvertersNum%256;
	
	SendData[packlength++] = baseInfo.LastInvertersNum/256;
	SendData[packlength++] = baseInfo.LastInvertersNum%256;

	SendData[packlength++] = baseInfo.Channel[0];
	SendData[packlength++] = baseInfo.Channel[1];
	
	SendData[packlength++] = '0' + (ECU_VERSION_LENGTH/100)%10;
	SendData[packlength++] = '0'+ (ECU_VERSION_LENGTH/10)%10;
	SendData[packlength++] = '0'+ (ECU_VERSION_LENGTH%10);
	sprintf(&SendData[packlength],"%s_%s.%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	packlength += ECU_VERSION_LENGTH;
	
	SendData[packlength++] = '0';
	SendData[packlength++] = '0';
	SendData[packlength++] = '9';
	sprintf(&SendData[packlength],"Etc/GMT-8");
	packlength += 9;	
	
	memcpy(&SendData[packlength],baseInfo.MacAddress,6);
	packlength += 6;
	
	memset(baseInfo.WifiMac,0x00,6);
	memcpy(&SendData[packlength],baseInfo.WifiMac,6);
	packlength += 6;
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	SendToSocketA(SendData ,packlength);
}

//02 COMMAND_POWERGENERATION		//�����������������  mapping :: 0x00 ƥ��  0x01 ��ƥ��
void APP_Response_PowerGeneration(char mapping,inverter_info *inverter,int VaildNum)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000201\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
		return ;
	}
	
	//ƥ��ɹ� 
	if(ecu.had_data_broadcast_time[0] == '\0')
	{
		sprintf(SendData,"APS110015000202\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
		return ;
	}
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS110000000200");
	packlength = 15;

	SendData[packlength++] = '0';
	SendData[packlength++] = '1';
	
	SendData[packlength++] = VaildNum/256;
	SendData[packlength++] = VaildNum%256;
	
	SendData[packlength++] = (ecu.had_data_broadcast_time[0] - '0')*16+(ecu.had_data_broadcast_time[1] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[2] - '0')*16+(ecu.had_data_broadcast_time[3] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[4] - '0')*16+(ecu.had_data_broadcast_time[5] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[6] - '0')*16+(ecu.had_data_broadcast_time[7] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[8] - '0')*16+(ecu.had_data_broadcast_time[9] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[10] - '0')*16+(ecu.had_data_broadcast_time[11] - '0');
	SendData[packlength++] = (ecu.had_data_broadcast_time[12] - '0')*16+(ecu.had_data_broadcast_time[13] - '0');
		
	for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)
	{
		if((curinverter->model == 7))
		{
			//UID
			SendData[packlength++] = ((curinverter->id[0]-'0') << 4) + (curinverter->id[1]-'0');
			SendData[packlength++] = ((curinverter->id[2]-'0') << 4) + (curinverter->id[3]-'0');
			SendData[packlength++] = ((curinverter->id[4]-'0') << 4) + (curinverter->id[5]-'0');
			SendData[packlength++] = ((curinverter->id[6]-'0') << 4) + (curinverter->id[7]-'0');
			SendData[packlength++] = ((curinverter->id[8]-'0') << 4) + (curinverter->id[9]-'0');
			SendData[packlength++] = ((curinverter->id[10]-'0') << 4)+ (curinverter->id[11]-'0');

			SendData[packlength++] = (curinverter->inverterstatus.dataflag & 0x01);

			//���������
			SendData[packlength++] = '0';
			SendData[packlength++] = '1';
			
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
			SendData[packlength++] = curinverter->gv / 256;
			SendData[packlength++] = curinverter->gv % 256;
		}else if((curinverter->model == 5) || (curinverter->model == 6))
		{
			//UID
			SendData[packlength++] = ((curinverter->id[0]-'0') << 4) + (curinverter->id[1]-'0');
			SendData[packlength++] = ((curinverter->id[2]-'0') << 4) + (curinverter->id[3]-'0');
			SendData[packlength++] = ((curinverter->id[4]-'0') << 4) + (curinverter->id[5]-'0');
			SendData[packlength++] = ((curinverter->id[6]-'0') << 4) + (curinverter->id[7]-'0');
			SendData[packlength++] = ((curinverter->id[8]-'0') << 4) + (curinverter->id[9]-'0');
			SendData[packlength++] = ((curinverter->id[10]-'0') << 4)+ (curinverter->id[11]-'0');

			SendData[packlength++] = (curinverter->inverterstatus.dataflag & 0x01);

			//���������
			SendData[packlength++] = '0';
			SendData[packlength++] = '2';
			
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


			//���������  C 
			SendData[packlength++] = curinverter->opc / 256;
			SendData[packlength++] = curinverter->opc % 256;
				
			//������ѹ    C
			SendData[packlength++] = curinverter->gvc / 256;
			SendData[packlength++] = curinverter->gvc % 256;

			//���������  D 
			SendData[packlength++] = curinverter->opd / 256;
			SendData[packlength++] = curinverter->opd % 256;
				
		}else
		{
			//UID
			SendData[packlength++] = ((curinverter->id[0]-'0') << 4) + (curinverter->id[1]-'0');
			SendData[packlength++] = ((curinverter->id[2]-'0') << 4) + (curinverter->id[3]-'0');
			SendData[packlength++] = ((curinverter->id[4]-'0') << 4) + (curinverter->id[5]-'0');
			SendData[packlength++] = ((curinverter->id[6]-'0') << 4) + (curinverter->id[7]-'0');
			SendData[packlength++] = ((curinverter->id[8]-'0') << 4) + (curinverter->id[9]-'0');
			SendData[packlength++] = ((curinverter->id[10]-'0') << 4)+ (curinverter->id[11]-'0');

			SendData[packlength++] = 0;
			
			//���������
			SendData[packlength++] = '0';
			SendData[packlength++] = '0';
		}
		
		
	}
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	SendToSocketA(SendData ,packlength);
}

//03 COMMAND_POWERCURVE					//������������   mapping :: 0x00 ƥ��  0x01 ��ƥ��   data ��ʾ����
void APP_Response_PowerCurve(char mapping,char * date)
{
	int packlength = 0,length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000301\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
		return ;
	}

	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS110015000300");
	packlength = 15;
	
	read_system_power(date,&SendData[15],&length);
	packlength += length;
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength);
}

//04 COMMAND_GENERATIONCURVE		//��������������    mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_GenerationCurve(char mapping,char request_type)
{
	int packlength = 0,len_body = 0;
	char date_time[15] = { '\0' };
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	apstime(date_time);
	//ƥ�䲻�ɹ�
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000401\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
		return ;
	}

	sprintf(SendData,"APS110015000400");
	packlength = 15;
	//ƴ����Ҫ���͵ı���
	if(request_type == '0')
	{//���һ��
		SendData[packlength++] = '0';
		SendData[packlength++] = '0';
		
		read_weekly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '1')
	{//���һ����
		SendData[packlength++] = '0';
		SendData[packlength++] = '1';
		read_monthly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '2')
	{//���һ��
		SendData[packlength++] = '0';
		SendData[packlength++] = '2';
		read_yearly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}

	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength);
}

//05 COMMAND_REGISTERID 				//�����IDע������ 		mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_RegisterID(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100150005%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}

//06 COMMAND_SETTIME						//ʱ����������			mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_SetTime(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100150006%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}

//07 COMMAND_SETWIREDNETWORK		//����������������			mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_SetWiredNetwork(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100150007%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}


//��ȡӲ����Ϣ
void APP_Response_GetECUHardwareStatus(char mapping)
{
	int packlength = 0;
	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110120000800%02d",WIFI_MODULE_TYPE);
		memset(&SendData[17],'0',100);
		SendData[117] = 'E';
		SendData[118] = 'N';
		SendData[119] = 'D';
		SendData[120] = '\n';
		packlength = 121;
	}else
	{
		sprintf(SendData,"APS110015000801\n");
		packlength = 16;
	}
	SendToSocketA(SendData ,packlength);

}


//10 COMMAND_SETWIFIPASSWD			//AP������������			mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_SetWifiPasswd(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	
	//ƴ����Ҫ���͵ı���
	sprintf(SendData,"APS1100150010%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}

//11	AP������������
void APP_Response_GetIDInfo(char mapping,inverter_info *inverter)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	char uid[7];
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110015001100");
		packlength = 15;
		for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)
		{
			
			uid[0] = (curinverter->id[0] - '0')*16+(curinverter->id[1] - '0');
			uid[1] = (curinverter->id[2] - '0')*16+(curinverter->id[3] - '0');
			uid[2] = (curinverter->id[4] - '0')*16+(curinverter->id[5] - '0');
			uid[3] = (curinverter->id[6] - '0')*16+(curinverter->id[7] - '0');
			uid[4] = (curinverter->id[8] - '0')*16+(curinverter->id[9] - '0');
			uid[5] = (curinverter->id[10] - '0')*16+(curinverter->id[11] - '0');
			//printf("%02x%02x%02x%02x%02x%02x\n",uid[0],uid[1],uid[2],uid[3],uid[4],uid[5]);
			memcpy(&SendData[packlength],uid,6);	
			packlength += 6;
		}
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';

		
	}else
	{
		sprintf(SendData,"APS110015001101\n");
		packlength = 16;
	}		
	SendToSocketA(SendData ,packlength);

}
//12	AP������������
void APP_Response_GetTime(char mapping,char *Time)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110032001200%sEND\n",Time);
		packlength = 33;
	}else
	{
		sprintf(SendData,"APS110015001201\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);

}


void APP_Response_FlashSize(char mapping,unsigned int Flashsize)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110000001300");
		packlength = 15;
		//printf("%d\n",Flashsize);
		SendData[packlength++] = (Flashsize/16777216)%256;
		SendData[packlength++] = (Flashsize/65536)%256;
		SendData[packlength++] = (Flashsize/256)%256;
		SendData[packlength++] = Flashsize%256;
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';
		
	}else
	{
		sprintf(SendData,"APS110015001301\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);

}


//14 COMMAND_SETWIREDNETWORK		//����������������			mapping :: 0x00 ƥ��  0x01 ��ƥ��  
void APP_Response_GetWiredNetwork(char mapping,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr)
{
	int packlength = 0;
	char MAC[13] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);	
	if(mapping == 0x00)
	{
		//ƴ����Ҫ���͵ı���
		sprintf(SendData,"APS1100150014%02d\n",mapping);
		packlength = 15;
		if(dhcpStatus == 0)
		{
			SendData[packlength++] = '0';
			SendData[packlength++] = '0';
		}else
		{
			SendData[packlength++] = '0';
			SendData[packlength++] = '1';
		}
		SendData[packlength++] = IPAddr.IP1;
		SendData[packlength++] = IPAddr.IP2;
		SendData[packlength++] = IPAddr.IP3;
		SendData[packlength++] = IPAddr.IP4;
				
		SendData[packlength++] = MSKAddr.IP1;
		SendData[packlength++] = MSKAddr.IP2;
		SendData[packlength++] = MSKAddr.IP3;
		SendData[packlength++] = MSKAddr.IP4;
		
		SendData[packlength++] = GWAddr.IP1;
		SendData[packlength++] = GWAddr.IP2;
		SendData[packlength++] = GWAddr.IP3;
		SendData[packlength++] = GWAddr.IP4;

		SendData[packlength++] = DNS1Addr.IP1;
		SendData[packlength++] = DNS1Addr.IP2;
		SendData[packlength++] = DNS1Addr.IP3;
		SendData[packlength++] = DNS1Addr.IP4;

		SendData[packlength++] = DNS2Addr.IP1;
		SendData[packlength++] = DNS2Addr.IP2;
		SendData[packlength++] = DNS2Addr.IP3;
		SendData[packlength++] = DNS2Addr.IP4;
		sprintf(MAC,"%02x%02x%02x%02x%02x%02x",ecu.MacAddress[0],ecu.MacAddress[1],ecu.MacAddress[2],ecu.MacAddress[3],ecu.MacAddress[4],ecu.MacAddress[5]);
		memcpy(&SendData[packlength],MAC,12);
		packlength += 12;

		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';
		SendToSocketA(SendData ,packlength);
	}else
	{
		sprintf(SendData,"APS110015001401\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
	}
	
}


void APP_Response_SetChannel(unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);
	if(mapflag == 1)
	{
		sprintf(SendData,"APS110015001501\n");
		SendToSocketA(SendData ,16);
	}else{
		sprintf(SendData,"APS110023001500%02x%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);		
		SendToSocketA(SendData ,24);
	}
}

void APP_Response_GetShortAddrInfo(char mapping,inverter_info *inverter)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	char uid[7];
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110015001800");
		packlength = 15;
		SendData[packlength++] = rateOfProgress;
		for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)
		{
			
			uid[0] = (curinverter->id[0] - '0')*16+(curinverter->id[1] - '0');
			uid[1] = (curinverter->id[2] - '0')*16+(curinverter->id[3] - '0');
			uid[2] = (curinverter->id[4] - '0')*16+(curinverter->id[5] - '0');
			uid[3] = (curinverter->id[6] - '0')*16+(curinverter->id[7] - '0');
			uid[4] = (curinverter->id[8] - '0')*16+(curinverter->id[9] - '0');
			uid[5] = (curinverter->id[10] - '0')*16+(curinverter->id[11] - '0');
			memcpy(&SendData[packlength],uid,6);	
			packlength += 6;
			SendData[packlength++] = curinverter->shortaddr/256;
			SendData[packlength++] = curinverter->shortaddr%256;
		}
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';

		
	}else
	{
		sprintf(SendData,"APS110015001801\n");
		packlength = 16;
	}		
	SendToSocketA(SendData ,packlength);

}

void APP_Response_GetECUAPInfo(char mapping,unsigned char connectStatus,char *info)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);

	if(mapping == 0x00)
	{
		if(0 == connectStatus)
		{
			sprintf(SendData,"APS110019002000%1dEND\n",connectStatus);
			packlength = 20;
		}else
		{
			sprintf(SendData,"APS11%04d002000%1d%sEND\n",(strlen(info) + 19),connectStatus,info);
			packlength = (strlen(info) + 20);
			printf("sendData:%s\n",SendData);
		}
		
	}else
	{
		sprintf(SendData,"APS110015002001\n");
		packlength = 16;
	}	
	SendToSocketA(SendData ,packlength);

}


//ECU-RS����WIFI����
void APP_Response_SetECUAPInfo(unsigned char result)
{
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);
	sprintf(SendData,"APS1100150021%02d\n",result);
	SendToSocketA(SendData ,16);
}


void APP_Response_GetECUAPList(char mapping,char *list)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT * INVERTER_PHONE_PER_LEN + INVERTER_PHONE_PER_OTHER);

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS11%04d002200%sEND\n",(strlen(list) + 18),list);
		packlength = (strlen(list) + 19);		
	}else
	{
		sprintf(SendData,"APS110015002201\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);
}

