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
#include "usart5.h"
#include "client.h"
#include "myfile.h"
#include <lwip/netdb.h> /* Ϊ�˽�������������Ҫ����netdb.hͷ�ļ� */
#include <lwip/sockets.h> /* ʹ��BSD socket����Ҫ����sockets.hͷ�ļ� */


extern rt_mutex_t wifi_uart_lock;
extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];

int getAddr(MyArray *array, int num,IPConfig_t *IPconfig)
{
	int i;
	ip_addr_t addr;
	for(i=0; i<num; i++){
		memset(&addr,0x00,sizeof(addr));
		if(!strlen(array[i].name))break;
		//IP��ַ
		if(!strcmp(array[i].name, "IPAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->IPAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->IPAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->IPAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->IPAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//�����ַ
		else if(!strcmp(array[i].name, "MSKAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->MSKAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->MSKAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->MSKAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->MSKAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//���ص�ַ
		else if(!strcmp(array[i].name, "GWAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->GWAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->GWAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->GWAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->GWAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS1��ַ
		else if(!strcmp(array[i].name, "DNS1Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS1Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS1Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS1Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS1Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS2��ַ
		else if(!strcmp(array[i].name, "DNS2Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS2Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS2Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS2Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS2Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}	
	}
	return 0;
}

	
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
	if(string[1] == '0')	//��̬IP
	{
		return 0;
	}else if(string[1] == '1') //��̬IP
	{
		IPAddr->IP1 = string[2];
		IPAddr->IP2 = string[3];
		IPAddr->IP3 = string[4];
		IPAddr->IP4 = string[5];
		MSKAddr->IP1 = string[6];
		MSKAddr->IP2 = string[7];
		MSKAddr->IP3 = string[8];
		MSKAddr->IP4 = string[9];
		GWAddr->IP1 = string[10];
		GWAddr->IP2 = string[11];
		GWAddr->IP3 = string[12];
		GWAddr->IP4 = string[13];	
		DNS1Addr->IP1 = string[14];	
		DNS1Addr->IP2 = string[15];	
		DNS1Addr->IP3 = string[16];	
		DNS1Addr->IP4 = string[17];	
		DNS2Addr->IP1 = string[18];	
		DNS2Addr->IP2 = string[19];	
		DNS2Addr->IP3 = string[20];	
		DNS2Addr->IP4 = string[21];	
		return 1;
	}else
	{
		return -1;
	}

}
	
//WIFI�¼�����
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
#ifdef WIFI_USE
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	stBaseInfo baseInfo;
	char setTime[15];
	char date[9];
	
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
				baseInfo.GenerationCurrentDay = (unsigned int)(ecu.today_energy * 100);//ECU ���췢����
				
				printf("%s\n",ecu.last_ema_time);
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
					
			case COMMAND_POWERCURVE:					//������������   OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERCURVE,WIFI_RecvData);
				memset(date,'\0',9);
				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_POWERCURVE  Mapping\n");
				memcpy(date,&WIFI_RecvData[28],8);
				APP_Response_PowerCurve(0x00,ID,date);

				break;
						
			case COMMAND_GENERATIONCURVE:			//��������������	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_GENERATIONCURVE,WIFI_RecvData);
				//ƥ��ɹ�������Ӧ����
				printf("COMMAND_GENERATIONCURVE  Mapping\n");
				APP_Response_GenerationCurve(0x00,ID,WIFI_RecvData[29]);
			
				break;
						
			case COMMAND_REGISTERID:					//�����IDע������	OK
				printf("WIFI_Recv_Event%d %s\n",COMMAND_REGISTERID,WIFI_RecvData);				

				{
					int AddNum = 0;
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_REGISTERID  Mapping\n");
					//����̨��
					AddNum = (Data_Len - 31)/12;
					printf("AddNum:%d\n",AddNum);
					APP_Response_RegisterID(0x00,ID);
					//���ID���ļ�
					phone_add_inverter(AddNum,&WIFI_RecvData[28]);
					
					//����main�߳�
					restartThread(TYPE_MAIN);					
					
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
						printmsg(ECU_DBG_WIFI,"dynamic IP");
						dhcp_reset();
					}else if (ModeFlag == 0x01)		//�̶�IP		
					{
						printmsg(ECU_DBG_WIFI,"static IP");
						//���������ַ
						sprintf(buff,"IPAddr=%d.%d.%d.%d\nMSKAddr=%d.%d.%d.%d\nGWAddr=%d.%d.%d.%d\nDNS1Addr=%d.%d.%d.%d\nDNS2Addr=%d.%d.%d.%d\n",IPAddr.IP1,IPAddr.IP2,IPAddr.IP3,IPAddr.IP4,
										MSKAddr.IP1,MSKAddr.IP2,MSKAddr.IP3,MSKAddr.IP4,GWAddr.IP1,GWAddr.IP2,GWAddr.IP3,GWAddr.IP4,DNS1Addr.IP1,DNS1Addr.IP2,DNS1Addr.IP3,DNS1Addr.IP4,DNS2Addr.IP1,DNS2Addr.IP2,DNS2Addr.IP3,DNS2Addr.IP4);
						echo("/yuneng/staticIP.con",buff);
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
						APP_Response_SearchWifiStatus(0x01,ID);
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
	#endif
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
	MyArray array[5];
	int fileflag = 0; 
	IPConfig_t IPconfig;

	get_ecuid(ecu.id);
	readconnecttime();
	
	
		
	//3?��??��IP
	fileflag = file_get_array(array, 5, "/yuneng/staticIP.con");
	if(fileflag == 0)
	{
		getAddr(array, 5,&IPconfig);
		StaticIP(IPconfig.IPAddr,IPconfig.MSKAddr,IPconfig.GWAddr,IPconfig.DNS1Addr,IPconfig.DNS2Addr);
	}

	
	while(1)
	{	
		//����
		rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
		//��ȡWIFI�¼�
		WIFI_GetEvent();
		//����
		rt_mutex_release(wifi_uart_lock);
		
		if(WIFI_Recv_SocketA_Event == 1)
		{
			print2msg(ECU_DBG_WIFI,"phone_server",(char *)WIFI_RecvSocketAData);
			WIFI_Recv_SocketA_Event = 0;
	
			process_WIFI(ID_A,(char *)WIFI_RecvSocketAData);
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND/2);
	}
	
}
