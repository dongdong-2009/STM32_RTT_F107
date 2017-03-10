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



int WiFi_Open(void);		//��WIFI����
int WiFi_SendData(tcp_address_t address ,char *data ,int length);	//��������
int WiFi_RecvData(int timeout,char *data);//��������
#endif /*__USR_WIFI232_H__ */
