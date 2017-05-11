#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>
#include "usr_wifi232.h"
#include "debug.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

extern struct rt_device serial4;		//串口1为WIFI收发串口
#define RD_DELAY 	(RT_TICK_PER_SECOND/2) //读取数据延时
#define WR_DELAY	(RT_TICK_PER_SECOND) //写数据延时
#define WIFI_SERIAL (serial4)
rt_mutex_t WIFI_lock = RT_NULL;
extern int WiFiReadFlag;
static int WiFireadtimeoutflag = 0;
//定时器超时函数
static void readtimeout_Wifi(void* parameter)
{
	WiFireadtimeoutflag = 1;
}

int selectWiFi(int timeout)			//Wifi串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{	
	rt_timer_t readtimer;
	readtimer = rt_timer_create("read", /* 定时器名字为 read */
					readtimeout_Wifi, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
	WiFireadtimeoutflag = 0;
	while(1)
	{
		if(WiFireadtimeoutflag)
		{
			rt_timer_delete(readtimer);
			return 0;
		}else 
		{
			rt_hw_us_delay(1);
			if(WiFiReadFlag == 1)	//串口数据监测,如果有数据则返回1
			{
				rt_timer_delete(readtimer);
				rt_thread_delay(RT_TICK_PER_SECOND/5);
				return 1;
			}
		}
	}
}

void clear_WIFI(void)		//清空串口缓冲区的数据
{
	char data[256];
	//清空缓冲器代码	通过将接收缓冲区的所有数据都读取出来，从而清空数据
	WIFI_SERIAL.read(&WIFI_SERIAL,0, data, 255);
	rt_thread_delay(RT_TICK_PER_SECOND/10);
}


//WIFI串口打开函数
int WiFi_Open(void)
{
	int result = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	rt_device_t new;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_6);		//设置引脚为高电平输出，使能WIFI模块
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	new = rt_device_find("uart4");		//寻找WIFI串口并配置模式
	if (new != RT_NULL)
	{
		result = rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
		
		if(result && WIFI_STATUS)
		{
			printdecmsg(ECU_DBG_WIFI,"open WIFI failed ",result);
		}
	}
	//直到WIFI连接上之后   算成功打开
	while(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8));
	WIFI_lock = rt_mutex_create("wifi_lock", RT_IPC_FLAG_FIFO);
	if (WIFI_lock != RT_NULL)
	{
		printmsg(ECU_DBG_WIFI,"open WIFI success");
	}

	return result;
}

//进入AT模式 进入AT模式		OK
int AT(void)
{
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	//先向模块写入"+++"然后再写入"a" 写入+++返回"a" 写入"a"返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"+++", 3);
	//获取到a
	if(selectWiFi(1) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%d %s\n",length,AT);
		if(memcmp(AT,"a",1))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	//发送数据a
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"a", 1);
	if(selectWiFi(1) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT WIFI Get reply time out 2");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		memset(AT,0x00,255);
	  WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%d %s\n",length,AT);
		if(memcmp(AT,"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"Into AT Mode");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//切换回原来的工作模式    OK
int AT_ENTM(void)
{
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	//向模块写入"AT+ENTM\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"AT+ENTM\n", 8);
	//获取到a
	if(selectWiFi(1) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_ENTM WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		if(memcmp(&AT[9],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"Into AT_ENTM Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//AT 模式下所需要的相关命令

//USR的版本信息			OK
int AT_VER(void)
{
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	//向模块写入"AT+ENTM\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"AT+VER\n", 7);
	//获取到a
	if(selectWiFi(1) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_VER WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		if(memcmp(&AT[8],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	print2msg(ECU_DBG_WIFI,"Version",AT);
	rt_mutex_release(WIFI_lock);
	return 0;
}
//复位模块			OK
int AT_Z(void)
{
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	//向模块写入"AT+AT_Z\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"AT+Z\n", 5);

	if(selectWiFi(1) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_Z WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		if(memcmp(&AT[6],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	while(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8));
	printmsg(ECU_DBG_WIFI,"AT_Z Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//恢复出厂设置
int AT_RELD(void)
{
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	//向模块写入"AT+AT_Z\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,"AT+RELD\n", 8);
	rt_thread_delay(RT_TICK_PER_SECOND*3);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_RELD WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[9],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_RELD Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//配置SOCKET A 服务器地址
int AT_NETP(char *IP,int port)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+NETP=TCP,Server,%d,%s\n",port,IP);
	//向模块写入"AT_NETP\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_NETP WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_NETP Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//配置SOCKET B 开启
int AT_TCPB_ON(void)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPB=on\n");
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPB_ON WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%d %s\n",strlen(send),AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPB_ON Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}


//配置SOCKET B IP地址
int AT_TCPADDB(char *IP)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPADDB=%s\n",IP);
	//向模块写入"AT_TCPADDB\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPADDB WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPADDB Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}


//配置SOCKET B IP端口
int AT_TCPPTB(int port)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPPTB=%d\n",port);
	//向模块写入"AT_NETP\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPPTB WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPPTB Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//配置SOCKET B 开启
int AT_TCPC_ON(void)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPC=on\n");
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPC_ON WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPC_ON Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//配置SOCKET C IP地址
int AT_TCPADDC(char *IP)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPADDC=%s\n",IP);
	//向模块写入"AT_NETP\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPADDC WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPADDC Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//配置SOCKET C IP端口
int AT_TCPPTC(int port)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+TCPPTC=%d\n",port);
	//向模块写入"AT_NETP\n",返回+ok
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_TCPPTC WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_TCPPTC Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}

//开启AP+STA功能模式
int AT_FAPSTA_ON(void)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+FAPSTA=on\n");
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND*2);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_FAPSTA_ON WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_FAPSTA_ON Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}
//设置WIFI工作模式  STA or AP
int AT_WMODE(char *WMode)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+WMODE=%s\n",WMode);
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_WMODE WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_WMODE Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}
//设置连接路由器SSID
int AT_WSSSID(char *SSSID)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+WSSSID=%s\n",SSSID);
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_WSSSID WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_WSSSID Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}
//设置连接路由器KEY
int AT_WSKEY(char *SKEY)
{
	char send[50] = {'\0'};
	char AT[255] = { '\0' };
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	//clear_WIFI();
	
	sprintf(send,"AT+WSKEY=WPA2PSK,AES,%s\n",SKEY);
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,send, (strlen(send)));
	rt_thread_delay(RT_TICK_PER_SECOND);
	if(selectWiFi(2) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"AT_WSKEY WIFI Get reply time out 1");
		rt_mutex_release(WIFI_lock);
		return -1;
	}
	else
	{
		WIFI_SERIAL.read(&WIFI_SERIAL,0, AT, 255);
		//printf("%s\n",AT);
		if(memcmp(&AT[strlen(send)+1],"+ok",3))
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	printmsg(ECU_DBG_WIFI,"AT_WSKEY Successful");
	rt_mutex_release(WIFI_lock);
	return 0;
}



//在串口指令模式下接收数据
static int WiFi_RecvData(int timeout,char *data)
{
	int length = 0;
	if(data == RT_NULL)
	{
			return -1;
	}
	if(selectWiFi(timeout) <= 0)
	{
		printmsg(ECU_DBG_WIFI,"WIFI Get reply time out");
		return -1;
	}
	else
	{
		length = WIFI_SERIAL.read(&WIFI_SERIAL,0, data, 255);
		printf("length : %d  %s\n",length,&data[9]);
		return length;
	}
}

typedef enum 
{
	SOCKET_A = 1,
	SOCKET_B = 2,
	SOCKET_C = 3,
}SocketType;

char socketdata[2048] = {'\0'};
char dataA[2048] = {'\0'};	//缓存从SOCKET A读到的数据  最后一次收到的的
int lengthA = 0;						//缓存SOCKET A读到的数据长度
char dataB[2048] = {'\0'};	//缓存从SOCKET B读到的数据	最后一次收到的的
int lengthB = 0;						//缓存SOCKET B读到的数据长度 
char dataC[2048] = {'\0'};	//缓存从SOCKET C读到的数据	最后一次收到的的
int lengthC = 0;						//缓存SOCKET C读到的数据长度 

int RecvSocketData(SocketType Type,char *data,int timeout)
{
	int i=0;
	int length = 0;
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	if(data == RT_NULL)
	{
			rt_mutex_release(WIFI_lock);
			return -1;
	}
	for(i = 0;i<3;i++)
	{
		//首先查看3路是否有数据
		switch(Type)
		{
			case SOCKET_A:
				if(lengthA != 0)
				{
					memcpy(data,dataA,lengthA);
					length = lengthA;
					lengthA = 0;
					rt_mutex_release(WIFI_lock);
					return length;
				}
				break;
			case SOCKET_B:
				if(lengthB != 0)
				{
					memcpy(data,dataB,lengthB);
					length = lengthB;
					lengthB = 0;
					rt_mutex_release(WIFI_lock);
					return length;
				}
				break;
			case SOCKET_C:
				if(lengthC != 0)
				{
					memcpy(data,dataC,lengthC);
					length = lengthC;
					lengthC = 0;
					rt_mutex_release(WIFI_lock);
					return length;
				}
				break;
		}
		
		//3路都不存在数据的情况下，读取一条数据
		memset(socketdata,0x00,2048);
		if((length = WiFi_RecvData(timeout,socketdata)) > 0)
		{
			switch(socketdata[0])
			{
				case 'a':
					memset(dataA,0x00,2048);
					memcpy(dataA,socketdata,length);
					lengthA = length;
					printf("a %d\n",lengthA);
					break;
				case 'b':
					memset(dataB,0x00,2048);
					memcpy(dataB,socketdata,length);
					lengthB = length;
					printf("b %d\n",lengthB);
					break;
				case 'c':
					memset(dataC,0x00,2048);
					memcpy(dataC,socketdata,length);
					lengthC = length;
					printf("c %d\n",lengthC);
					break;
			}
		}else
		{
			rt_mutex_release(WIFI_lock);
			return -1;
		}
	}
	rt_mutex_release(WIFI_lock);
	return -1;
	
}

int SendToSocketB(char *data ,int length)
{
	char *sendbuff = NULL;
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	sendbuff = malloc(4096);
	sprintf(sendbuff,"b00000000%s",data);
	//clear_WIFI();
	lengthB = 0;
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,sendbuff, (length+9));

	free(sendbuff);
	sendbuff = NULL;
	return 0;
}

int SendToSocketC(char *data ,int length)
{
	char *sendbuff = NULL;
	rt_mutex_take(WIFI_lock, RT_WAITING_FOREVER);
	sendbuff = malloc(4096);
	sprintf(sendbuff,"c00000000%s",data);
	//clear_WIFI();
	lengthC = 0;
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,sendbuff, (length+9));

	free(sendbuff);
	sendbuff = NULL;
	return 0;
}



//串口指令模式下收发数据
//在串口指令模式下发送数据    发送TCP报文
int WiFi_SendData(tcp_address_t address ,char *data ,int length)   
{
	int domain_length = 0,msg_length = 0,index = 0;
	unsigned char check_sum = 0;
	char tcp_msg[256];
	if ((data == RT_NULL) || (length <= 0))
	{
		printmsg(ECU_DBG_WIFI,"WIFI_SendData failed ...");
		return -1;
	}
	
	tcp_msg[0] = 0x55;
	tcp_msg[1] = 0xAA;
	
	if(address.address_type == TYPE_IP)
	{
		tcp_msg[4] = 0x03;
		
		tcp_msg[9] = address.address.ip[3]; 
		tcp_msg[10] = address.address.ip[2];
		tcp_msg[11] = address.address.ip[1];
		tcp_msg[12] = address.address.ip[0];
		msg_length =  5 + 4 + length;
		tcp_msg[2] = msg_length/256;
		tcp_msg[3] = msg_length%256;
		rt_memcpy(&tcp_msg[13],data,length);
	}else if(address.address_type == TYPE_DOMAIN)
	{
		tcp_msg[4] = 0x03;
		domain_length = rt_strlen(address.address.domain);
		rt_memcpy(&tcp_msg[9],address.address.ip,domain_length);
		tcp_msg[9+domain_length] = '\0';
		domain_length++;
		msg_length =  5 + domain_length + length;
		tcp_msg[2] = msg_length/256;
		tcp_msg[3] = msg_length%256;
		rt_memcpy(&tcp_msg[9+domain_length],data,length);
	}else
	{
		printmsg(ECU_DBG_WIFI,"WIFI_SendData failed ...");
		return -2;
	}
	
	tcp_msg[5] = 0x00;
	tcp_msg[6] = 0x00;
	tcp_msg[7] = address.port%256;
	tcp_msg[8] = address.port/256;
	
	for(index = 0 ; index < msg_length ; index++)
	{
		check_sum += tcp_msg[index+4]; 
	}
	tcp_msg[4+msg_length] = check_sum;
	
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,tcp_msg, (msg_length+5));
	
	//printhexmsg("wifi","WiFi_SendData", (char *)tcp_msg, (msg_length+5));
	return 	(msg_length+5);
}


//初始化WIFI模块为工作模式  //工作模式 socketB 和socketC为 client和Control_Client服务器配置信息
int initWorkMode(char *clientIP,int clientPort,char *controlIP,int controlPort)
{
	int i = 0;
	//进入AT模式
	rt_thread_delay(RT_TICK_PER_SECOND*10);
	for(i = 0;i<3;i++)
	{
		if(0 == AT())
		{
			break;
		}else
		{
			AT_ENTM();
		}
	}
	//只有在进入AT模式之后才能进行后续的操作
	
		
	
	
	return 0;
}

int initUpdateMode(char *FTP1IP,int FTP1Port,char *DataIP,int DataPort)
{
	
	return 0;
}
#ifdef RT_USING_FINSH
#include <finsh.h>

void testWIFIRecv(SocketType Type,int timeout)	//无线接收测试
{
	char data[2048] = {0};
	int length = 0;
	length = RecvSocketData(Type,data,timeout);
	rt_kprintf("WiFi_RecvData:%d   %s\n",length,&data[9]);
}
FINSH_FUNCTION_EXPORT(testWIFIRecv , WIFI Recv Test.)

FINSH_FUNCTION_EXPORT(AT , Into AT Mode.)
FINSH_FUNCTION_EXPORT(WiFi_Open , Open WIFI Modle.)
FINSH_FUNCTION_EXPORT(AT_ENTM , Into Last Mode.)
FINSH_FUNCTION_EXPORT(AT_VER , Get WIFI SoftWare Version.)
FINSH_FUNCTION_EXPORT(AT_Z , Reset WIFI Modle.)
FINSH_FUNCTION_EXPORT(AT_RELD , Reload the default setting and reboot.)
FINSH_FUNCTION_EXPORT(AT_NETP , Set SOCKET A Configure.)

FINSH_FUNCTION_EXPORT(AT_TCPB_ON , TCPB ON.)
FINSH_FUNCTION_EXPORT(AT_TCPADDB , Set SOCKET B IP.)
FINSH_FUNCTION_EXPORT(AT_TCPPTB , Set SOCKET B PORT.)

FINSH_FUNCTION_EXPORT(AT_TCPC_ON , TCPC ON.)
FINSH_FUNCTION_EXPORT(AT_TCPADDC , Set SOCKET C IP.)
FINSH_FUNCTION_EXPORT(AT_TCPPTC , Set SOCKET C PORT.)

FINSH_FUNCTION_EXPORT(AT_FAPSTA_ON , Set AP+STA Mode.)
FINSH_FUNCTION_EXPORT(AT_WMODE , Set Modle Mode.)
FINSH_FUNCTION_EXPORT(AT_WSSSID , Set STA SSID.)
FINSH_FUNCTION_EXPORT(AT_WSKEY , Set STA Key.)

FINSH_FUNCTION_EXPORT(SendToSocketB , Send SOCKET B.)
FINSH_FUNCTION_EXPORT(SendToSocketC , Send SOCKET C.)


#endif
