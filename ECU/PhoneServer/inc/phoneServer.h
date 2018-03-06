#ifndef __PHONESERVER_H__
#define	__PHONESERVER_H__
/*****************************************************************************/
/* File      : phoneServer.h                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-19 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
typedef enum  {
  SERVER_UPDATE_GET = 1,
  SERVER_CLIENT_GET = 2,
  SERVER_CONTROL_GET = 3,
  SERVER_UPDATE_SET = 4,
  SERVER_CLIENT_SET = 5,
  SERVER_CONTROL_SET = 6
}eServerCmdType;
typedef struct ECUServerInfo {
	eServerCmdType serverCmdType;
	char domain[100];
	unsigned char IP[4];
	unsigned short Port1;
	unsigned short Port2;
		  
} ECUServerInfo_t;

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

//��ȡ������Ϣ
void Phone_GetBaseInfo(int Data_Len,const char *recvbuffer); 				//��ȡ������Ϣ����
//��ȡ����������
void Phone_GetGenerationData(int Data_Len,const char *recvbuffer); 	//��ȡ���������������
//��ȡ��������
void Phone_GetPowerCurve(int Data_Len,const char *recvbuffer); 			//��ȡ��������
//��ȡ����������
void Phone_GetGenerationCurve(int Data_Len,const char *recvbuffer); 			//��ȡ����������
//ע��ID
void Phone_RegisterID(int Data_Len,const char *recvbuffer); 			//�����IDע��
//����ʱ��
void Phone_SetTime(int Data_Len,const char *recvbuffer); 			//ECUʱ������
//������������
void Phone_SetWiredNetwork(int Data_Len,const char *recvbuffer); 			//������������
//��ȡӲ����Ϣ
void Phone_GetECUHardwareStatus(int Data_Len,const char *recvbuffer);
//����WIFI AP ����
void Phone_SetWIFIPasswd(int Data_Len,const char *recvbuffer); 			//AP��������
//��ȡID��Ϣ
void Phone_GetIDInfo(int Data_Len,const char *recvbuffer); 			//��ȡID��Ϣ
//��ȡʱ��
void Phone_GetTime(int Data_Len,const char *recvbuffer); 			//��ȡʱ��
//��ȡFlash�ռ�
void Phone_FlashSize(int Data_Len,const char *recvbuffer);
//��ȡ����������Ϣ
void Phone_GetWiredNetwork(int Data_Len,const char *recvbuffer);			//������������
//�����ŵ�
void Phone_SetChannel(int Data_Len,const char *recvbuffer);
//��ȡ�̵�ַ
void Phone_GetShortAddrInfo(int Data_Len,const char *recvbuffer);
//��ȡECU����AP��Ϣ��   ������ESP07S
void Phone_GetECUAPInfo(int Data_Len,const char *recvbuffer) ;	
//����ECU����AP��   ������ESP07S
void Phone_SetECUAPInfo(int Data_Len,const char *recvbuffer); 	
//��ȡECU������AP�б�������ESP07S
void Phone_ListECUAPInfo(int Data_Len,const char *recvbuffer); 
void Phone_GetFunctionStatusInfo(int Data_Len,const char *recvbuffer);
void Phone_ServerInfo(int Data_Len,const char *recvbuffer);
//Phone Server�߳�
void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
