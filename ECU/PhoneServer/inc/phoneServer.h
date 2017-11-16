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
void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 				//��ȡ������Ϣ����
//��ȡ����������
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer); 	//��ȡ���������������
//��ȡ��������
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡ��������
//��ȡ����������
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡ����������
//ע��ID
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//�����IDע��
//����ʱ��
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//ECUʱ������
//������������
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//������������
//��ȡӲ����Ϣ
void Phone_GetECUHardwareStatus(unsigned char * ID,int Data_Len,const char *recvbuffer);
//����WIFI AP ����
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//AP��������
//��ȡID��Ϣ
void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡID��Ϣ
//��ȡʱ��
void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡʱ��
//��ȡFlash�ռ�
void Phone_FlashSize(unsigned char * ID,int Data_Len,const char *recvbuffer);
//��ȡ����������Ϣ
void Phone_GetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);			//������������

void Phone_SetChannel(unsigned char * ID,int Data_Len,const char *recvbuffer);

void Phone_GetShortAddrInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
//Phone Server�߳�
void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
