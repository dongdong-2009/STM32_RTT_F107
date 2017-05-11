#ifndef __USR_WIFI232_H__
#define __USR_WIFI232_H__

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
//��SOCKET C����
int AT_TCPC_ON(void);
//����SOCKET C�ķ�����IP��ַ
int AT_TCPADDC(char *IP);
//����SOCKET C�ķ������˿ں�
int AT_TCPPTC(int port);
//����AP+STA����ģʽ
int AT_FAPSTA_ON(void);
//����WIFI����ģʽ  STA or AP
int AT_WMODE(char *WMode);
//��������·����SSID
int AT_WSSSID(char *SSSID);
//��������·����KEY
int AT_WSKEY(char *SKEY);

/***********************************************************************/

int SendToSocketB(char *data ,int length);
int SendToSocketC(char *data ,int length);

#endif /*__USR_WIFI232_H__ */
