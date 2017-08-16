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

//4个字节
unsigned short packetlen_A(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//2个字节
unsigned short packetlen_B(unsigned char *packet)
{
	unsigned short len = 0,count = 0;
	count = (packet[0] - '0') * 10 + (packet[1] - '0');
	len =  3 + 9 + count *14;
	return len;
	
}

//5个字节
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

//WIFI发送函数 
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

//初始化IO 串口5
//bound:波特率
void uart5_init(u32 bound){
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//使能串口时钟UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);	//使能GPIOA时钟

 	//USART_DeInit(UART5);  //复位串口5
	 //UART5_TX   PC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC12
   
    //UART5_RX	  PD.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PD2
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//收发模式

    USART_Init(UART5, &USART_InitStructure); //初始化串口

   //UART5 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(UART5, ENABLE);                    //使能串口 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	

}

unsigned char ID[9] = {'\0'};

//WIFI  socket A 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketAData[SOCKETA_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketA_Event = 0;
unsigned int WIFI_Recv_SocketA_LEN =0;
unsigned char ID_A[9] = {'\0'};
	
//WIFI  socket B 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketBData[SOCKETB_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketB_Event = 0;
unsigned int WIFI_Recv_SocketB_LEN =0;

//WIFI  socket C 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketCData[SOCKETC_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketC_Event = 0;
unsigned int WIFI_Recv_SocketC_LEN =0;

//wifi  串口当前收到的数据 
unsigned char USART_RX_BUF[USART_REC_LEN];     			//接收缓冲,最大USART_REC_LEN个字节.
unsigned short Cur = 0;															//当前采值位置
unsigned short PackLen = 0;
eRecvSM eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;	//数据采集状态机
eRecvType eTypeMachine = EN_RECV_TYPE_UNKNOWN;     	//
unsigned short pos = 0;															//数据解析位置

void UART5_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(UART5);//(UART5->DR);	//读取接收到的数据
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
		
		//接收A报文头部
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
	
		//接收ID
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
				
				if(9 == pos)   //接收版本号完毕
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
		if(eStateMachine == EN_RECV_ST_GET_A_HEAD)    //接收报文头部
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
				
				if(14 == pos)   //接收版本号完毕
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
				//判断是否有a出现  如果出现了判断后面8个字节
				if(18 == pos)   //接收数据长度结束
				{
					PackLen = (packetlen_A(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "LENGTH11111 : %d\n",PackLen);
					//计算长度
					eStateMachine = EN_RECV_ST_GET_A_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器

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

				if((PackLen - 3) == pos)   //接收数据长度结束
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
						
						//报文接收完毕
						//进行完毕的相应操作
						//将采集成功的数据复制到成功数组
						memset(WIFI_RecvSocketAData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvSocketAData,&USART_RX_BUF[9],(PackLen-9));
						memcpy(ID_A,ID,8);
						//解析数据，去掉多包问题
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
		if(eStateMachine == EN_RECV_ST_GET_B_HEAD)    //接收报文头部
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_HEAD\n");
      while(pos < Cur)
      {	
				if(10 == pos)   // 101   14字节的时间点
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
				//判断是否有a出现  如果出现了判断后面8个字节
				if(19 == pos)   //接收数据长度结束
				{
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_LEN LENGTH11111 :     %s\n",&USART_RX_BUF[19]);
					PackLen = (packetlen_B(&USART_RX_BUF[19])+9);
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_B_LEN LENGTH11111 : %d\n",PackLen);
					//计算长度
					eStateMachine = EN_RECV_ST_GET_B_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器

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
				if(PackLen == pos)   //接收数据长度结束
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
			//报文接收完毕
			//进行完毕的相应操作
			//将采集成功的数据复制到成功数组
			memset(WIFI_RecvSocketBData,0x00,USART_REC_LEN);
			memcpy(WIFI_RecvSocketBData,&USART_RX_BUF[9],(PackLen-9));
			//解析数据，去掉多包问题
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
		if(eStateMachine == EN_RECV_ST_GET_C_HEAD)    //接收报文头部
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
				
				if(14 == pos)   //接收版本号完毕
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
				//判断是否有a出现  如果出现了判断后面8个字节
				if(19 == pos)   //接收数据长度结束
				{
					PackLen = (packetlen_C(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_C_LEN LENGTH11111 : %d\n",PackLen);
					//计算长度
					eStateMachine = EN_RECV_ST_GET_C_DATA;
					delayMS(10);
					TIM3_Int_Init(299,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器

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
				if((PackLen-3) == pos)   //接收数据长度结束
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
						
						//报文接收完毕
						//进行完毕的相应操作
						//将采集成功的数据复制到成功数组
						memset(WIFI_RecvSocketCData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvSocketCData,&USART_RX_BUF[9],(PackLen-9));
						//解析数据，去掉多包问题
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

//进入AT模式
int AT(void)
{
	int i = 0,flag_failed = 0;
	delayMS(400);
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//先向模块写入"+++"然后再写入"a" 写入+++返回"a" 写入"a"返回+ok
	WIFI_SendData("+++", 3);
	//获取到a
	
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
	//接着发送a
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
	printf("AT :a+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}


//切换回原来的工作模式    OK
int AT_ENTM(void)
{
	int i = 0,flag_failed = 0;
	
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//发送"AT+ENTM\n",返回+ok
	WIFI_SendData("AT+ENTM\n", 8);
	
	for(i = 0;i< 400;i++)
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
	printf("AT+ENTM :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
	
}

//usr 版本信息
int AT_VER(void)
{
	int i = 0,flag_failed = 0;
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	WIFI_SendData("AT+VER\n", 7);

	for(i = 0;i< 400;i++)
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
		if(memcmp(&USART_RX_BUF[7],"+ok",3))
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
	//发送"AT+Z\n",返回+ok
	WIFI_SendData("AT+Z\n", 5);
	
	for(i = 0;i< 400;i++)
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
	printf("AT+Z :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
	
}

//设置连接无线路由器SSID
int AT_WSSSID(char *SSSID)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//发送"AT+WSSSID\n",返回+ok
	sprintf(AT,"AT+WSSSID=%s\n",SSSID);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	delayMS(5);
	for(i = 0;i< 400;i++)
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
	printf("AT+WSSSID :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//设置连接路由器KEY
int AT_WSKEY(char Auth,char Encry,char *SKEY,int passWDLen)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	char AuthString[20] = {'\0'};
	char EncryString[20] = {'\0'};
	int AT_length = 0;
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();

	if(Auth == '1')
	{
		sprintf(AuthString,"OPEN");
	}else if(Auth == '2')
	{
		sprintf(AuthString,"SHARED");
	}else if(Auth == '3')
	{
		sprintf(AuthString,"WPAPSK");
	}else if(Auth == '4')
	{
		sprintf(AuthString,"WPA2PSK");
	}

	if(Encry == '1')
	{
		sprintf(EncryString,"NONE");
	}else if(Encry == '2')
	{
		sprintf(EncryString,"WEP-H");
	}else if(Encry == '3')
	{	
		sprintf(EncryString,"WEP-A");
	}else if(Encry == '4')
	{
		sprintf(EncryString,"TKIP");
	}else if(Encry == '5')
	{
		sprintf(EncryString,"AES");
	}
	
	//发送"AT+WSSSID\n",返回+ok
	if(passWDLen > 0)
	{
		//密码长度大于0
		
		sprintf(AT,"AT+WSKEY=%s,%s,",AuthString,EncryString);
		AT_length = strlen(AT);
		memcpy(&AT[AT_length],SKEY,passWDLen);
		AT[AT_length+passWDLen]  = '\n';
		AT_length = AT_length+passWDLen + 1;
		
	}else
	{
		//密码长度为0
		sprintf(AT,"AT+WSKEY=%s,%s\n",AuthString,EncryString);
		AT_length = strlen(AT);
	}
	
	printf("%s\n",AT);
	WIFI_SendData(AT, (AT_length+1));
	
	for(i = 0;i< 400;i++)
	{
		delayMS(5);
		if(Cur >= (AT_length+4)) 
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
		if(memcmp(&USART_RX_BUF[AT_length+1],"+ok",3))
		{
			rt_mutex_release(wifi_uart_lock);
			return -1;
		}
	}
	printf("AT+WSSSID :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//设置WIFI SSID
int AT_WAP(char *ECUID12)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAP=11BGN,ECU_R_%s,Auto\n",ECUID12);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+WAP :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//设置WIFI密码
int AT_WAKEY(char *NewPasswd)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAKEY=WPA2PSK,AES,%s\n",NewPasswd);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+WAKEY :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//设置WIFI密码
int AT_WAKEY_Clear(void)
{
	int i = 0,flag_failed = 0;
	char AT[100] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAKEY=OPEN,NONE\n");
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+WAKEY Clear :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}
//配置SOCKET B 开启
int AT_TCPB_ON(void)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	sprintf(AT,"AT+TCPB=on\n");
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPB=on :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//配置SOCKET B IP地址
int AT_TCPADDB(char *IP)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+TCPADDB=%s\n",IP);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPADDB :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//配置SOCKET B IP端口
int AT_TCPPTB(int port)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+TCPPTB=%d\n",port);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPPTB :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//配置SOCKET C 开启
int AT_TCPC_ON(void)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+TCPC=on\n");
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPC :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//配置SOCKET C IP端口
int AT_TCPADDC(char *IP)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+TCPADDC=%s\n",IP);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPADDC :+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//配置SOCKET C IP端口
int AT_TCPPTC(int port)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+TCPPTC=%d\n",port);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPPTC:+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;

}

//开启AP+STA功能模式
int AT_FAPSTA_ON(void)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+FAPSTA=on\n");
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 600;i++)
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
	printf("AT+TCPPTC:+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}

//设置WIFI工作模式  STA or AP
int AT_WMODE(char *WMode)
{
	int i = 0,flag_failed = 0;
	char AT[255] = { '\0' };
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	clear_WIFI();
	
	sprintf(AT,"AT+WMODE=%s\n",WMode);
	printf("%s\n",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	for(i = 0;i< 400;i++)
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
	printf("AT+TCPPTC:+ok\n");
	clear_WIFI();
	rt_mutex_release(wifi_uart_lock);
	return 0;
}


int InitTestMode(void)
{
	int i = 0,res = 0;
	//进入AT模式
	for(i = 0;i<3;i++)
	{
		if(0 == AT())
		{//进入AT模式成功
			//printf("AT Successful\n");
			res = 0;
			break;
		}else
		{//进入AT模式失败，尝试先退出AT模式。
			//printf("AT Failed,AT_ENTM\n");
			res = -1;
			AT_ENTM();
		}
	}
	if(res == -1) return -1;
	//只有在进入AT模式之后才能进行后续的操作

	//设置工作模式 WMODE
		for(i = 0;i<3;i++)
	{
		if(0 == AT_WMODE("STA"))
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;	

	//设置同时开启AP-STA模式
	for(i = 0;i<3;i++)
	{
		if(0 == AT_FAPSTA_ON())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;	

	//打开SOCKET B功能
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPB_ON())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;	
	//配置SOCKET B的服务器IP地址
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPADDB("139.168.200.158"))
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	
	//配置SOCKET B的服务器端口号
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPPTB(8093))
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	
	//打开SOCKET C功能
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPC_ON())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	
	//配置SOCKET C的服务器IP地址
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPADDC("139.168.200.158"))
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	//配置SOCKET C的服务器端口号
	for(i = 0;i<3;i++)
	{
		if(0 == AT_TCPPTC(8997))
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	
	for(i = 0;i<3;i++)
	{
		if(0 == AT_ENTM())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;
	
	printmsg(ECU_DBG_WIFI,"WIFI_InitTestMode Over");
	return 0;


}

int InitWorkMode(void)
{
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


//创建SOCKET连接
//返回0 表示创建连接成功  -1表示创建连接失败
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
						//创建SOCKET 成功
						printmsg(ECU_DBG_WIFI,"WIFI_CreateSocket Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//创建SOCKET 失败
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

//关闭Socket连接
//返回0 表示断开连接成功  -1表示断开连接失败
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
						//创建SOCKET 成功
						printmsg(ECU_DBG_WIFI,"WIFI_CloseSocket Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//创建SOCKET 失败
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
						//创建SOCKET 成功
						printmsg(ECU_DBG_WIFI,"WIFI_QueryStatus Successful");
						rt_mutex_release(wifi_uart_lock);
						clear_WIFI();
						return 0;
					}else
					{
						//创建SOCKET 失败
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

//SOCKET A 发送数据  \n需要在传入字符串中带入
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

//SOCKET B 发送数据
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

//SOCKET C 发送数据
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

int WIFI_ChangeSSID(char *SSID,char Auth,char Encry,char *Passwd,int passWDLen)
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
		ret =AT_WSKEY(Auth,Encry,Passwd,passWDLen);
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

FINSH_FUNCTION_EXPORT(AT , Into AT Mode.)
FINSH_FUNCTION_EXPORT(AT_ENTM , Into AT_ENTM Mode.)
FINSH_FUNCTION_EXPORT(AT_VER , Get version.)
FINSH_FUNCTION_EXPORT(AT_Z , Reset USR232.)

FINSH_FUNCTION_EXPORT(AT_WSSSID , Set WSSSID.)
FINSH_FUNCTION_EXPORT(AT_WSKEY , Set WSKEY.)

FINSH_FUNCTION_EXPORT(AT_WAP , Set WAP.)
FINSH_FUNCTION_EXPORT(AT_WAKEY , Set WAKEY.)
FINSH_FUNCTION_EXPORT(AT_WAKEY_Clear , Clear WAKEY.)

FINSH_FUNCTION_EXPORT(AT_TCPB_ON , Set TCP B ON.)
FINSH_FUNCTION_EXPORT(AT_TCPADDB , Set TCP B ADDR.)
FINSH_FUNCTION_EXPORT(AT_TCPPTB , Set TCP B PORT.)
FINSH_FUNCTION_EXPORT(AT_TCPC_ON , Set TCP C ON.)
FINSH_FUNCTION_EXPORT(AT_TCPADDC , Set TCP C ADDR.)
FINSH_FUNCTION_EXPORT(AT_TCPPTC , Set TCP C PORT.)

FINSH_FUNCTION_EXPORT(AT_FAPSTA_ON , Set FAPSTA ON.)
FINSH_FUNCTION_EXPORT(AT_WMODE , Set WMode AP or STA.)



FINSH_FUNCTION_EXPORT(InitTestMode , Init Test Mode.)

FINSH_FUNCTION_EXPORT(WIFI_Factory , Set WIFI ID .)

FINSH_FUNCTION_EXPORT(SendToSocketB , Send SOCKET B.)
FINSH_FUNCTION_EXPORT(SendToSocketC , Send SOCKET C.)

#endif


