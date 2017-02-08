#include <stm32f10x.h>
#include <core_cm3.h>
#include <rthw.h>

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;
    rt_uint32_t current_delay;

        /*�����ʱ������tick��*/
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));

        /* ��õ�ǰʱ�� */
    delta = SysTick->VAL;

        /* ��ѵ��õ�ǰʱ��,ֱ���ﵽָ��ʱ����˳���ѵ */
    do
    {
        if ( delta > SysTick->VAL )
            current_delay = delta - SysTick->VAL;
        else
        /*��ʱ��Խһ��OS tick�ı߽�ʱ�Ĵ��� */
            current_delay = SysTick->LOAD + delta - SysTick->VAL;
    } while( current_delay < us );
}
