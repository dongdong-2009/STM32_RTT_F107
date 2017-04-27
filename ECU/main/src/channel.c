#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include <dfs_posix.h> 
#include "channel.h"
#include "zigbee.h"
#include "file.h"

extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];

/* �ŵ����� */
int process_channel()
{
	FILE *fp;
	int oldChannel, newChannel;

	if (channel_need_change()) {

		//��ȡ���ǰ����ŵ�
		oldChannel = getOldChannel();
		newChannel = getNewChannel();

		//�޸��ŵ�
		changeChannelOfInverters(oldChannel, newChannel);

		//��ECU�����ŵ����������ļ�
		saveECUChannel(newChannel);

		//��ձ�־λ
		fp = fopen("/tmp/changech.con", "w");
		fclose(fp);
	}
	return 0;
}

/* �ж��Ƿ���Ҫ�ı��ŵ� */
int channel_need_change()
{
	FILE *fp;
	char buff[2] = {'\0'};

	fp = fopen("/tmp/changech.con", "r");
	if (fp) {
		fgets(buff, 2, fp);
		fclose(fp);
	}

	return ('1' == buff[0]);
}

// ��ȡ�ŵ�����Χ��11~26��16���ŵ�
int getOldChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/old_chan.con", "r");
	if (fp) {
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 0; //δ֪�ŵ�
}
int getNewChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/new_chan.con", "r");
	if (fp) {
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 16; //Ĭ���ŵ�
}

/*
void saveECUChannel(int channel)
{
	FILE *fp;
	char buffer[5] = {'\0'};

	snprintf(buffer, sizeof(buffer), "0x%02X", channel);
	printf("%s\n", buffer);
	fp = fopen("/yuneng/channel.con", "w");
	if (fp) {
		fputs(buffer, fp);
		fclose(fp);
	}
	ecu.channel = channel;
}
*/

int saveECUChannel(int channel)
{
	FILE *fp;
	char buffer[5] = {'\0'};

	snprintf(buffer, sizeof(buffer), "0x%02X", channel);
	printf("%s\n", buffer);
	fp = fopen("/yuneng/channel.con", "w");
	if (fp) {
		echo("/yuneng/limiteid.con","1");
		fputs(buffer, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}


void changeChannelOfInverters(int oldChannel, int newChannel)
{
	int num = 0,i = 0,nChannel;
	inverter_info *curinverter = inverter;
	FILE *fp;
	//��ȡ�����ID
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)			//��Ч�������ѵ
	{
		if(curinverter->flag == 1)
			num++;
	}

	//�����ŵ�
	if (num > 0) {
		//ԭ�ŵ���֪
		if (oldChannel) {
			//����ECU�ŵ�Ϊԭ�ŵ�
			zb_restore_ecu_panid_0xffff(oldChannel);

			//����ÿ̨������ŵ�
			curinverter = inverter;
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)			//��Ч�������ѵ
			{
				if(curinverter->flag == 1)
				{
					zb_change_inverter_channel_one(curinverter->id, newChannel);
					curinverter->flag = 0;
				}	
		
			}
		}
		//ԭ�ŵ�δ֪
		else {
			//ECU��ÿһ���ŵ�����ÿһ̨��������͸����ŵ���ָ��
			for (nChannel=11; nChannel<=26; nChannel++ ) {
				zb_restore_ecu_panid_0xffff(nChannel);
				curinverter = inverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)			//��Ч�������ѵ
				{
					if(curinverter->flag == 1)
					{
						zb_change_inverter_channel_one(curinverter->id, newChannel);
						curinverter->flag = 0;
					}	
				}
			}
		}
	}
	//��������浽/home/data/id�ļ���
	fp = fopen("/home/data/id","w");
	if(fp)
	{
		curinverter = inverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)			//��Ч�������ѵ
		{
			fprintf(fp,"%s,%d,%d,%d,%d,%d,%d\n",curinverter->id,curinverter->shortaddr,curinverter->model,curinverter->version,curinverter->bindflag,curinverter->zigbee_version,curinverter->flag);
			
		}
		fclose(fp);
	}

}
