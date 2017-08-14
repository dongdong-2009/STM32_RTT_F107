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


void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 				//��ȡ������Ϣ����
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer); 	//��ȡ���������������
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡ��������
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡ����������
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//�����IDע��
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//ECUʱ������
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//������������
void Phone_SetWIFI(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//������������
void Phone_SearchWIFIStatus(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//������������״̬
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//AP��������
void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡID��Ϣ
void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//��ȡʱ��


void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
