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
enum CommandID{
	P0000, P0001, P0002, P0003, P0004, P0005, P0006, P0007, P0008, P0009, //0-9
	P0010, P0011, P0012, P0013, P0014, P0015, P0016, P0017, P0018, P0019, //10-19
	P0020, P0021, P0022, P0023, P0024, P0025, P0026, P0027, P0028, P0029, //20-29
	P0030, P0031, P0032, P0033, P0034, P0035, P0036, P0037, P0038, P0039, //30-39
	P0040, P0041, P0042, P0043, P0044, P0045, P0046, P0047, P0048, P0049, //40-49
	P0050, P0051, P0052, P0053, P0054, P0055, P0056, P0057, P0058, P0059, //50-59
};


void (*pfun_Phone[100])(unsigned char * ID,int Data_Len,const char *recvbuffer);

void add_Phone_functions(void)
{
  pfun_Phone[P0001] = Phone_GetBaseInfo; 				//��ȡ������Ϣ����
  pfun_Phone[P0002] = Phone_GetGenerationData; 	//��ȡ���������������
	pfun_Phone[P0003] = Phone_GetPowerCurve; 			//��ȡ��������
	pfun_Phone[P0004] = Phone_GetGenerationCurve; 			//��ȡ����������
	pfun_Phone[P0005] = Phone_RegisterID; 			//�����IDע��
	pfun_Phone[P0006] = Phone_SetTime; 			//ECUʱ������
	pfun_Phone[P0007] = Phone_SetWiredNetwork; 			//������������
	pfun_Phone[P0008] = Phone_SetWIFI; 			//������������
	pfun_Phone[P0009] = Phone_SearchWIFIStatus; 			//������������״̬
	pfun_Phone[P0010] = Phone_SetWIFIPasswd; 			//AP��������
	pfun_Phone[P0011] = Phone_GetIDInfo; 			//��ȡID��Ϣ
	pfun_Phone[P0012] = Phone_GetTime; 			//��ȡʱ��

}


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

int ResolveWifiSSID(char *SSID,int *SSIDLen,char *Auth,char *Encry,char *PassWD,int *PassWDLen,char *string)
{
	*SSIDLen = (string[0]-'0')*100+(string[1]-'0')*10+(string[2]-'0');
	memcpy(SSID,&string[3],*SSIDLen);
	*Auth = string[3+(*SSIDLen)];
	*Encry = string[4+(*SSIDLen)];
	*PassWDLen = (string[5+(*SSIDLen)]-'0')*100+(string[6+(*SSIDLen)]-'0')*10+(string[7+(*SSIDLen)]-'0');
	memcpy(PassWD,&string[8+*SSIDLen],*PassWDLen);
	return 0;
}	

//����0 ��ʾ��̬IP  ����1 ��ʾ�̶�IP   ����-1��ʾʧ��
int ResolveWired(const char *string,IP_t *IPAddr,IP_t *MSKAddr,IP_t *GWAddr,IP_t *DNS1Addr,IP_t *DNS2Addr)
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
	
 
void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer) 				//��ȡ������Ϣ����
{
	stBaseInfo baseInfo;
	printf("WIFI_Recv_Event%d %s\n",P0001,recvbuffer);
			
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
	memcpy(baseInfo.MacAddress,ecu.MacAddress,7);//ECU ����Mac��ַ										
	APP_Response_BaseInfo(ID,baseInfo);
}
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer) 	//��ȡ���������������
{
	printf("WIFI_Recv_Event%d %s\n",P0002,recvbuffer);
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_POWERGENERATION  Mapping\n");
	APP_Response_PowerGeneration(0x00,ID,inverter,ecu.count);
}
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//��ȡ��������
{
	char date[9];
	printf("WIFI_Recv_Event%d %s\n",P0003,recvbuffer);
	memset(date,'\0',9);
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_POWERCURVE  Mapping\n");
	memcpy(date,&recvbuffer[28],8);
	APP_Response_PowerCurve(0x00,ID,date);

}
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//��ȡ����������
{
	printf("WIFI_Recv_Event%d %s\n",P0004,recvbuffer);
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_GENERATIONCURVE  Mapping\n");
	APP_Response_GenerationCurve(0x00,ID,recvbuffer[29]);
}
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//�����IDע��
{
	printf("WIFI_Recv_Event%d %s\n",P0005,recvbuffer);				

	{
	int AddNum = 0;
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_REGISTERID  Mapping\n");
	//����̨��
	AddNum = (Data_Len - 31)/6;
	printf("AddNum:%d\n",AddNum);
	APP_Response_RegisterID(0x00,ID);
	//���ID���ļ�
	phone_add_inverter(AddNum,&recvbuffer[28]);
					
	//����main�߳�
	restartThread(TYPE_MAIN);					
					
	}
}
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//ECUʱ������
{
	char setTime[15];
	printf("WIFI_Recv_Event%d %s\n",P0006,recvbuffer);
	
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_SETTIME  Mapping\n");
	memset(setTime,'\0',15);
	memcpy(setTime,&recvbuffer[28],14);
	//����ʱ��
	set_time(setTime);
	APP_Response_SetTime(0x00,ID);

}
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer)			//������������
{
	printf("WIFI_Recv_Event%d %s\n",P0007,recvbuffer);	

	{
	int ModeFlag = 0;
	char buff[200] = {'\0'};
	IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_SETWIREDNETWORK  Mapping\n");
	//�����DHCP  ���ǹ̶�IP
	ModeFlag = ResolveWired(&recvbuffer[28],&IPAddr,&MSKAddr,&GWAddr,&DNS1Addr,&DNS2Addr);
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
}
void Phone_SetWIFI(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//������������
{
	printf("WIFI_Recv_Event%d %s\n",P0008,recvbuffer);
	{
	char SSID[100] = {'\0'};
	char Password[100] = {'\0'};
	char Auth;
	char Encry;
	int SSIDLen,passWDLen;
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_SETWIFI  Mapping\n");
	ResolveWifiSSID(SSID,&SSIDLen,&Auth,&Encry,Password,&passWDLen,(char *)&recvbuffer[28]);
	APP_Response_SetWifi(0x00,ID);
	WIFI_ChangeSSID(SSID,Auth,Encry,Password,passWDLen);
					
	}	
}
void Phone_SearchWIFIStatus(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//������������״̬
{
	printf("WIFI_Recv_Event%d %s\n",P0009,recvbuffer);
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
}
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//AP��������
{
	printf("WIFI_Recv_Event%d %s\n",P0010,recvbuffer);
	{
	char OldPassword[100] = {'\0'};
	char NewPassword[100] = {'\0'};
	int oldLen,newLen;
					
	//ƥ��ɹ�������Ӧ����
	printf("COMMAND_SETWIFIPASSWD  Mapping\n");
	//��ȡ����
	ResolveWifiPasswd(OldPassword,&oldLen,NewPassword,&newLen,(char *)&recvbuffer[28]);
	APP_Response_SetWifiPasswd(0x00,ID);
	WIFI_ChangePasswd(NewPassword);
					
	}	
}


void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//��ȡID��Ϣ
{
	int i = 0;
	inverter_info *curinverter = inverter;
	printf("WIFI_Recv_Event%d %s\n",P0011,recvbuffer);
	
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		rt_memset(curinverter->id, '\0', sizeof(curinverter->id));		//��������UID
		//rt_memset(curinverter->tnuid, '\0', sizeof(curinverter->tnuid));			//��������ID

		curinverter->model = 0;

		curinverter->dv=0;			//��յ�ǰһ��ֱ����ѹ
		curinverter->di=0;			//��յ�ǰһ��ֱ������
		curinverter->op=0;			//��յ�ǰ������������
		curinverter->gf=0;			//��յ���Ƶ��
		curinverter->it=0;			//���������¶�
		curinverter->gv=0;			//��յ�����ѹ
		curinverter->dvb=0;			//B·��յ�ǰһ��ֱ����ѹ
		curinverter->dib=0;			//B·��յ�ǰһ��ֱ������
		curinverter->opb=0;			//B·��յ�ǰ������������
		curinverter->gvb=0;
		curinverter->dvc=0;
		curinverter->dic=0;
		curinverter->opc=0;
		curinverter->gvc=0;
		curinverter->dvd=0;
		curinverter->did=0;
		curinverter->opd=0;
		curinverter->gvd=0;



		curinverter->curgeneration = 0;	//����������ǰһ�ַ�����
		curinverter->curgenerationb = 0;	//B·��յ�ǰһ�ַ�����

		curinverter->preaccgen = 0;
		curinverter->preaccgenb = 0;
		curinverter->curaccgen = 0;
		curinverter->curaccgenb = 0;
		curinverter->preacctime = 0;
		curinverter->curacctime = 0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//��������״̬
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));		//��������״̬
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));		//B·��������״̬

		curinverter->dataflag = 0;		//��һ�������ݵı�־��λ
	//	curinverter->bindflag=0;		//���������־λ����0
		curinverter->no_getdata_num=0;	//ZK,���������ȡ�����Ĵ���
		curinverter->disconnect_times=0;		//û���������ͨ���ϵĴ�����0, ZK
		curinverter->signalstrength=0;			//�ź�ǿ�ȳ�ʼ��Ϊ0

		curinverter->updating=0;
		curinverter->raduis=0;
	}
	
	get_id_from_file(inverter);
	APP_Response_GetIDInfo(0x00,ID,inverter);
}

void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//��ȡʱ��
{
	char Time[15] = {'\0'};
	printf("WIFI_Recv_Event%d %s\n",P0012,recvbuffer);
	apstime(Time);
	Time[14] = '\0';
	APP_Response_GetTime(0x00,ID,Time);

}

//WIFI�¼�����
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
#ifdef WIFI_USE
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		(*pfun_Phone[Command_Id])(ID,Data_Len,WIFI_RecvData);
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
	get_mac((unsigned char*)ecu.MacAddress);			//ECU ����Mac��ַ
	readconnecttime();
	
	add_Phone_functions();
		
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
		
		rt_thread_delay(RT_TICK_PER_SECOND/8);
	}
	
}
