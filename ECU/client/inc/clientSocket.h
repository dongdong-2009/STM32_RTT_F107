#ifndef __CLIENT_SOCKET_H__
#define __CLIENT_SOCKET_H__

int writeconnecttime(void);			//�������һ�������Ϸ�������ʱ��
void showconnected(void);		//������EMA
void showdisconnected(void);		//�޷�����EMA	

int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);

#endif /*__CLIENT_SOCKET_H__*/
