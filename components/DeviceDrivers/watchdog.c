/*
 * File      : watchdog.h
 * Change Logs:
 * Date           Author       		Notes
 * 2017-05-17     shengfeng dong  first version.
 */

#include <watchdog.h>
#include <stm32f10x.h>
#include "stm32f10x_iwdg.h"
void rt_hw_watchdog_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ʹ�ܶ�
	IWDG_SetPrescaler(IWDG_Prescaler_256); //����IWDGԤ��Ƶֵ
	IWDG_SetReload(3125); //����IWDG��װ��ֵ
	IWDG_ReloadCounter(); //����IWDG��װ�ؼĴ�����ֵ ��װ��IWDG������
	IWDG_Enable(); //ʹ��IWDG
}

void kickwatchdog(void)
{
	IWDG_ReloadCounter();//reload
}


