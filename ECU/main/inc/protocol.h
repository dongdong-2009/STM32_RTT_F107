#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
/*****************************************************************************/
/*  File      : protocol.h                                                   */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "variation.h"

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//把所有逆变器的数据按照ECU和EMA的通信协议转换，见协议
int protocol_APS18(struct inverter_info_t *firstinverter, char *sendcommanddatetime);

/* 逆变器异常状态A123 */
int protocol_status(struct inverter_info_t *firstinverter, char *datetime);

/*保存本地事件*/
int saveevent(inverter_info *inverter, char *sendcommanddatatime);
void save_alarm_event(inverter_info *inverter, char *date_time);
#endif  /*__PROTOCOL_H__*/
