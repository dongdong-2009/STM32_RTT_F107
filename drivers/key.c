/*****************************************************************************/
/* File      : key.c                                                         */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "key.h"
#include <rtthread.h>
#include <stm32f10x.h>
#include "file.h"
#include "rthw.h"


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//����IO��ʼ��
void KEY_Init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��portB��ʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;//PB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��PB9
	
}

//�жϳ�ʼ��
void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  KEY_Init();	 //�����˿ڳ�ʼ��

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ø���ʱ�ӹ���
	
	//GPIOB.9 �ж����Լ��жϳ�ʼ������   �½��ش���
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource9);
 	EXTI_InitStructure.EXTI_Line=EXTI_Line9;	//KEY_RESET
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
 	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
 	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
 	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

 	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ���KEY_RESET���ڵ��ⲿ�ж�ͨ��
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//��ռ���ȼ�2
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
 	NVIC_Init(&NVIC_InitStructure);  
		
}

//�ⲿ�ж�9_5�������
void EXTI9_5_IRQHandler(void)
{
	rt_hw_ms_delay(20);
	if(KEY_Reset==0)	 	 
	{
		//�������ļ��ָ�����������
		//initPath();
		rt_kprintf("EXTI9_5_IRQHandler\n");  //��ӻָ��������õĹ���
		reboot();
		//for(;;);
	}  
	EXTI_ClearITPendingBit(EXTI_Line9); //���LINE9�ϵ��жϱ�־λz  
}
