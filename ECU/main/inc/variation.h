#ifndef __VARIATION_H__
#define __VARIATION_H__

#define MAXINVERTERCOUNT 120	//�����������
#define RECORDLENGTH 150		//�Ӽ�¼�ĳ���
#define RECORDTAIL 100			//���¼�Ľ�β��������������ʱ�����Ϣ
#define TNUIDLENGTH 7			//�����3501ID�ĳ���
#define idLEN 13		//�����ID�ĳ���
#define STATUSLENGTH 3			//�������ǰ״̬�ĳ���
#define DVLENGTH 5				//ֱ����ѹ�ĳ��ȣ�EMAͨ��Э����ʹ��
#define DILENGTH 3				//ֱ�������ĳ��ȣ�EMAͨ��Э����ʹ��
#define POWERLENGTH 5			//���ʳ��ȣ�EMAͨ��Э����ʹ��
#define REACTIVEPOWERLENGTH 5	//�޹����ʳ��ȣ�EMAͨ��Э����ʹ��
#define ACTIVEPOWERLENGTH 5		//�й����ʳ��ȣ�EMAͨ��Э����ʹ��
#define FREQUENCYLENGTH 5		//����Ƶ�ʳ��ȣ�EMAͨ��Э����ʹ��
#define TEMPERATURELENGTH 3		//�¶ȳ��ȣ�EMAͨ��Э����ʹ��
#define GRIDVOLTLENGTH 3		//������ѹ���ȣ�EMAͨ��Э����ʹ��
#define CURGENLENGTH 6			//��ǰһ�ַ������ĳ��ȣ�EMAͨ��Э����ʹ��
#define SYSTEMPOWERLEN 10		//ϵͳ���ʣ�EMAͨ��Э����ʹ��
#define CURSYSGENLEN 10			//ϵͳ��ǰ��������EMAͨ��Э����ʹ��
#define LIFETIMEGENLEN 10		//��ʷ��������EMAͨ��Э����ʹ��

#define PROCESS_RESULT_HEAD			51		
#define PROCESS_RESULT_RECORD_LEN			16
#define STATUS_PER_LEN				64

#define INVARTER_STATUS_PER_LEN			16
#define INVARTER_STATUS_PER_OTHER			16

#define INVERTER_PHONE_PER_LEN			27
#define INVERTER_PHONE_PER_OTHER			100


//Client ���ͨ�Ų���
#define CLIENT_RECORD_HEAD							20
#define CLIENT_RECORD_ECU_HEAD					78
#define CLIENT_RECORD_INVERTER_LENGTH		104
#define CLIENT_RECORD_OTHER							100

#define CONTROL_RECORD_HEAD							18
#define CONTROL_RECORD_ECU_HEAD					33
#define CONTROL_RECORD_INVERTER_LENGTH	41
#define CONTROL_RECORD_OTHER						100

typedef struct
{
    unsigned int deputy_model:4;				//�������루600��1��BB��2��B1��
	unsigned int dataflag:1;					//1��ʾ������ǰ���ݣ�0��ʾ��ȡ����ʧ��
	unsigned int bindflag:1;					//������󶨶̵�ַ��־��1��ʾ�󶨣�0��ʾδ��
	unsigned int flag:1;						//id�е�flag��־
	unsigned int response_protection_paras_one:1;	//��̨���ñ��������Ƿ���Ҫ�ϱ���־

	unsigned int last_turn_on_off_flag:1;
	unsigned int turn_on_off_changed_flag:1;		//0 1
	unsigned int last_gfdi_flag:1;
	unsigned int gfdi_changed_flag:1;				//0 1
	unsigned int fill_up_data_flag:3;				//0 1 2 3			//������Ƿ��в����ݹ��ܵı�־λ��1Ϊ�й���,2Ϊû�й��ܣ�Ĭ��0Ϊû����Ӧ���ߵ�һ��
	unsigned int updating:1;						//0 1

}status_t;

typedef struct inverter_info_t{
	char id[13];		//�������ID
	unsigned short shortaddr;			//Zigbee�Ķ̵�ַ
	
	unsigned char model;					//���ͣ�1��YC250CN,2��YC250NA��3��YC500CN��4��YC500NA��5��YC900CN��6��YC900NA 7��YC600
	status_t inverterstatus;		//λ��ĸ���״̬
	int version;				//����汾��(��zigbee  zb_query_inverter_info����)	
	unsigned char signalstrength;			//�����Zigbee�ź�ǿ��
	unsigned char raduis;			
	
	float dv;					//ֱ����ѹ
	float di;					//ֱ������
	int op;						//�������
	float gf;					//����Ƶ��
	int gv;						//������ѹ
	int it;						//�����¶�
	/********B·����*****************/
	float dvb;					//ֱ����ѹ
	float dib;					//ֱ������
	int opb;					//�������
	int gvb;					//������ѹ
	/********C·����*****************/
	float dvc;					//ֱ����ѹ
	float dic;					//ֱ������
	int opc;					//�������
	int gvc;					//������ѹ
	/********D·����*****************/
	float dvd;					//ֱ����ѹ
	float did;					//ֱ������
	int opd;					//�������
	int gvd;					//������ѹ

	float reactive_power;		//�޹�����
	float reactive_powerb;		//B·�޹�����
	float reactive_powerc;		//C·�޹�����
	float active_power;			//�й�����
	float active_powerb;		//B·�й�����
	float active_powerc;		//C·�й�����
	float output_energy;		//���������ֵ
	float output_energyb;		//B·���������ֵ
	float output_energyc;		//C·���������ֵ
	float pre_output_energy;	//��һ�ֶ������������
	float pre_output_energyb;	//B·��һ�ֶ������������
	float pre_output_energyc;	//C·��һ�ֶ������������
	float cur_output_energy;	//���ֶ������������
	float cur_output_energyb;	//B·���ֶ������������
	float cur_output_energyc;	//C·���ֶ������������

	float curgeneration;		//�������ǰһ�ֵĵ���
	float curgenerationb;		//�����B·��ǰһ�ֵĵ���
	float curgenerationc;		//�����C·��ǰһ�ֵĵ���
	float curgenerationd;		//�����D·��ǰһ�ֵĵ���

	float preaccgen;			//A·��һ�ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float preaccgenb;			//B·��һ�ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float preaccgenc;			//C·��һ�ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float preaccgend;			//D·��һ�ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float curaccgen;			//A·���ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float curaccgenb;			//B·���ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float curaccgenc;			//C·���ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	float curaccgend;			//D·���ַ��ص��ۼƵ���������������������ͣ�ۼƣ�ֱ����������
	int preacctime;				//��һ�ַ��ص��ۼ�ʱ�䣨�����������ʱ�䲻ͣ�ۼƣ�ֱ����������
	int curacctime;				//���ַ��ص��ۼ�ʱ�䣨�����������ʱ�䲻ͣ�ۼƣ�ֱ����������

	char status_web[50];		//����ECU�������ݿ��״̬�����ڱ���ҳ����ʾ
	char status[12];			//�����״̬
	char statusb[12];			//�����B·״̬
	char statusc[12];			//�����C·״̬
	char statusd[12];			//�����D·״̬
	char status_ema[64];		//16������¼�
	char status_send_flag;		//16������¼�

	char last_report_time[16];	//���͸�EMAʱ�����ں�ʱ�䣬��ʽ��������ʱ����
	unsigned char no_getdata_num;					//unsigned char(��������255)����û�л�ȡ����������ݵĴ���
	unsigned char disconnect_times;				//unsigned char(��������255) һ����û���������ͨ���ϵ����д��� ZK
	unsigned char zigbee_version;					//unsigned char   zigbee�汾��ZK		//turned_off_rpt_flag

	int updating_time;
	
}inverter_info;

typedef struct ecu_info_t{
	char id[13];				//ECU��ID
	//char ccuid[8];				//Zigbee/3501��ID
	unsigned short panid;				//Zigbee��panid
	char channel;				//Zigbee�ŵ�
	char ip[20];
	float life_energy;			//ϵͳ��ʷ�ܵ���
	float current_energy;		//ϵͳ��ǰһ�ֵ���
	float today_energy;			//����ķ�����
	int system_power;			//ϵͳ�ܹ���
	int count;					//ϵͳ��ǰһ�������ݵ��������
	int total;					//ϵͳ���������
	int assigned_shortaddr_count;	//�Ѿ���ȡ���̵�ַ�����������

	int type;					//ECU����:1��ʾNA��0��ʾSAA
	int zoneflag;				//�޸Ĺ�ʱ���ı�־��1��ʾ�޸ģ�0��ʾδ�޸�
	char MacAddress[7];			//ECU  MAC��ַ
	char broadcast_time[16];	//���͸�EMAʱ�����ں�ʱ�䣬��ʽ��������ʱ����
	char had_data_broadcast_time[16];	//���͸�EMAʱ�����ں�ʱ�䣬��ʽ��������ʱ����
	char last_ema_time[16];		//client ���һ�κ�clientͨ��ʱ��
//	char inverterid_noget_shortaddr[MAXINVERTERCOUNT][13];	//û�л�ȡ���̵�ַ�������ID

	int no_assigned_shortaddr_count;	//û�л�ȡ���̵�ַ�����������
	int flag_ten_clock_getshortaddr;	//ÿ��10����û�����»�ȡ�̵�ַ��־
	int polling_total_times;			//ECUһ��֮���ܵ���ѯ���� ZK
}ecu_info;


#endif /*__VARIATION_H__*/
