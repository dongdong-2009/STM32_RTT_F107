#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include "variation.h"

//����������������ݰ���ECU��EMA��ͨ��Э��ת������Э��
int protocol_APS18(struct inverter_info_t *firstinverter, char *sendcommanddatetime);

/* ������쳣״̬A123 */
int protocol_status(struct inverter_info_t *firstinverter, char *datetime);

/*���汾���¼�*/
int saveevent(inverter_info *inverter, char *sendcommanddatatime);
#endif  /*__PROTOCOL_H__*/
