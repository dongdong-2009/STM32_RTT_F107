#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__
#include "variation.h"

int selectZigbee(int timeout);			//zigbee�������ݼ�� ����0 ��ʾ����û������  ����1��ʾ����������
int openzigbee(void);
void zigbee_reset(void);
int zb_shortaddr_cmd(int shortaddr, char *buff, int length);		//zigbee �̵�ַ��ͷ
int zb_shortaddr_reply(char *data,int shortaddr,char *id);			//��ȡ������ķ���֡,�̵�ַģʽ
int zb_get_reply(char *data,inverter_info *inverter);			//��ȡ������ķ���֡
int zb_get_reply_update_start(char *data,inverter_info *inverter);			//��ȡ�����Զ�̸��µ�Update_start����֡��ZK��������Ӧʱ�䶨Ϊ10��
int zb_get_reply_restore(char *data,inverter_info *inverter);			//��ȡ�����Զ�̸���ʧ�ܣ���ԭָ���ķ���֡��ZK����Ϊ��ԭʱ��Ƚϳ������Ե���дһ������
int zb_get_reply_from_module(char *data);			//��ȡzigbeeģ��ķ���֡
int zb_get_id(char *data);			//��ȡ�����ID
int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter);			//�ر��޶�����������ϱ�ID����
int zb_turnoff_rptid(int short_addr);			//�رյ���������ϱ�ID����
int zb_get_inverter_shortaddress_single(inverter_info *inverter);			//��ȡ��ָ̨��������̵�ַ��ZK
int zb_turnon_rtpid(inverter_info *firstinverter);			//����������Զ��ϱ�ID
int zb_change_inverter_panid_broadcast(void);	//�㲥�ı��������PANID��ZK
int zb_change_inverter_panid_single(inverter_info *inverter);	//����ı��������PANID���ŵ���ZK
int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter);	//���㻹ԭ�������PANID��0X8888���ŵ�0X10��ZK
int zb_change_ecu_panid(void);		//����ECU��PANID���ŵ�
int zb_restore_ecu_panid_0x8888(void);			//�ָ�ECU��PANIDΪ0x8888,ZK
int zb_restore_ecu_panid_0xffff(int channel); 		//����ECU��PANIDΪ0xFFFF,�ŵ�Ϊָ���ŵ�(ע:�������������������ʱ,�轫ECU��PANID��Ϊ0xFFFF)
int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee��ͷ
int zb_broadcast_cmd(char *buff, int length);		//zigbee�㲥��ͷ
int zb_query_inverter_info(inverter_info *inverter);		//����������Ļ�����
int zb_query_data(inverter_info *inverter);		//���������ʵʱ����
int zb_test_communication(void);		//zigbee����ͨ����û�жϿ�
int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter);		//�����޸�CCָ��
int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DA_reply);		//�洢������ѯDDָ��
int zb_afd_broadcast(void);		//AFD�㲥ָ��
int zb_turnon_inverter_broadcast(void);		//����ָ��㲥,OK
int zb_boot_single(inverter_info *inverter);		//����ָ���,OK
int zb_shutdown_broadcast(void);		//�ػ�ָ��㲥,OK
int zb_shutdown_single(inverter_info *inverter);		//�ػ�ָ���,OK
int zb_boot_waitingtime_single(inverter_info *inverter);		//�����ȴ�ʱ���������Ƶ���,OK
int zb_clear_gfdi_broadcast(void);		//���GFDI�㲥,OK
int zb_clear_gfdi(inverter_info *inverter);		//���GFDI,OK
int zb_ipp_broadcast(void);		//IPP�㲥
int zb_ipp_single(inverter_info *inverter);		//IPP����,��ʱ����,ZK
int zb_frequency_protectime_broadcast(void);		//ǷƵ����ʱ��㲥
int zb_frequency_protectime_single(inverter_info *inverter);		//ǷƵ����ʱ�䵥��
int zb_voltage_protectime_broadcast(void);		//Ƿѹ����ʱ��㲥
int zb_voltage_protectime_single(inverter_info *inverter);		//Ƿѹ����ʱ�䵥��

		
#endif /*__ZIGBEE_H__*/
