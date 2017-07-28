/*****************************************************************************/
/* File      : phoneServer.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-19 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "phoneServer.h"
#include "rtthread.h"
#include "usr_wifi232.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"
#include "threadlist.h"
#include "wifi_comm.h"
#include "variation.h"
#include "main-thread.h"
#include "file.h"
#include "rtc.h"
#include "version.h"
#include "arch/sys_arch.h"

extern rt_mutex_t usr_wifi_lock;
extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];
	
int ResolveWifiPasswd(char *oldPasswd,int *oldLen,char *newPasswd,int *newLen,char *passwdstring)
{
	*oldLen = (passwdstring[0]-'0')*10+(passwdstring[1]-'0');
	memcpy(oldPasswd,&passwdstring[2],*oldLen);
	*newLen = (passwdstring[2+(*oldLen)]-'0')*10+(passwdstring[3+(*oldLen)]-'0');
	memcpy(newPasswd,&passwdstring[4+*oldLen],*newLen);
	
	return 0;
}	

int ResolveWifiSSID(char *SSID,int *SSIDLen,char *PassWD,int *PassWDLen,char *string)
{
	*SSIDLen = (string[0]-'0')*100+(string[1]-'0')*10+(string[2]-'0');
	memcpy(SSID,&string[3],*SSIDLen);
	*PassWDLen = (string[3+(*SSIDLen)]-'0')*100+(string[4+(*SSIDLen)]-'0')*10+(string[5+(*SSIDLen)]-'0');
	memcpy(PassWD,&string[6+*SSIDLen],*PassWDLen);
	
	return 0;
}	

//����0 ��ʾ��̬IP  ����1 ��ʾ�̶�IP   ����-1��ʾʧ��
int ResolveWired(char *string,IP_t *IPAddr,IP_t *MSKAddr,IP_t *GWAddr,IP_t *DNS1Addr,IP_t *DNS2Addr)
{
	if(string[0] == 0x00)	//��̬IP
	{
		return 0;
	}else if(string[0] == 0x01) //��̬IP
	{
		IPAddr->IP1 = string[1];
		IPAddr->IP2 = string[2];
		IPAddr->IP3 = string[3];
		IPAddr->IP4 = string[4];
		MSKAddr->IP1 = string[5];
		MSKAddr->IP2 = string[6];
		MSKAddr->IP3 = string[7];
		MSKAddr->IP4 = string[8];
		GWAddr->IP1 = string[9];
		GWAddr->IP2 = string[10];
		GWAddr->IP3 = string[11];
		GWAddr->IP4 = string[12];	
		DNS1Addr->IP1 = string[13];	
		DNS1Addr->IP2 = string[14];	
		DNS1Addr->IP3 = string[15];	
		DNS1Addr->IP4 = string[16];	
		DNS2Addr->IP1 = string[17];	
		DNS2Addr->IP2 = string[18];	
		DNS2Addr->IP3 = string[19];	
		DNS2Addr->IP4 = string[20];	
		return 1;
	}else
	{
		return -1;
	}

}
	
//WIFI�¼�����
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	stBaseInfo baseInfo;
	char setTime[15];
	char date[9];
	
	get_ecuid(ecu.id);
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		switch(Command_Id)
		{
			case COMMAND_BASEINFO:						//��ȡ������Ϣ����	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_BASEINFO,WIFI_RecvData);
				memcpy(baseInfo.ECUID,ecu.id,13);											//ECU ID		OK
			
				baseInfo.LifttimeEnergy = (int)(ecu.life_energy*10);				//ECU ��ʷ������		OK
				baseInfo.LastSystemPower = ecu.system_power;			//ECU ��ǰϵͳ����		OK
				baseInfo.GenerationCurrentDay = ecu.today_energy;//ECU ���췢����
			
				memset(baseInfo.LastToEMA,'\0',8);	//ECU ���һ������EMA��ʱ��
				baseInfo.LastToEMA[0] = (ecu.last_ema_time[0] - '0')*16+(ecu.last_ema_time[1] - '0');
				baseInfo.LastToEMA[1] = (ecu.last_ema_time[2] - '0')*16+(ecu.last_ema_time[3] - '0');
				baseInfo.LastToEMA[2] = (ecu.last_ema_time[4] - '0')*16+(ecu.last_ema_time[5] - '0');
				baseInfo.LastToEMA[3] = (ecu.last_ema_time[6] - '0')*16+(ecu.last_ema_time[7] - '0');
				baseInfo.LastToEMA[4] = (ecu.last_ema_time[8] - '0')*16+(ecu.last_ema_time[9] - '0');
				baseInfo.LastToEMA[5] = (ecu.last_ema_time[10] - '0')*16+(ecu.last_ema_time[11] - '0');
				baseInfo.LastToEMA[6] = (ecu.last_ema_time[12] - '0')*16+(ecu.last_ema_time[13] - '0');

				baseInfo.InvertersNum = ecu.total;				//ECU ���������
				baseInfo.LastInvertersNum = ecu.count;		//ECU ��ǰ���ӵ����������
				baseInfo.Length = ECU_VERSION_LENGTH;								//ECU �汾�ų���
				sprintf(baseInfo.Version,"%s_%s_%s",ECU_VERSION,MAJORVERSION,MINORVERSION);	//ECU �汾
				baseInfo.TimeZoneLength = 9;				//ECU ʱ������
				sprintf(baseInfo.TimeZone,"Etc/GMT+8");									//ECU ʱ��
				memset(baseInfo.MacAddress,'\0',7);
				get_mac((unsigned char*)baseInfo.MacAddress);			//ECU ����Mac��ַ									
				APP_Response_BaseInfo(ID,baseInfo);
				break;	
			
			case COMMAND_POWERGENERATION:			//�����������������		OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERGENERATION,WIFI_RecvData);
				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_POWERGENERATION  Mapping\n");
				APP_Response_PowerGeneration(0x00,ID,inverter,ecu.count);

				break;
					
			case COMMAND_POWERCURVE:					//������������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERCURVE,WIFI_RecvData);
				memset(date,'\0',9);
				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_POWERCURVE  Mapping\n");
				memcpy(date,&WIFI_RecvData[28],8);
				APP_Response_PowerCurve(0x01,ID,date);

				break;
						
			case COMMAND_GENERATIONCURVE:			//��������������	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_GENERATIONCURVE,WIFI_RecvData);
				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_GENERATIONCURVE  Mapping\n");
				APP_Response_GenerationCurve(0x00,ID,WIFI_RecvData[28]);
			
				break;
						
			case COMMAND_REGISTERID:					//�����IDע������	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_REGISTERID,WIFI_RecvData);				

				{
					int AddNum = 0;
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_REGISTERID  Mapping\n");
					//����̨��
					AddNum = (Data_Len - 29)/6;
					//���ID���ļ�
					phone_add_inverter(AddNum,&WIFI_RecvData[28]);
					//����main�߳�
					restartThread(TYPE_MAIN);					
					APP_Response_RegisterID(0x00,ID);
				}
				break;
				
			case COMMAND_SETTIME:							//ʱ����������	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETTIME,WIFI_RecvData);
	

				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_SETTIME  Mapping\n");
				memset(setTime,'\0',15);
				memcpy(setTime,&WIFI_RecvData[28],14);
				//����ʱ��
				set_time(setTime);
				APP_Response_SetTime(0x00,ID);

				break;
			
			case COMMAND_SETWIREDNETWORK:			//����������������  OK
				
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIREDNETWORK,WIFI_RecvData);	

				{
					int ModeFlag = 0;
					char buff[200] = {'\0'};
					IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIREDNETWORK  Mapping\n");
					//�����DHCP  ���ǹ̶�IP
					ModeFlag = ResolveWired(&WIFI_RecvData[28],&IPAddr,&MSKAddr,&GWAddr,&DNS1Addr,&DNS2Addr);
					if(ModeFlag == 0x00)		//DHCP
					{
						dhcp_reset();
					}else if (ModeFlag == 0x01)		//�̶�IP		
					{
						//���������ַ
						sprintf(buff,"IPAddr=%d.%d.%d.%d\nMSKAddr=%d.%d.%d.%d\nGWAddr=%d.%d.%d.%d\nDNS1Addr=%d.%d.%d.%d\nDNS2Addr=%d.%d.%d.%d\n",IPAddr.IP1,IPAddr.IP2,IPAddr.IP3,IPAddr.IP4,
										MSKAddr.IP1,MSKAddr.IP2,MSKAddr.IP3,MSKAddr.IP4,GWAddr.IP1,GWAddr.IP2,GWAddr.IP3,GWAddr.IP4,DNS1Addr.IP1,DNS1Addr.IP2,DNS1Addr.IP3,DNS1Addr.IP4,DNS2Addr.IP1,DNS2Addr.IP2,DNS2Addr.IP3,DNS2Addr.IP4);
						//���ù̶�IP
						StaticIP(IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);
					}
					
					APP_Response_SetWiredNetwork(0x00,ID);
				}
				break;
			
			case COMMAND_SETWIFI:							//����������������	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFI,WIFI_RecvData);
				{
					char SSID[100] = {'\0'};
					char Password[100] = {'\0'};
					int SSIDLen,passWDLen;
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIFI  Mapping\n");
					ResolveWifiSSID(SSID,&SSIDLen,Password,&passWDLen,(char *)&WIFI_RecvData[28]);
					APP_Response_SetWifi(0x00,ID);
					WIFI_ChangeSSID(SSID,Password);
					
				}
				break;
			
			case COMMAND_SEARCHWIFISTATUS:		//������������״̬����	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SEARCHWIFISTATUS,WIFI_RecvData);
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SEARCHWIFISTATUS  Mapping\n");
					if(0 == WIFI_ConStatus())
					{
						APP_Response_SearchWifiStatus(0x00,ID);
					}else
					{
						APP_Response_SearchWifiStatus(0x02,ID);
					}
				}
				break;
			
			case COMMAND_SETWIFIPASSWD:				//AP������������     OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFIPASSWD,WIFI_RecvData);
				{
					char OldPassword[100] = {'\0'};
					char NewPassword[100] = {'\0'};
					int oldLen,newLen;
					
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIFIPASSWD  Mapping\n");
					//��ȡ����
					ResolveWifiPasswd(OldPassword,&oldLen,NewPassword,&newLen,(char *)&WIFI_RecvData[28]);
					APP_Response_SetWifiPasswd(0x00,ID);
					WIFI_ChangePasswd(NewPassword);
					
				}
				break;
		}
	}
}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Phone Server Application program entry                                  */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter  unused                                                       */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void phone_server_thread_entry(void* parameter)
{
	char *data = NULL;
	int length = 0;
	unsigned char ID[9] = {'\0'};
	data = malloc(2048);	
	while(1)
	{
		memset(data,0x00,2048);
#ifdef WIFI_USE 	
		rt_mutex_take(usr_wifi_lock, RT_WAITING_FOREVER);
		//Recv socket A data by serial,If the data is received, the phone is sent.
		length = RecvSocketData(SOCKET_A,data,1);
		rt_mutex_release(usr_wifi_lock);
		if(length > 0)
		{			
			memcpy(ID,&data[1],8);
			ID[8] = '\0';
			print2msg(ECU_DBG_WIFI,"phone_server",&data[9]);
			
			//������ĸ����ܺ���		
			process_WIFI(ID,&data[9]);
			
		}
#endif		
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	//free(data);
	
}
