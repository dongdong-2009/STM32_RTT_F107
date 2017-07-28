/*****************************************************************************/
/*  File      : main-thread.c                                                */
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
#include "main-thread.h"
#include <board.h>
#include "zigbee.h"
#include "resolve.h"
#include "variation.h"
#include "checkdata.h"
#include "rtc.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"
#include "ema_control.h"
#include "bind_inverters.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_update.h"
#include "version.h"
#include "threadlist.h"
#include "debug.h"
#include "SEGGER_RTT.h"
#include "client.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
inverter_info inverter[MAXINVERTERCOUNT];
ecu_info ecu;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int init_ecu()
{
	get_ecuid(ecu.id);
	//��ȡpanid
	ecu.panid = get_panid();
	//��ȡECU�ŵ�
	ecu.channel = get_channel();
	
	memset(ecu.had_data_broadcast_time,'\0',16);
	rt_memset(ecu.ip, '\0', sizeof(ecu.ip));
	ecu.life_energy = get_lifetime_power();
	ecu.current_energy = 0;
	ecu.system_power = 0;
	ecu.count = 0;
	ecu.total = 0;
	ecu.flag_ten_clock_getshortaddr = 1;			//ZK
	ecu.polling_total_times=0;					//ECUһ��֮����ѵ�Ĵ�����0, ZK
	ecu.type = 0;
	ecu.zoneflag = 0;				//ʱ��
	printecuinfo(&ecu);
	zb_change_ecu_panid();
	readconnecttime();
	return 1;
}

int init_inverter(inverter_info *inverter)
{
	int i;
	char flag_limitedid = '0';				//�޶�ID��־
	FILE *fp;
	inverter_info *curinverter = inverter;
	
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		rt_memset(curinverter->id, '\0', sizeof(curinverter->id));		//��������UID
		rt_memset(curinverter->tnuid, '\0', sizeof(curinverter->tnuid));			//��������ID

		curinverter->model = 0;

		curinverter->dv=0;			//��յ�ǰһ��ֱ����ѹ
		curinverter->di=0;			//��յ�ǰһ��ֱ������
		curinverter->op=0;			//��յ�ǰ������������
		curinverter->gf=0;			//��յ���Ƶ��
		curinverter->it=0;			//���������¶�
		curinverter->gv=0;			//��յ�����ѹ
		curinverter->dvb=0;			//B·��յ�ǰһ��ֱ����ѹ
		curinverter->dib=0;			//B·��յ�ǰһ��ֱ������
		curinverter->opb=0;			//B·��յ�ǰ������������
		curinverter->gvb=0;
		curinverter->dvc=0;
		curinverter->dic=0;
		curinverter->opc=0;
		curinverter->gvc=0;
		curinverter->dvd=0;
		curinverter->did=0;
		curinverter->opd=0;
		curinverter->gvd=0;



		curinverter->curgeneration = 0;	//����������ǰһ�ַ�����
		curinverter->curgenerationb = 0;	//B·��յ�ǰһ�ַ�����

		curinverter->preaccgen = 0;
		curinverter->preaccgenb = 0;
		curinverter->curaccgen = 0;
		curinverter->curaccgenb = 0;
		curinverter->preacctime = 0;
		curinverter->curacctime = 0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//��������״̬
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));		//��������״̬
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));		//B·��������״̬

		curinverter->dataflag = 0;		//��һ�������ݵı�־��λ
	//	curinverter->bindflag=0;		//���������־λ����0
		curinverter->no_getdata_num=0;	//ZK,���������ȡ�����Ĵ���
		curinverter->disconnect_times=0;		//û���������ͨ���ϵĴ�����0, ZK
		curinverter->signalstrength=0;			//�ź�ǿ�ȳ�ʼ��Ϊ0

		curinverter->updating=0;
		curinverter->raduis=0;
	}

	get_ecu_type();		//��ȡECU�ͺ�
	

	while(1) {
		ecu.total = get_id_from_file(inverter);
		if (ecu.total > 0) {
			break; //ֱ���������������0ʱ�˳�ѭ��
		} else {
			printmsg(ECU_DBG_MAIN,"please Input Inverter ID---------->"); //��ʾ�û����������ID
			rt_thread_delay(20*RT_TICK_PER_SECOND);
		}
	}

	fp = fopen("/yuneng/limiteid.con", "r");
	if(fp)
	{
		flag_limitedid = fgetc(fp);
		fclose(fp);
	}
	
	if ('1' == flag_limitedid) {
		bind_inverters(); //�������
		fp = fopen("/yuneng/limiteid.con", "w");
		if (fp) {
			fputs("0", fp);
			fclose(fp);
		}
	}
	return 1;
}

int init_all(inverter_info *inverter)
{
	openzigbee();
	zb_test_communication();
	init_ecu();
	init_inverter(inverter);
	read_gfdi_turn_on_off_status(inverter);
	return 0;
}

int reset_inverter(inverter_info *inverter)
{
	int i;
	inverter_info *curinverter = inverter;

	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		curinverter->dataflag = 0;

		curinverter->dv=0;
		curinverter->di=0;
		curinverter->op=0;
		curinverter->gf=0;
		curinverter->it=0;
		curinverter->gv=0;

		curinverter->dvb=0;
		curinverter->dib=0;
		curinverter->opb=0;

		curinverter->curgeneration = 0;
		curinverter->curgenerationb = 0;
		curinverter->status_send_flag=0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//????????
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));
	}

	return 1;
}

void main_thread_entry(void* parameter)
{
	int thistime=0, durabletime=65535, reportinterval=300;					//thistime:��������������͹㲥Ҫ���ݵ�ʱ��;durabletime:ECU�����������Ҫ���ݵĳ���ʱ��
	char broadcast_hour_minute[3]={'\0'};									//����������͹㲥����ʱ��ʱ��
	int cur_time_hour;														//��ǰ��ʱ��Сʱ

	
	
#if ECU_JLINK_DEBUG
	SEGGER_RTT_printf(0,"\n---********** main.exe %s_%s_%s **********---\n", ECU_VERSION,MAJORVERSION,MINORVERSION);
#endif

#if ECU_DEBUG
#if ECU_DEBUG_MAIN
	printf("\n---********** main.exe %s_%s_%s **********---\n", ECU_VERSION,MAJORVERSION,MINORVERSION);
#endif
#endif
	init_all(inverter);   //��ʼ�����������
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_MAIN);
	printmsg(ECU_DBG_MAIN,"Start-------------------------------------------------");
	
	while(1)
	{
		if(compareTime(durabletime ,thistime,reportinterval)){
		//if(compareTime(durabletime ,thistime,60)){
			//printf("******1-----------main acquire_time:\n");
			thistime = acquire_time();
			rt_memset(ecu.broadcast_time, '\0', sizeof(ecu.broadcast_time));				//��ձ��ι㲥ʱ��

			cur_time_hour = get_time(ecu.broadcast_time, broadcast_hour_minute);					//���»�ȡ���ι㲥�¼�

			printmsg(ECU_DBG_MAIN,"****************************************");
			print2msg(ECU_DBG_MAIN,"ecu.broadcast_time",ecu.broadcast_time);
			
			ecu.count = getalldata(inverter);			//��ȡ�������������,���ص�ǰ�����ݵ����������

			
			//��������һ�ֲɼ����ݵ�ʱ��
			memcpy(ecu.had_data_broadcast_time,ecu.broadcast_time,16);
			
			//printdecmsg(ECU_DBG_MAIN,"ecu.count",ecu.count);
			ecu.life_energy = ecu.life_energy + ecu.current_energy;				//����ϵͳ��ʷ������
			printfloatmsg(ECU_DBG_MAIN,"ecu.life_energy",ecu.life_energy);
			update_life_energy(ecu.life_energy);								//����ϵͳ��ʷ������
		
			
			if(ecu.count>0)
			{
				ecu.system_power = 277;
				ecu.current_energy = 0.23;
				save_system_power(ecu.system_power,ecu.broadcast_time);			//����ϵͳ����
				update_daily_energy(ecu.current_energy,ecu.broadcast_time);		//����ÿ�շ�����
				update_monthly_energy(ecu.current_energy,ecu.broadcast_time);	//����ÿ�µķ�����
			}
			printf("today energy:%f\n",ecu.today_energy);
						
			optimizeFileSystem();
			if(ecu.count>0)
			{
				protocol_APS18(inverter, ecu.broadcast_time);
				protocol_status(inverter, ecu.broadcast_time);
				saveevent(inverter, ecu.broadcast_time);							//���浱ǰһ��������¼�
			}
			/*
			if(ecu.count>0)
			{
				displayonweb(inverter, ecu.broadcast_time);								//ʵʱ����ҳ������
			}
			*/
//			printinverterinfo(&inverter);										//��ӡ�����������Ϣ,ZK
//			format(inverter, ecu.broadcast_time, ecu.system_power, ecu.current_energy, ecu.life_energy);
			
			reset_inverter(inverter);											//����ÿ�������
			
			remote_update(inverter);
			
			if((cur_time_hour>9)&&(1 == ecu.flag_ten_clock_getshortaddr))
			{
				get_inverter_shortaddress(inverter);
				if(ecu.polling_total_times>3)
				{
					ecu.flag_ten_clock_getshortaddr = 0;							//ÿ��10��ִ�������»�ȡ�̵�ַ���־λ��Ϊ0
				}
			}

			//������ѵû�����ݵ�������������»�ȡ�̵�ַ����
			bind_nodata_inverter(inverter);
			process_all(inverter);
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		durabletime = acquire_time();				//�����ѵһ�ߵ�ʱ�䲻��5����,��ôһֱ�ȵ�5��������ѵ��һ��,����5������ȴ�10���ӡ�����5��������

		if((durabletime-thistime)<=305)
			reportinterval = 300;
		else if((durabletime-thistime)<=600)
			reportinterval = 600;
		else
			reportinterval = 900;
	}
	
}
