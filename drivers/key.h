#ifndef __KEY_H
#define __KEY_H	 
 
#define KEY_Reset  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//?����?���?��0

void KEY_Init(void);//IO��ʼ��
void EXTIX_Init(void);//�ⲿ�жϳ�ʼ��   
#endif
