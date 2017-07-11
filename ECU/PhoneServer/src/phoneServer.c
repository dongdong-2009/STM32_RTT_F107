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

extern rt_mutex_t usr_wifi_lock;
extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];
	
//WIFI�¼�����
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	stBaseInfo baseInfo;
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		switch(Command_Id)
		{
			case COMMAND_BASEINFO:						//��ȡ������Ϣ����
				printf("WIFI_Recv_Event%d %s\n",COMMAND_BASEINFO,WIFI_RecvData);
//					memcpy(baseInfo.ECUID,ecu.id,13);											//ECU ID
//	baseInfo.LifttimeEnergy = (int)(ecu.life_energy*10);				//ECU ��ʷ������
//	unsigned short LastSystemPower;			//ECU ��ǰϵͳ����
//	unsigned short GenerationCurrentDay;//ECU ���췢����
//	char LastToEMA[8];									//ECU ���һ������EMA��ʱ��
//	unsigned short InvertersNum;				//ECU ���������
//	unsigned short LastInvertersNum;		//ECU ��ǰ���ӵ����������
//	unsigned char Length;								//ECU �汾�ų���
//	char Version[20];										//ECU �汾
//	unsigned char TimeZoneLength;				//ECU ʱ������
//	char TimeZone[20];									//ECU ʱ��
//	char MacAddress[7];									//ECU ����Mac��ַ
//	char WifiMac[7];										//ECU ����Mac��ַ
				APP_Response_BaseInfo(ID,baseInfo);
				break;	
			
			case COMMAND_POWERGENERATION:			//�����������������	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERGENERATION,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_POWERGENERATION  Mapping\n");
					APP_Response_PowerGeneration(0x00,ID,inverter,ecu.count);
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					printf("COMMAND_POWERGENERATION   Not Mapping");
					APP_Response_PowerGeneration(0x01,ID,inverter,ecu.count);
				}
				break;
					
			case COMMAND_POWERCURVE:					//������������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERCURVE,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_POWERCURVE  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_POWERCURVE   Not Mapping");
				}
				break;
						
			case COMMAND_GENERATIONCURVE:			//��������������	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_GENERATIONCURVE,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_GENERATIONCURVE  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_GENERATIONCURVE   Not Mapping");
				}
			
				break;
						
			case COMMAND_REGISTERID:					//�����IDע������	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_REGISTERID,WIFI_RecvData);				
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_REGISTERID  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_REGISTERID   Not Mapping");
				}
				break;
				
			case COMMAND_SETTIME:							//ʱ����������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETTIME,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETTIME  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_SETTIME   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIREDNETWORK:			//����������������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIREDNETWORK,WIFI_RecvData);	
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIREDNETWORK  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_SETWIREDNETWORK   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIFI:							//����������������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFI,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIFI  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_SETWIFI   Not Mapping");
				}
				break;
			
			case COMMAND_SEARCHWIFISTATUS:		//������������״̬����
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SEARCHWIFISTATUS,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SEARCHWIFISTATUS  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_SEARCHWIFISTATUS   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIFIPASSWD:				//AP������������
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFIPASSWD,WIFI_RecvData);
				//���ȶԱ�ECU ID�Ƿ�ƥ��
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//ƥ��ɹ�������Ӧ����
					printf("COMMAND_SETWIFIPASSWD  Mapping\n");
				}else
				{	
					//��ƥ�䣬���� ƥ��ʧ�ܱ���
					
					printf("COMMAND_SETWIFIPASSWD   Not Mapping");
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
