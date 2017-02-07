#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__
#include "variation.h"

extern int openzigbee(void);
extern int zb_shortaddr_cmd(int shortaddr, char *buff, int length);		//zigbee �̵�ַ��ͷ
extern int zb_shortaddr_reply(char *data,int shortaddr,char *id);			//��ȡ������ķ���֡,�̵�ַģʽ
extern int zb_get_reply(char *data,inverter_info *inverter);			//��ȡ������ķ���֡
extern int zb_get_reply_update_start(char *data,inverter_info *inverter);			//��ȡ�����Զ�̸��µ�Update_start����֡��ZK��������Ӧʱ�䶨Ϊ10��
extern int zb_get_reply_restore(char *data,inverter_info *inverter);			//��ȡ�����Զ�̸���ʧ�ܣ���ԭָ���ķ���֡��ZK����Ϊ��ԭʱ��Ƚϳ������Ե���дһ������
extern int zb_get_reply_from_module(char *data);			//��ȡzigbeeģ��ķ���֡
extern int zb_get_id(char *data);			//��ȡ�����ID
extern int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter);			//�ر��޶�����������ϱ�ID����
extern int zb_turnoff_rptid(int short_addr);			//�رյ���������ϱ�ID����
extern int zb_get_inverter_shortaddress_single(inverter_info *inverter);			//��ȡ��ָ̨��������̵�ַ��ZK
extern int zb_turnon_rtpid(inverter_info *firstinverter);			//����������Զ��ϱ�ID
extern int zb_change_inverter_panid_broadcast(void);	//�㲥�ı��������PANID��ZK
extern int zb_change_inverter_panid_single(inverter_info *inverter);	//����ı��������PANID���ŵ���ZK
extern int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter);	//���㻹ԭ�������PANID��0X8888���ŵ�0X10��ZK
extern int zb_change_ecu_panid(void);		//����ECU��PANID���ŵ�
extern int zb_restore_ecu_panid_0x8888(void);			//�ָ�ECU��PANIDΪ0x8888,ZK
extern int zb_restore_ecu_panid_0xffff(int channel); 		//����ECU��PANIDΪ0xFFFF,�ŵ�Ϊָ���ŵ�(ע:�������������������ʱ,�轫ECU��PANID��Ϊ0xFFFF)
extern int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee��ͷ
extern int zb_broadcast_cmd(char *buff, int length);		//zigbee�㲥��ͷ
extern int zb_query_inverter_info(inverter_info *inverter);		//����������Ļ�����
extern int zb_query_data(inverter_info *inverter);		//���������ʵʱ����
extern int zb_test_communication(void);		//zigbee����ͨ����û�жϿ�
extern int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter);		//�����޸�CCָ��
extern int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DA_reply);		//�洢������ѯDDָ��
extern int zb_afd_broadcast(void);		//AFD�㲥ָ��
extern int zb_turnon_inverter_broadcast(void);		//����ָ��㲥,OK
extern int zb_boot_single(inverter_info *inverter);		//����ָ���,OK
extern int zb_shutdown_broadcast(void);		//�ػ�ָ��㲥,OK
extern int zb_shutdown_single(inverter_info *inverter);		//�ػ�ָ���,OK
extern int zb_boot_waitingtime_single(inverter_info *inverter);		//�����ȴ�ʱ���������Ƶ���,OK
extern int zb_clear_gfdi_broadcast(void);		//���GFDI�㲥,OK
extern int zb_clear_gfdi(inverter_info *inverter);		//���GFDI,OK
extern int zb_ipp_broadcast(void);		//IPP�㲥
extern int zb_ipp_single(inverter_info *inverter);		//IPP����,��ʱ����,ZK
extern int zb_frequency_protectime_broadcast(void);		//ǷƵ����ʱ��㲥
extern int zb_frequency_protectime_single(inverter_info *inverter);		//ǷƵ����ʱ�䵥��
extern int zb_voltage_protectime_broadcast(void);		//Ƿѹ����ʱ��㲥
extern int zb_voltage_protectime_single(inverter_info *inverter);		//Ƿѹ����ʱ�䵥��

		
#endif /*__ZIGBEE_H__*/
