/*****************************************************************************/
/* File      : usart5.c                                                      */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "usart5.h"	
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"
#include "stm32f10x.h"
#include "rthw.h"
#include "stdio.h"
#include "rtthread.h"
#include "stdlib.h"
#include "debug.h"
#include "client.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_RCC                    RCC_APB2Periph_GPIOC
#define WIFI_GPIO                   GPIOC
#define WIFI_PIN                    (GPIO_Pin_6)

rt_mutex_t wifi_uart_lock = RT_NULL;
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

void delayMS(unsigned int ms)
{
	rt_hw_ms_delay(ms);
}

//4���ֽ�
unsigned short packetlen_A(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//2���ֽ�
unsigned short packetlen_B(unsigned char *packet)
{
	unsigned short len = 0,count = 0;
	count = (packet[0] - '0') * 10 + (packet[1] - '0');
	len =  3 + 9 + count *14;
	return len;
	
}

//5���ֽ�
unsigned short packetlen_C(unsigned char *packet)
{
	unsigned short len = 0;
	int i = 0;
	for(i = 0;i < 5;i++)
	{
		if(packet[i] == 'A') packet[i] = '0';
	}
	len = ((packet[0]-'0')*10000 +(packet[1]-'0')*1000 + (packet[2]-'0')*100 + (packet[3]-'0')*10 + (packet[4]-'0'));
	return len;
}

//WIFI���ͺ��� 
int WIFI_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); 
    USART_SendData(UART5,(uint16_t)ch);
	}
	return index;
}

//��ʼ��IO ����5
//bound:������
void uart5_init(u32 bound){
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ�ܴ���ʱ��UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);	//ʹ��GPIOAʱ��

 	//USART_DeInit(UART5);  //��λ����5
	 //UART5_TX   PC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PC12
   
    //UART5_RX	  PD.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //��ʼ��PD2
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ

    USART_Init(UART5, &USART_InitStructure); //��ʼ������

   //UART5 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ��� 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	

}

unsigned char ID[9] = {'\0'};

//WIFI  socket A �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketAData[SOCKETA_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketA_Event = 0;
unsigned int WIFI_Recv_SocketA_LEN =0;
unsigned char ID_A[9] = {'\0'};
	
//WIFI  socket B �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketBData[SOCKETB_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketB_Event = 0;
unsigned int WIFI_Recv_SocketB_LEN =0;

//WIFI  socket C �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketCData[SOCKETC_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketC_Event = 0;
unsigned int WIFI_Recv_SocketC_LEN =0;

//wifi  ���ڵ�ǰ�յ������� 
unsigned char USART_RX_BUF[USART_REC_LEN];     			//���ջ���,���USART_REC_LEN���ֽ�.
unsigned short Cur = 0;															//��ǰ��ֵλ��
unsigned short PackLen = 0;
eRecvSM eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;	//���ݲɼ�״̬��
eRecvType eTypeMachine = EN_RECV_TYPE_UNKNOWN;     	//
unsigned short pos = 0;															//���ݽ���λ��

void UART5_IRQHandler(void)                	//����1�жϷ������
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(UART5);//(UART5->DR);	//��ȡ���յ�������
		//SEGGER_RTT_printf(0, "[%d] : %x %c\n",Cur,USART_RX_BUF[Cur],USART_RX_BUF[Cur]);
		Cur +=1;
		if(Cur >=USART_REC_LEN)
		{
			Cur = 0;
		}
	}
} 

void WIFI_GetEvent(void)
{
	  pos = 0;
		
		//����A����ͷ��
		if(eStateMachine == EN_RECV_ST_GET_SCOKET_HEAD)
		{
			while(pos < Cur)
      {
				if(1 == pos)   //'a'
				{
						if((USART_RX_BUF[0] != 'a') && (USART_RX_BUF[0] != 'b') && (USART_RX_BUF[0] != 'c'))
						{
							Cur = 0;
							pos = 0;
							eTypeMachine = EN_RECV_TYPE_UNKNOWN;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}else if(USART_RX_BUF[0] == 'a')
						{
							//SEGGER_RTT_printf(0, "a %d\n",eTypeMachine);
							delayMS(2);
							eStateMachine = EN_RECV_ST_GET_SCOKET_ID;
							eTypeMachine = EN_RECV_TYPE_A;
							break;
						}else if(USART_RX_BUF[0] == 'b')
						{
							delayMS(2);
							eTypeMachine = EN_RECV_TYPE_B;
							//SEGGER_RTT_printf(0, "b %d\n",eTypeMachine);
							eStateMachine = EN_RECV_ST_GET_SCOKET_ID;
						}else if(USART_RX_BUF[0] == 'c')
						{
							delayMS(2);
							eTypeMachine = EN_RECV_TYPE_C;
							//SEGGER_RTT_printf(0, "c %d\n",eTypeMachine);
							eStateMachine = EN_RECV_ST_GET_SCOKET_ID;
						}
				}
				pos++;
			}
		}		
	
		//����ID
		if(eStateMachine == EN_RECV_ST_GET_SCOKET_ID)
		{
      while(pos < Cur)
      {
				if(2 == pos)
				{
					ID[0] = USART_RX_BUF[1];
				}
				
				if(3 == pos)
				{
					ID[1] = USART_RX_BUF[2];
				}	
				
				if(4 == pos)
				{
					ID[2] = USART_RX_BUF[3];
				}
				
				if(5 == pos) 
				{
					ID[3] = USART_RX_BUF[4];
				}
				
				if(6 == pos)
				{
					ID[4] = USART_RX_BUF[5];
				}
				
				if(7 == pos)
				{
					ID[5] = USART_RX_BUF[6];
				}
				
				if(8 == pos)   
				{
					ID[6] = USART_RX_BUF[7];
				}
				
				if(9 == pos)   //���հ汾�����
				{
					//SEGGER_RTT_printf(0, "eTypeMachine %d\n",eTypeMachine);
					ID[7] = USART_RX_BUF[8];
					if(eTypeMachine == EN_RECV_TYPE_A)
					{
						//SEGGER_RTT_printf(0, "ID A\n");
						eStateMachine = EN_RECV_ST_GET_A_HEAD;
					}
					else if(eTypeMachine == EN_RECV_TYPE_B)
					{
						//SEGGER_RTT_printf(0, "ID B\n");
						eStateMachine = EN_RECV_ST_GET_B_HEAD;
					}
					else if(eTypeMachine == EN_RECV_TYPE_C)
					{
						//SEGGER_RTT_printf(0, "ID C\n");
						eStateMachine = EN_RECV_ST_GET_C_HEAD;
					}
					else
					{
						Cur = 0;
						pos = 0;
						eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
					}
							
					break;
				}
								
				pos++;
			}
		}			
	
		//SOCKET A
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_A_HEAD)    //���ձ���ͷ��
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_A_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(pos < Cur)
      {

				if(10 == pos)   //'A'
				{
						if(USART_RX_BUF[9] != 'A')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if(11 == pos)   //'P'
				{
						if(USART_RX_BUF[10] != 'P')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}	
				
				if(12 == pos)   //'S'
				{
						if(USART_RX_BUF[11] != 'S')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if(14 == pos)   //���հ汾�����
				{
					//SEGGER_RTT_printf(0, "APS11\n");
					eStateMachine = EN_RECV_ST_GET_A_LEN;
					break;
				}
				
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_A_LEN)
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_LEN\n");
			while(pos < Cur)
      {
				//�ж��Ƿ���a����  ����������жϺ���8���ֽ�
				if(18 == pos)   //�������ݳ��Ƚ���
				{
					PackLen = (packetlen_A(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "LENGTH11111 : %d\n",PackLen);
					//���㳤��
					eStateMachine = EN_RECV_ST_GET_A_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms �򿪶�ʱ��

					break;
				}
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_A_DATA)
		{
			pos = 0;
			while(pos < Cur)
      {

				if((PackLen - 3) == pos)   //�������ݳ��Ƚ���
				{
					eStateMachine = EN_RECV_ST_GET_A_END;
					break;
				}
				pos++;
			}
		}		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_A_END)
		{
			pos = 0;
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END\n");
			while(pos <= Cur)
      {
				
				if((PackLen - 2) == pos)   //'A'
				{
						if(USART_RX_BUF[PackLen - 3] != 'E')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if((PackLen - 1) == pos)   //'P'
				{
						if(USART_RX_BUF[PackLen - 2] != 'N')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}	
				
				if((PackLen) == pos)   //'S'
				{
						if(USART_RX_BUF[PackLen - 1] != 'D')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						//SEGGER_RTT_printf(0, "ID %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
						
						//���Ľ������
						//������ϵ���Ӧ����
						//���ɼ��ɹ������ݸ��Ƶ��ɹ�����
						memset(WIFI_RecvSocketAData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvSocketAData,&USART_RX_BUF[9],(PackLen-9));
						memcpy(ID_A,ID,8);
						//�������ݣ�ȥ���������
						WIFI_Recv_SocketA_LEN = PackLen-9;
						
						WIFI_RecvSocketAData[WIFI_Recv_SocketA_LEN] = '\0';
						WIFI_Recv_SocketA_Event = 1;
						//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvSocketAData);
						eTypeMachine = EN_RECV_TYPE_UNKNOWN;
						eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
						Cur = 0;
						pos = 0;		
						TIM3_Int_Deinit();
						break;
				}
								
				pos++;
			}
		}
		
		//SOCKET B
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_B_HEAD)    //���ձ���ͷ��
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_HEAD\n");
      while(pos < Cur)
      {	
				if(10 == pos)   // 101   14�ֽڵ�ʱ���
				{
					if(USART_RX_BUF[9] != '1')
					{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
					}
					else
					{
						
						eStateMachine = EN_RECV_ST_GET_B_LEN;
						break;
					}
				}
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_B_LEN)
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_LEN\n");
			while(pos < Cur)
      {
				//�ж��Ƿ���a����  ����������жϺ���8���ֽ�
				if(19 == pos)   //�������ݳ��Ƚ���
				{
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_LEN LENGTH11111 :     %s\n",&USART_RX_BUF[19]);
					PackLen = (packetlen_B(&USART_RX_BUF[19])+9);
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_LEN LENGTH11111 : %d\n",PackLen);
					//���㳤��
					eStateMachine = EN_RECV_ST_GET_B_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms �򿪶�ʱ��

					break;
				}
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_B_DATA)
		{
			pos = 0;
			while(pos <= Cur)
      {
				if(PackLen == pos)   //�������ݳ��Ƚ���
				{
					eStateMachine = EN_RECV_ST_GET_B_END;
					break;
				}
				pos++;
			}
		}		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_B_END)
		{
			//���Ľ������
			//������ϵ���Ӧ����
			//���ɼ��ɹ������ݸ��Ƶ��ɹ�����
			memset(WIFI_RecvSocketBData,0x00,USART_REC_LEN);
			memcpy(WIFI_RecvSocketBData,&USART_RX_BUF[9],(PackLen-9));
			//�������ݣ�ȥ���������
			WIFI_Recv_SocketB_LEN = PackLen-9;
			
			WIFI_RecvSocketBData[WIFI_Recv_SocketB_LEN] = '\0';
			WIFI_Recv_SocketB_Event = 1;
			//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvSocketBData);
			eTypeMachine = EN_RECV_TYPE_UNKNOWN;
			eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
			Cur = 0;
			pos = 0;		
			TIM3_Int_Deinit();
		}
		
		//SOCKET C
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_C_HEAD)    //���ձ���ͷ��
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_C_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(pos < Cur)
      {

				if(10 == pos)   //'A'
				{
						if(USART_RX_BUF[9] != 'A')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if(11 == pos)   //'P'
				{
						if(USART_RX_BUF[10] != 'P')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}	
				
				if(12 == pos)   //'S'
				{
						if(USART_RX_BUF[11] != 'S')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if(14 == pos)   //���հ汾�����
				{
					//SEGGER_RTT_printf(0, "APS11\n");
					eStateMachine = EN_RECV_ST_GET_C_LEN;
					break;
				}
				
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_C_LEN)
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_C_LEN\n");
			while(pos < Cur)
      {
				//�ж��Ƿ���a����  ����������жϺ���8���ֽ�
				if(19 == pos)   //�������ݳ��Ƚ���
				{
					PackLen = (packetlen_C(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_C_LEN LENGTH11111 : %d\n",PackLen);
					//���㳤��
					eStateMachine = EN_RECV_ST_GET_C_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms �򿪶�ʱ��

					break;
				}
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_C_DATA)
		{
			pos = 0;
			while(pos <= Cur)
      {
				if((PackLen-3) == pos)   //�������ݳ��Ƚ���
				{
					eStateMachine = EN_RECV_ST_GET_C_END;
					break;
				}
				pos++;
			}
		}		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_C_END)
		{
			pos = 0;
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_C_END\n");
			while(pos <= Cur)
      {
				
				if((PackLen - 2) == pos)   //'A'
				{
						if(USART_RX_BUF[PackLen - 3] != 'E')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}
				
				if((PackLen - 1) == pos)   //'P'
				{
						if(USART_RX_BUF[PackLen - 2] != 'N')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
				}	
				
				if((PackLen) == pos)   //'S'
				{
						if(USART_RX_BUF[PackLen - 1] != 'D')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
							break;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						//SEGGER_RTT_printf(0, "ID %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
						
						//���Ľ������
						//������ϵ���Ӧ����
						//���ɼ��ɹ������ݸ��Ƶ��ɹ�����
						memset(WIFI_RecvSocketCData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvSocketCData,&USART_RX_BUF[9],(PackLen-9));
						//�������ݣ�ȥ���������
						WIFI_Recv_SocketC_LEN = PackLen-9;
						
						WIFI_RecvSocketCData[WIFI_Recv_SocketC_LEN] = '\0';
						WIFI_Recv_SocketC_Event = 1;
						//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvSocketCData);
						eTypeMachine = EN_RECV_TYPE_UNKNOWN;
						eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
						Cur = 0;
						pos = 0;		
						TIM3_Int_Deinit();
						break;
				}
								
				pos++;
			}
		}	
}

void clear_WIFI(void)
{
	//TIM3_Int_Deinit();
	eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;
	Cur = 0;
}

//����ATģʽ
int AT(void)
{
	int i = 0,flag_failed = 0;
	delayMS(400);
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����ģ��д��"+++"Ȼ����д��"a" д��+++����"a" д��"a"����+ok
	WIFI_SendData("+++", 3);
	//��ȡ��a
	
	for(i = 0;i< 70;i++)
	{
		delayMS(5);
		if(Cur >= 1) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		clear_WIFI();
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"a",1))
		{
			rt_mutex_release(wifi_uart_lock);
			clear_WIFI();
			return -1;
		}
	}
	flag_failed = 0;
	//���ŷ���a
	clear_WIFI();
	WIFI_SendData("a", 1);
	
	for(i = 0;i< 70;i++)
	{
		delayMS(5);
		if(Cur >= 3)
		{
			flag_failed = 1;
			break;
		}
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		clear_WIFI();
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			clear_WIFI();
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT :a+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}


//�л���ԭ���Ĺ���ģʽ    OK
int AT_ENTM(void)
{
	int i = 0,flag_failed = 0;
	
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+ENTM\n",����+ok
	WIFI_SendData("AT+ENTM\n", 8);
	
	for(i = 0;i< 60;i++)
	{
		delayMS(5);
		if(Cur >= 11) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		clear_WIFI();
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[9],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			clear_WIFI();
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+ENTM :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
	
}

//usr �汾��Ϣ
int AT_VER(void)
{
	int i = 0,flag_failed = 0;
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	WIFI_SendData("AT+VER\n", 7);

	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= 10) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		clear_WIFI();
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[8],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			print2msg(ECU_DBG_WIFI,"Version",(char *)USART_RX_BUF);
			clear_WIFI();
			return 0;
		}
	}
	

	printmsg(ECU_DBG_WIFI,"AT_VER WIFI Get reply time out 1");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return -1;

}


int AT_Z(void)
{
	int i = 0,flag_failed = 0;
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+Z\n",����+ok
	WIFI_SendData("AT+Z\n", 5);
	
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= 8) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[6],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+Z :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
	
}

//������������·����SSID
int AT_WSSSID(char *SSSID)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+WSSSID\n",����+ok
	sprintf(AT,"AT+WSSSID=%s\n",SSSID);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	delayMS(5);
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= (strlen(AT)+ 4)) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WSSSID :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//��������·����KEY
int AT_WSKEY(char *SKEY)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+WSSSID\n",����+ok
	sprintf(AT,"AT+WSKEY=WPA2PSK,AES,%s\n",SKEY);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= (strlen(AT)+4)) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WSSSID :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//����WIFI SSID
int AT_WAP(char *ECUID12)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAP=11BGN,ECU_R_%s,Auto\n",ECUID12);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= (strlen(AT)+4)) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAP :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//����WIFI����
int AT_WAKEY(char *NewPasswd)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAKEY=WPA2PSK,AES,%s\n",NewPasswd);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= (strlen(AT)+4)) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//����WIFI����
int AT_WAKEY_Clear(void)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAKEY=OPEN,NONE\n");
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 200;i++)
	{
		delayMS(5);
		if(Cur >= (strlen(AT)+4)) 
		{
			flag_failed = 1;
			break;
		}
		
	}
	
	if(flag_failed == 0)
	{
		rt_mutex_release(wifi_uart_lock);
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY Clear :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

int WIFI_ChangePasswd(char *NewPasswd)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	delayMS(5);
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_WAKEY(NewPasswd);
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();;
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;
}

int WIFI_Reset(void)
{
	GPIO_ResetBits(WIFI_GPIO, WIFI_PIN);
	
	delayMS(1000);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	return 0;
}

int WIFI_SoftReset(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delayMS(5);	
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) 
	{
		WIFI_Reset();	
		return -1;
	}
	
	return 0;
}

int WIFI_ClearPasswd(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delayMS(5);
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret = AT_WAKEY_Clear();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}


int WIFI_Test(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0)return 0;
		}
	
		return -1;
	}	
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();
		if(ret == 0) return 0;
	}
	return -1;
}


int WIFI_Factory(char *ECUID12)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delayMS(5);
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret = AT_WAP(ECUID12);
		ret = AT_WAKEY("88888888");
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}


//����SOCKET����
//����0 ��ʾ�������ӳɹ�  -1��ʾ��������ʧ��
int WIFI_Create(eSocketType Type)
{
	char send[50] = {'\0'};
	char recv[255] = { '\0' };
	int i = 0,j = 0;
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	send[0]= 0x65;
	if(Type == SOCKET_B)
		send[1]= 0x62;
	else if(Type == SOCKET_C)
		send[1]= 0x63;
	else
		return -1;
	
	send[2]= 0x06;
	send[3]= 0x01;
	send[4]= 0x00;
	send[5]= 0x02;
	for(i = 0;i<2;i++)
	{
		clear_WIFI();
		WIFI_SendData(send, 6);
		for(j = 0;j <300;j++)
		{
				if(Cur >= 6)
				{
					memcpy(recv,USART_RX_BUF,6);
					if((recv[0] == 0x65)&&
						(recv[1] == send[1])&&
						(recv[2] == 0x06)&&
						(recv[4] == 0x01))
					{
						//����SOCKET �ɹ�
						printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//����SOCKET ʧ��
						printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket Failed");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return -1;
					}
					
				}
				delayMS(10);
		}
		printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket WIFI Get reply time out 1");
	}	
	rt_mutex_release(wifi_uart_lock);
	clear_WIFI();
	return -1;
}

//�ر�Socket����
//����0 ��ʾ�Ͽ����ӳɹ�  -1��ʾ�Ͽ�����ʧ��
int WIFI_Close(eSocketType Type)
{
	char send[50] = {'\0'};
	char recv[255] = { '\0' };
	int i = 0,j = 0;
	if(1 != WIFI_QueryStatus(Type))
		return -1;

	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	send[0]= 0x65;
	if(Type == SOCKET_B)
		send[1]= 0x62;
	else if(Type == SOCKET_C)
		send[1]= 0x63;
	else
		return -1;
	
	send[2]= 0x06;
	send[3]= 0x02;
	send[4]= 0x00;
	send[5]= 0x03;
	for(i = 0;i<2;i++)
	{
		clear_WIFI();
		WIFI_SendData(send, 6);
		for(j = 0;j <300;j++)
		{
				if(Cur >= 6)
				{
					memcpy(recv,USART_RX_BUF,6);
					if((recv[0] == 0x65)&&
					(recv[1] == send[1])&&
					(recv[2] == 0x06)&&
					(recv[4] == 0x01))
					{
						//����SOCKET �ɹ�
						printmsg(ECU_DBG_WIFI,"WIFI_CloseSocket Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//����SOCKET ʧ��
						printmsg(ECU_DBG_WIFI,"WIFI_CloseSocket Failed");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return -1;
					}
					
				}
				delayMS(10);
		}
		printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket WIFI Get reply time out 1");
	}	
	rt_mutex_release(wifi_uart_lock);
	clear_WIFI();
	return -1;

}


int WIFI_QueryStatus(eSocketType Type)
{
	char send[50] = {'\0'};
	char recv[255] = { '\0' };
	int i = 0,j = 0;
	clear_WIFI();
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	send[0]= 0x65;
	if(Type == SOCKET_B)
		send[1]= 0x62;
	else if(Type == SOCKET_C)
		send[1]= 0x63;
	else
		return -1;
	
	send[2]= 0x06;
	send[3]= 0x03;
	send[4]= 0x00;
	send[5]= 0x04;

	for(i = 0;i<2;i++)
	{
		clear_WIFI();
		WIFI_SendData(send, 6);
		for(j = 0;j <300;j++)
		{
				if(Cur >= 6)
				{
					memcpy(recv,USART_RX_BUF,6);
					if((recv[0] == 0x65)&&
					(recv[1] == send[1])&&
					(recv[2] == 0x06))
					{
						//����SOCKET �ɹ�
						printmsg(ECU_DBG_WIFI,"WIFI_QueryStatus Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//����SOCKET ʧ��
						printmsg(ECU_DBG_WIFI,"WIFI_QueryStatus Failed");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return -1;
					}
					
				}
				delayMS(10);
		}
		printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket WIFI Get reply time out 1");
	}	
	rt_mutex_release(wifi_uart_lock);
	clear_WIFI();
	return -1;
}

//SOCKET A ��������  \n��Ҫ�ڴ����ַ����д���
int SendToSocketA(char *data ,int length,unsigned char ID[8])
{
	char *sendbuff = NULL;
	sendbuff = malloc(4096);
	sprintf(sendbuff,"a%c%c%c%c%c%c%c%c",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
	memcpy(&sendbuff[9],data,length);
	clear_WIFI();
	WIFI_SendData(sendbuff, (length+9));
	
	free(sendbuff);
	sendbuff = NULL;
	return 0;
}

//SOCKET B ��������
int SendToSocketB(char *data ,int length)
{
	char *sendbuff = NULL;
	if((1 == WIFI_QueryStatus(SOCKET_B)) || (0 == WIFI_Create(SOCKET_B)))
	{
		writeconnecttime();
		sendbuff = malloc(4096);
		sprintf(sendbuff,"b00000000");
		memcpy(&sendbuff[9],data,length);
		
		clear_WIFI();
		WIFI_SendData(sendbuff, (length+9));

		free(sendbuff);
		sendbuff = NULL;
		return 0;
	}
	return -1;
}

//SOCKET C ��������
int SendToSocketC(char *data ,int length)
{
	int i = 0;
	char *sendbuff = NULL;
	char msg_length[6] = {'\0'};
	if((1 == WIFI_QueryStatus(SOCKET_C)) || (0 == WIFI_Create(SOCKET_C)))
	{
		sendbuff = malloc(4096);

		if(data[strlen(data)-1] == '\n'){
			sprintf(msg_length, "%05d", strlen(data)-1);
		}
		else{
			sprintf(msg_length, "%05d", strlen(data));
			strcat(data, "\n");
		}
		strncpy(&data[5], msg_length, 5);

		print2msg(ECU_DBG_WIFI,"SendToSocketC",data);
		sprintf(sendbuff,"c00000000");
		memcpy(&sendbuff[9],data,length);
		clear_WIFI();

		for(i = 0;i<(length+9);i++)	
		{
			printf("%x ",sendbuff[i]);
		}
		printf("\n");

		WIFI_SendData(sendbuff, (length+9));

		free(sendbuff);
		sendbuff = NULL;

		return 0;
		
		
	}
	return -1;
}

int WIFI_ChangeSSID(char *SSID,char *Passwd)
{
		int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	delayMS(5);
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_WSSSID(SSID);
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();;
			if(ret == 0) break;
		}
	
		return -1;
	}	

	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_WSKEY(Passwd);
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delayMS(5);
			ret =AT_ENTM();;
			if(ret == 0) break;
		}
	
		return -1;
	}	

	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delayMS(5);
		ret =AT_ENTM();
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;
}

int WIFI_ConStatus(void)
{
	int status = 0;
	status = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8); 
	printf("status : %d\n",status);
	return status;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(SendToSocketB , Send SOCKET B.)
FINSH_FUNCTION_EXPORT(SendToSocketC , Send SOCKET C.)

#endif


