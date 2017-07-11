#ifndef __USR_WIFI232_H__
#define __USR_WIFI232_H__
/*****************************************************************************/
/* File      : usr_wifi232.h                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-01 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_STATUS  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

typedef enum 
{
	TYPE_IP,			//��ַ����ΪIP
	TYPE_DOMAIN,	//��ַ����Ϊ����
	
}tcp_address_type;

typedef struct 
{
	tcp_address_type address_type; //ʹ�õ�ַ�����ͣ�IP�������� 
	union AddressData
  {
		char ip[4];				//�����IP���ò��ָ�ֵ
		char domain[20];	//������������ò��ָ�ֵ
	}address;
	unsigned short port;	//�˿ں�
}tcp_address_t;

typedef enum 
{
	SOCKET_A = 1,
	SOCKET_B = 2,
	SOCKET_C = 3,
}SocketType;

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

//��WIFI���� ��ʹ��WIFI��ʱ�����򿪴���
int WiFi_Open(void);		
//���WIFI���ڻ�����������
void clear_WIFI(void);
//��������   �÷����ݸ�ʽ�ڴ���ָ��ģʽ
int WiFi_SendData(tcp_address_t address ,char *data ,int length);
//�Ӵ��ڽ�������
int WiFi_RecvData(int timeout,char *data);
// ����ATģʽ
int AT(void);

/********************ATģʽ����ص�����*********************************/
// ����ԭ����ģʽ  
int AT_ENTM(void);
//��ȡUSRģ������汾������
int AT_VER(void);
//ģ�鸴λ����
int AT_Z(void);
//ģ��ָ�����������
int AT_RELD(void);
//����SOCKET A����ع���   TCP������ģʽ
int AT_NETP(char *IP,int port);

//��SOCKET B����
int AT_TCPB_ON(void);
//����SOCKET B�ķ�����IP��ַ
int AT_TCPADDB(char *IP);
//����SOCKET B�ķ������˿ں�
int AT_TCPPTB(int port);
//����SOCKET B�ĳ�ʱʱ��
int AT_TCPTOB(int timeout);

//��SOCKET C����
int AT_TCPC_ON(void);
//����SOCKET C�ķ�����IP��ַ
int AT_TCPADDC(char *IP);
//����SOCKET C�ķ������˿ں�
int AT_TCPPTC(int port);
//����SOCKET C�ĳ�ʱʱ��
int AT_TCPTOC(int timeout);

//����AP+STA����ģʽ
int AT_FAPSTA_ON(void);
//����WIFI����ģʽ  STA or AP
int AT_WMODE(char *WMode);
//��������·����SSID
int AT_WSSSID(char *SSSID);
//��������·����KEY
int AT_WSKEY(char *SKEY);
//��ʼ��WIFIģ��Ϊ����ģʽ  //����ģʽ socketB ��socketCΪ client��Control_Client������������Ϣ
int initWorkIP(char *clientIP,int clientPort,char *controlIP,int controlPort);

/***********************************************************************/

/*******************��ATģʽ����صĲ���********************************/
int WIFI_Create(SocketType Type);
int WIFI_Close(SocketType Type);
int WIFI_QueryStatus(SocketType Type);
int WIFI_QueryMac(void);
int SendToSocketA(char *data ,int length,char ID[8]);
int SendToSocketB(char *data ,int length);
int SendToSocketC(char *data ,int length);
int RecvSocketData(SocketType Type,char *data,int timeout);
/***********************************************************************/

#endif /*__USR_WIFI232_H__ */
