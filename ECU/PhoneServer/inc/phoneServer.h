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

void Phone_SetChannel(int Data_Len,const char *recvbuffer);

void Phone_GetShortAddrInfo(int Data_Len,const char *recvbuffer);

void APP_GetECUAPInfo(int Data_Len,const char *recvbuffer) ;			//��ȡECU����AP��Ϣ
void APP_SetECUAPInfo(int Data_Len,const char *recvbuffer); 			//����ECU����AP
void APP_ListECUAPInfo(int Data_Len,const char *recvbuffer); 
//Phone Server�߳�
void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
