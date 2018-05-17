/****************************************Copyright (c)****************************************************
**                             �� �� �� �� �� �� �� �� �� �� �� ˾
**                                http://www.6lowpanworld.com
**                                http://hichard.taobao.com
**
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:           rtreg.h
** Last modified Date:  2014-12-23
** Last Version:        V1.0
** Description:         ֱ�ӷ���Ӳ���Ĵ����ĺ������壬��Ҫ�û���д�豸��������
** 
**--------------------------------------------------------------------------------------------------------
** Created By:          Renhaibo�κ���
** Created date:        2014-12-23
** Version:             V1.0
** Descriptions:        The original version ��ʼ�汾
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#ifndef __RT_REG_H__
#define __RT_REG_H__

#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************************************************
** ͷ�ļ�����
*********************************************************************************************************/
#include "rtdef.h"

/*********************************************************************************************************
** ����궨��
*********************************************************************************************************/
#define RT_MACRO_START     do {
#define RT_MACRO_END       } while (0)

/*********************************************************************************************************
** ���ʼĴ����ĺ궨��
*********************************************************************************************************/
/*********************************************************************************************************
** ֱ�ӽ��Ĵ���ӳ��
*********************************************************************************************************/
#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))

/*********************************************************************************************************
** �ο�ecos��ֲ�����ģ�ͨ����������
*********************************************************************************************************/
//  8λ���ʼĴ����ĺ궨��
#define HAL_READ_UINT8( _register_, _value_ )       \
    ((_value_) = *((volatile rt_uint8_t *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ )      \
    (*((volatile rt_uint8_t *)(_register_)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    (_buf_)[_i_] = ((volatile rt_uint8_t *)(_register_))[_j_];      \
    RT_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    ((volatile rt_uint8_t *)(_register_))[_j_] = (_buf_)[_i_];      \
    RT_MACRO_END

#define HAL_READ_UINT8_STRING( _register_, _buf_, _count_ )             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    (_buf_)[_i_] = ((volatile rt_uint8_t *)(_register_))[_i_];      \
    RT_MACRO_END

#define HAL_WRITE_UINT8_STRING( _register_, _buf_, _count_ )            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    ((volatile rt_uint8_t *)(_register_)) = (_buf_)[_i_];           \
    RT_MACRO_END

//  16λ���ʼĴ����ĺ궨��
#define HAL_READ_UINT16( _register_, _value_ )      \
    ((_value_) = *((volatile rt_uint16_t *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ )     \
    (*((volatile rt_uint16_t *)(_register_)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    (_buf_)[_i_] = ((volatile rt_uint16_t *)(_register_))[_j_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    ((volatile rt_uint16_t *)(_register_))[_j_] = (_buf_)[_i_];     \
    RT_MACRO_END

#define HAL_READ_UINT16_STRING( _register_, _buf_, _count_)             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    (_buf_)[_i_] = ((volatile rt_uint16_t *)(_register_))[_i_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT16_STRING( _register_, _buf_, _count_)            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    ((volatile rt_uint16_t *)(_register_))[_i_] = (_buf_)[_i_];     \
    RT_MACRO_END

// 32λ���ʼĴ����ĺ궨��
#define HAL_READ_UINT32( _register_, _value_ )      \
    ((_value_) = *((volatile rt_uint32_t *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ )     \
    (*((volatile rt_uint32_t *)(_register_)) = (_value_))

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    (_buf_)[_i_] = ((volatile rt_uint32_t *)(_register_))[_j_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
    ((volatile rt_uint32_t *)(_register_))[_j_] = (_buf_)[_i_];     \
    RT_MACRO_END

#define HAL_READ_UINT32_STRING( _register_, _buf_, _count_)             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    (_buf_)[_i_] = ((volatile rt_uint32_t *)(_register_))[_i_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT32_STRING( _register_, _buf_, _count_)            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
    ((volatile rt_uint32_t *)(_register_))[_i_] = (_buf_)[_i_];     \
    RT_MACRO_END

#ifdef __cplusplus
}
#endif      // __cplusplus

#endif // endif of __RT_REG_H__
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
