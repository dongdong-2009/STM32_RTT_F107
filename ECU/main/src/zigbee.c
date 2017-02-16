#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include "debug.h"
#include "zigbee.h"
#include <rthw.h>
#include <stm32f10x.h>
#include "resolve.h"

extern struct rt_device serial4;		//����4ΪZigbee�շ�����

/*extern*/ ecu_info ecu;

#define RD_DELAY 	(RT_TICK_PER_SECOND)
#define WR_DELAY	(RT_TICK_PER_SECOND)
#define ZIGBEE_SERIAL (serial4)

void clear_zbmodem(void)		//��մ��ڻ�����������
{
	//��ջ���������
	//������
	rt_thread_delay(RT_TICK_PER_SECOND);
}

int openzigbee(void)
{
	int result = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//��������Ϊ�ߵ�ƽ�����ʹ��Zigbbeģ��
	
	result = ZIGBEE_SERIAL.open(&ZIGBEE_SERIAL,RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |
                          RT_DEVICE_FLAG_INT_TX |   RT_DEVICE_FLAG_DMA_RX);
	if(result)
	{
		rt_kprintf("open serial 4 failed : %d\r\n",result);
	}else
	{
		rt_kprintf("open serial 4 success\r\n");
	}
	return result;
}

//��λzigbeeģ��  ͨ��PC7�ĵ�ƽ�ø��õ�Ȼ��ﵽ��λ��Ч��
void zigbee_reset(void)
{
	//������PC7Ϊ�͵�ƽ��Ȼ��������Ϊ�ߵ�ƽ�ﵽ��λ�Ĺ���
	GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);		//��������Ϊ�͵�ƽ���
	rt_thread_delay(RT_TICK_PER_SECOND);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//��������Ϊ�ߵ�ƽ���
	rt_thread_delay(RT_TICK_PER_SECOND * 10);
}

int zb_shortaddr_cmd(int shortaddr, char *buff, int length)		//zigbee �̵�ַ��ͷ
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = shortaddr>>8;
	sendbuff[6]  = shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;


	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}
	
	if(0!=shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0,sendbuff, (length+15));
		printhexmsg("zb_shortaddr_cmd", (char *)sendbuff, 15);
		return 1;
	}
	else
		return -1;

}

int zb_shortaddr_reply(char *data,int shortaddr,char *id)			//��ȡ������ķ���֡,�̵�ַģʽ
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;
	
	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==shortaddr/256)&&(data_all[3]==shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(id,inverterid)))
		{
				return size;
		}
		else
		{
			return -1;
		}
	}else
	{
		rt_kprintf("serial2.read failed \r\n");
		return -1;
	}

}


int zb_get_reply(char *data,inverter_info *inverter)			//��ȡ������ķ���֡
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;
	
	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			inverter->signalstrength=data_all[4];
			return size;
		}
		else
		{
			inverter->signalstrength=0;
			return -1;
		}
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
	
}

int zb_get_reply_update_start(char *data,inverter_info *inverter)			//��ȡ�����Զ�̸��µ�Update_start����֡��ZK��������Ӧʱ�䶨Ϊ10��
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size>0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}

}

int zb_get_reply_restore(char *data,inverter_info *inverter)			//��ȡ�����Զ�̸���ʧ�ܣ���ԭָ���ķ���֡��ZK����Ϊ��ԭʱ��Ƚϳ������Ե���дһ������
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);

		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}	
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
}

int zb_get_reply_from_module(char *data)			//��ȡzigbeeģ��ķ���֡
{
	int size;

	rt_thread_delay( RD_DELAY );
	size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	
	if(size > 0)
	{
		printhexmsg("Reply", data, size);
		return size;
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
}

int zb_get_id(char *data)			//��ȡ�����ID
{
	int size;

	rt_thread_delay( RD_DELAY * 10 );
	size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	
	if(size > 0)
	{
		printhexmsg("Reply", data, size);
		return size;
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}

}

int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter)			//�ر��޶�����������ϱ�ID����
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	clear_zbmodem();			//����ָ��ǰ������ջ�����
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	if(0!=inverter->shortaddr)
	{
		printmsg("Turn off limited report id");
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0,sendbuff, 15);
		printhexmsg("sendbuff",(char *)sendbuff,15);
		ret = zb_get_reply_from_module(data);
		if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		{
			if(inverter->zigbee_version!=data[9])
			{
				inverter->zigbee_version = data[9];
//				update_zigbee_version(inverter);
			}
			return 1;
		}
		else
			return -1;
	}
	else
		return -1;

}

int zb_turnoff_rptid(int short_addr)			//�رյ���������ϱ�ID����
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0;
	int check=0;
	printmsg("Turn off report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
//	ret = zb_get_reply(data);
//	if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		return 1;
//	else
//		return -1;
}

int zb_get_inverter_shortaddress_single(inverter_info *inverter)			//��ȡ��ָ̨��������̵�ַ��ZK
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	char inverterid[13] = {'\0'};
	int check=0;
	printmsg("Get inverter shortaddresssingle");

	clear_zbmodem();			//����ָ��ǰ����ջ�����
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0E;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;//panid
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;

	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));

//	strcpy(&sendbuff[15],inverter->inverterid);


	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",(char *)sendbuff,21);
	ret = zb_get_reply_from_module(data);

	rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[4],data[5],data[6],data[7],data[8],data[9]);

	if((11 == ret)&&(0xFF == data[2])&&(0==rt_strcmp(inverter->inverterid,inverterid)))
	{
		inverter->shortaddr = data[0]*256 + data[1];
//		update_inverter_addr(inverter->inverterid,inverter->shortaddr);
		return 1;
	}
	else
		return -1;

}

//����(��������ϱ�ID,���Դ�����ID���е���������а�)
int zb_turnon_limited_rtpid(inverter_info *firstinverter)
{
	
	return 0;
}

int zb_turnon_rtpid(inverter_info *firstinverter)			//����������Զ��ϱ�ID
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int i, count=0;
	int short_addr;
	char inverterid[256] = {'\0'};
	int check=0;
	printmsg("Turn on report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x02;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);

	for(i = 0;i<60;i++)
	{
		rt_thread_delay(RT_TICK_PER_SECOND*5);//�ϱ�ʱ��Ϊ10����
		rt_memset(data, '\0', sizeof(data));
		if((11 == zb_get_id(data)) && (0xFF == data[2]) && (0xFF == data[3]))
		{
			short_addr = data[0] * 256 + data[1];
			for(i=0; i<6; i++){
				inverterid[2*i] = (data[i+4]>>4) + 0x30;
				inverterid[2*i+1] = (data[i+4]&0x0f) + 0x30;
			}
			print2msg("inverterid",inverterid);
//			save_inverter_id(inverterid,short_addr);  //����������Լ��̵�ַ
			rt_thread_delay(RT_TICK_PER_SECOND*5);
			zb_turnoff_rptid(short_addr);
			rt_memset(inverterid, '\0', sizeof(inverterid));
		}else
		{
			rt_thread_delay(RT_TICK_PER_SECOND*5);
		}
	}
	
//	count=get_id_from_db(firstinverter);
	return count;
}

int zb_change_inverter_panid_broadcast(void)	//�㲥�ı��������PANID��ZK
{
	char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x03;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg("sendbuff",sendbuff,15);

	return 1;
}

int zb_change_inverter_panid_single(inverter_info *inverter)	//����ı��������PANID���ŵ���ZK
{
	char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));
	
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	rt_thread_delay(RT_TICK_PER_SECOND);
	return 1;

}

int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter)	//���㻹ԭ�������PANID��0X8888���ŵ�0X10��ZK
{
	char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x10;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	rt_thread_delay(RT_TICK_PER_SECOND);
	return 1;

}

//����ECU��PANID���ŵ�
int zb_change_ecu_panid(void)
{
	unsigned char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg("Set ECU PANID and Channel", (char *)sendbuff, 15);

	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_thread_delay(RT_TICK_PER_SECOND*2); //��ʱ2S����Ϊ������ECU�ŵ���PANID��ᷢ6��FF
		return 1;
	}

	return -1;
}

int zb_restore_ecu_panid_0x8888(void)			//�ָ�ECU��PANIDΪ0x8888,ZK
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg("sendbuff",(char *)sendbuff,15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;
}

//����ECU��PANIDΪ0xFFFF,�ŵ�Ϊָ���ŵ�(ע:�������������������ʱ,�轫ECU��PANID��Ϊ0xFFFF)
int zb_restore_ecu_panid_0xffff(int channel)
{
	unsigned char sendbuff[15] = {'\0'};
	char recvbuff[256];
	int i;
	int check=0;
	//��ECU��������
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0xFF;
	sendbuff[8]  = 0xFF;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	printhexmsg("Change ECU channel (PANID:0xFFFF)", (char *)sendbuff, 15);

	//���շ���
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_thread_delay(RT_TICK_PER_SECOND*2);
		return 1;
	}

	return -1;
}

int zb_send_cmd(inverter_info *inverter, char *buff, int length)		//zigbee��ͷ
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = inverter->shortaddr>>8;
	sendbuff[6]  = inverter->shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	printdecmsg("shortaddr",inverter->shortaddr);
	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}
	
	rt_thread_delay(RT_TICK_PER_SECOND/5);
	if(0!=inverter->shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);
		//printhexmsg("Send", sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_broadcast_cmd(char *buff, int length)		//zigbee�㲥��ͷ
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);

	return 1;
}

int zb_query_inverter_info(inverter_info *inverter)		//����������Ļ�����
{
	int i = 0;
	char sendbuff[256];
	char recvbuff[256];
	
	clear_zbmodem();
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDC;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	zb_send_cmd(inverter, sendbuff, i);
	print2msg("Query Inverter's Model and Version", inverter->inverterid);

	if ((16 == zb_get_reply(recvbuff, inverter))
			&& (0xFB == recvbuff[0])
			&& (0xFB == recvbuff[1])
			&& (0xDC == recvbuff[3])
			&& (0xFE == recvbuff[14])
			&& (0xFE == recvbuff[15])) {
		inverter->model = recvbuff[4];
		inverter->version = recvbuff[5]*256 + recvbuff[6];
		return 1;
	}

	return -1;
}

int zb_query_data(inverter_info *inverter)		//���������ʵʱ����
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

/*  //ģ����������� K1-K80
	char test_data_BB[80] = {0x05, 0x16, 0x13, 0xFC, 0x00, 0x00, 0x00, 0x47, 0x4C, 0x6E,
							 0xF0, 0x00, 0x29, 0x32, 0x80, 0x25, 0x82, 0x81, 0x27, 0xA1,
							 0xBD, 0x28, 0x47, 0x60, 0x27, 0xFF, 0x9D, 0x04, 0xF2, 0xD4,
							 0xEF, 0xD5, 0x04, 0xCF, 0x21, 0x97, 0xC5, 0x04, 0x84, 0x4F,
							 0xB8, 0x7F, 0x04, 0xD6, 0xD4, 0x78, 0xFC, 0x00, 0x2E, 0x57,
							 0x00, 0x19, 0xC1, 0x00, 0x17, 0xF7, 0x02, 0xD9, 0xD6, 0x02,
							 0xCD, 0xE1, 0x02, 0xCC, 0x5F, 0x01, 0x66, 0x74, 0x01, 0x5F,
							 0xB9, 0x01, 0x5F, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
*/
	print2msg("Query inverter data",inverter->inverterid);
	clear_zbmodem();			//����ָ��ǰ,����ջ�����
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xBB;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);

/*	//�滻��ģ������
	for (i=0; i<80; i++) {
		data[4+i] = test_data_BB[i];
	}
*/

	if((88 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[86])&&(0xFE == data[87]))
	{

		//����YC1000��������
		if (inverter->shortaddr == 2733 || inverter->shortaddr == 48945) {
			inverter->dataflag = 0;
			return -1;
		}

		inverter->no_getdata_num = 0;	//һ�����յ����ݾ���0,ZK
		inverter->dataflag = 1;	//���յ����ݾ���Ϊ1

		if(1==inverter->model)
			resolvedata_250(&data[4], inverter);
		else if(2==inverter->model)
			resolvedata_250(&data[4], inverter);
		else if(3==inverter->model)
			resolvedata_500(&data[4], inverter);
		else if(4==inverter->model)
			resolvedata_500(&data[4], inverter);
		else if(5==inverter->model)
			resolvedata_1000(&data[4], inverter);
		else if(6==inverter->model)
			resolvedata_1000(&data[4], inverter);
		else
			{;}

		return 1;
	}
	else
	{
		inverter->dataflag = 0;		//û�н��յ����ݾ���Ϊ0
		return -1;
	}
}

int zb_test_communication(void)		//zigbee����ͨ����û�жϿ�
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	printmsg("test zigbee communication");
	clear_zbmodem();			//����ָ��ǰ,����ջ�����
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0D;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;

}

int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter)		//�����޸�CCָ��
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	clear_zbmodem();			//��������ǰ,��ջ�����
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = protect_parameter[0];
	sendbuff[i++] = protect_parameter[1];
	sendbuff[i++] = protect_parameter[2];
	sendbuff[i++] = protect_parameter[3];
	sendbuff[i++] = protect_parameter[4];
	sendbuff[i++] = protect_parameter[5];
	sendbuff[i++] = protect_parameter[6];
	sendbuff[i++] = protect_parameter[7];
	sendbuff[i++] = protect_parameter[8];
	sendbuff[i++] = protect_parameter[9];
	sendbuff[i++] = protect_parameter[10];
	sendbuff[i++] = protect_parameter[11];
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(inverter->inverterid,"Set protect parameters");
	printhexmsg("Set protect parameters", sendbuff, i);
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DD_reply)		//�洢������ѯDDָ��
{
	int i=0,ret;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(inverter->inverterid, "Query protect parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(protect_data_DD_reply,inverter);
	if((33 == ret) && (0xDD == protect_data_DD_reply[3]) && (0xFB == protect_data_DD_reply[0]) && (0xFB == protect_data_DD_reply[1]) && (0xFE == protect_data_DD_reply[31]) && (0xFE == protect_data_DD_reply[32]))
		return 1;
	if((58 == ret) &&
				(0xFB == protect_data_DD_reply[0]) &&
				(0xFB == protect_data_DD_reply[1]) &&
				(0xDA == protect_data_DD_reply[3]) &&
				(0xFE == protect_data_DD_reply[56]) &&
				(0xFE == protect_data_DD_reply[57]))
		return 1;
	else
		return -1;
}

int zb_afd_broadcast(void)		//AFD�㲥ָ��
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);

	return 1;
}

int zb_turnon_inverter_broadcast(void)		//����ָ��㲥,OK
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_boot_single(inverter_info *inverter)		//����ָ���,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_shutdown_broadcast(void)		//�ػ�ָ��㲥,OK
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_shutdown_single(inverter_info *inverter)		//�ػ�ָ���,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_boot_waitingtime_single(inverter_info *inverter)		//�����ȴ�ʱ���������Ƶ���,OK
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_clear_gfdi_broadcast(void)		//���GFDI�㲥,OK
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_clear_gfdi(inverter_info *inverter)		//���GFDI,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_ipp_broadcast(void)		//IPP�㲥
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_ipp_single(inverter_info *inverter)		//IPP����
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_frequency_protectime_broadcast(void)		//ǷƵ����ʱ��㲥
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_frequency_protectime_single(inverter_info *inverter)		//ǷƵ����ʱ�䵥��
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC6;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_voltage_protectime_broadcast(void)		//Ƿѹ����ʱ��㲥
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC9;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_voltage_protectime_single(inverter_info *inverter)		//Ƿѹ����ʱ�䵥��
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xCA;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int process_gfdi(inverter_info *firstinverter)
{
	
	return 0;
}

int compare_protect_data(char *set_protect_data, char *actual_protect_data)				//�Ƚ���������ص�Ԥ��ֵ��ҳ���������Ԥ��ֵ
{
	int i;

	for(i=0; i<12; i++)
	{
		if(set_protect_data[i] != actual_protect_data[i])
			return -1;
	}

	return 0;
}

int resolve_presetdata(inverter_info *inverter, char * protect_data_result)	//?????????????
{
	float temp;

	/*if(1 == ecu.type)
		temp = (presetdata[0]*256 + presetdata[1])/2.90345;
	else
		temp = (presetdata[0]*256 + presetdata[1])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vl1 = (int)temp +1;
	else
		inverter->protect_vl1 = (int)temp;

	if(1 == ecu.type)
		temp = (presetdata[2]*256 + presetdata[3])/2.90345;
	else
		temp = (presetdata[2]*256 + presetdata[3])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vu1 = (int)temp +1;
	else
		inverter->protect_vu1 = (int)temp;*/

	/*
	if(1 == ecu.type)
		temp = (protect_data_result[4]*256 + protect_data_result[5])/2.90345;
	else
		temp = (protect_data_result[4]*256 + protect_data_result[5])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if(1 == ecu.type)
		temp = (protect_data_result[6]*256 + protect_data_result[7])/2.90345;
	else
		temp = (protect_data_result[6]*256 + protect_data_result[7])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if(1 == ecu.type)
		inverter->protect_frequency_min = 600-protect_data_result[8];
	else
		inverter->protect_frequency_min = 500-protect_data_result[8];
	if(1 == ecu.type)
		inverter->protect_frequency_max = 600+protect_data_result[9];
	else
		inverter->protect_frequency_max = 500+protect_data_result[9];
	inverter->recovery_time = protect_data_result[10]*256 + protect_data_result[11];
*/
	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//??????
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/11614.45;

	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//??????
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/11614.45;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_min = 223750/(protect_data_result[9]*256 + protect_data_result[10]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_min = 256000/(protect_data_result[9]*256 + protect_data_result[10]);

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_max = 223750/(protect_data_result[11]*256 + protect_data_result[12]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_max = 256000/(protect_data_result[11]*256 + protect_data_result[12]);

	inverter->recovery_time = protect_data_result[13]*256 + protect_data_result[14];

	return 1;
}

int process_protect_data(inverter_info *firstinverter)
{
	
	return 0;
}
