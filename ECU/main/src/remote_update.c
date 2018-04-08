/*****************************************************************************/
/*  File      : remote_update.c                                              */
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
#include <stdio.h>
#include <stdlib.h>
#include <dfs_posix.h> 
#include <string.h>
#include "variation.h"
#include "debug.h"
#include "zigbee.h"
#include <string.h>
#include "myfile.h"
#include "file.h"
#include <dfs_posix.h> 
#include "debug.h"
#include "rthw.h"
#include "datetime.h"
#include "crc.h"
#include "rtc.h"
#include "variation.h"

extern inverter_info inverter[MAXINVERTERCOUNT];

int remote_update_fd = -1;		//������ͨ���ļ�������


/*********************************************************************
upinv����ֶ�:
id,update_result,update_time,update_flag
**********************************************************************/

typedef enum  remotetype{
  Remote_UpdateSuccessful    	= 0,
  Remote_UpdateFailed_SendStart	= 1,
  Remote_UpdateFailed_FillingPackageStartNoResponse   	= 2,
  Remote_UpdateFailed_FillingPackageNoResponse   	= 3,
  Remote_UpdateFailed_Other	= 4,
  Remote_UpdateFailed_ModelUnMatch     	= 5,
  Remote_UpdateFailed_OpenFile    	= 6,
  Remote_UpdateFailed_CRC	= 7,
  Remote_UpdateFailed_CRCNoResponse     	= 8,
  Remote_UpdateFailed_GetSector     	= 9,
  Remote_UpdateFailed_UpdateSector     	= 10,
}eRemoteType;

//���������صĴ���ת��Ϊ���͸�EMA�Ĵ����
int getResult(int ret)
{
	if(0 == ret)
	{
		return Remote_UpdateSuccessful;
	}else if(1 == ret)
	{
		return Remote_UpdateFailed_SendStart;
	}else if(2 == ret)
	{
		return Remote_UpdateFailed_ModelUnMatch;
	}else if(3 == ret)
	{
		return Remote_UpdateFailed_OpenFile;
	}else if(4 == ret)
	{
		return Remote_UpdateFailed_FillingPackageStartNoResponse;
	}else if(5 == ret)
	{
		return Remote_UpdateFailed_FillingPackageNoResponse;
	}else if(6 == ret)
	{
		return Remote_UpdateFailed_CRC;
	}else if(7 == ret)
	{	
		return Remote_UpdateFailed_CRCNoResponse;
	}else if(8 == ret)
	{	
		return Remote_UpdateFailed_GetSector;
	}else if(9 == ret)
	{	
		return Remote_UpdateFailed_UpdateSector;
	}else
	{
		return ret;
	}
}


/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                        */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -1 : ����ʧ��                                                           */
/*    32 ���·��ɹ�,���� �ϵ���������                                                         */
/*    24 ���·��ɹ��������ڶϵ���������                                                          */
/*****************************************************************************/
int Sendupdatepackage_start(inverter_info *inverter)	//������ʼ֡
{
	int ret;
	int i=0,crc=0;
	unsigned char data[256];
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x01;
	sendbuff[3]=0x80;
	crc=crc_array(&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<10;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_MAIN,"Sendupdatepackage_start");
		ret = zb_get_reply((char *)data,inverter);
		if((0 == ret%8) && (0x01 == data[2]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			return data[3];
	}
	return -1;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -2 : û��BIN�ļ�                                                        */
/*   -1 : ECU��֧�ָû�����������                                            */
/*    1 ���·��������ɹ�                                                           */
/*****************************************************************************/
int Sendupdatepackage_single(inverter_info *inverter)	//���͵������ݰ�
{
	int fd;
	int i=0,package_num=0;
	unsigned char package_buff[100];
	unsigned char sendbuff[74]={0xff};
	unsigned short check = 0x00;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x40;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN����
		fd=open("/ftp/UPYC1000.BIN", O_RDONLY,0);
	else if((7 == inverter->model))		//YC600CN����
		fd=open("/ftp/UPYC600.BIN", O_RDONLY,0);
	else if((23 == inverter->model))		//YC600CN����
		fd=open("/ftp/UPQS1200.BIN", O_RDONLY,0);
	else
		return -1;		//û�и������

	if(fd>=0)
	{
		while(read(fd,package_buff,64)>0){
			sendbuff[4]=package_num/256;
			sendbuff[5]=package_num%256;
			for(i=0;i<64;i++){
				sendbuff[i+6]=package_buff[i];
			}

			for(i=2; i<70; i++)
				check = check + sendbuff[i];

			printdecmsg(ECU_DBG_MAIN,"check",check);

			sendbuff[70]=check >> 8;			//У��
			sendbuff[71]=check;

			zb_send_cmd(inverter,(char *)sendbuff,74);
			package_num++;
			printdecmsg(ECU_DBG_MAIN,"package_num",package_num);
			printhexmsg(ECU_DBG_MAIN,"package_msg", (char *)sendbuff, 74);
			memset(package_buff, 0, sizeof(package_buff));
			check = 0x00;
		}
		close(fd);
		return 1;
	}
	else
		return -2;		//���ļ�ʧ��
}


/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -2 :������������                                            */
/*   -1 :������7��û��Ӧ�����                                         */
/*   0 : ������ʼָ���޻�Ӧ                                                        */
/*    1 ���ɹ�����                                                   */
/*****************************************************************************/
int Complementupdatepackage_single(inverter_info *inverter)	//���©�������ݰ�������
{
	int fd = -1;
	int ret;
	int i=0,k=0;
	unsigned char data[256];
	unsigned char checkbuff[74]={0x00};
	unsigned char sendbuff[74]={0xff};
	unsigned char package_buff[100];
	unsigned short check = 0x00;

	checkbuff[0]=0xfc;
	checkbuff[1]=0xfc;
	checkbuff[2]=0x04;
	checkbuff[3]=0x20;
	checkbuff[72]=0xfe;
	checkbuff[73]=0xfe;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x4f;
//	sendbuff[70]=0x00;
//	sendbuff[71]=0x00;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	clear_zbmodem();
	do{
		zb_send_cmd(inverter,(char *)checkbuff,74);
		printmsg(ECU_DBG_MAIN,"Complementupdatepackage_single_checkbuff");
		ret = zb_get_reply((char *)data,inverter);
		printdecmsg(ECU_DBG_MAIN,"ret",ret);
		i++;
	}while((12!=ret)&&(i<5));		//(-1==ret)

	if((12 == ret) && (0x42 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))
	{

 		if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN����
			fd=open("/ftp/UPYC1000.BIN", O_RDONLY,0);
		else if((7 == inverter->model))		//YC600CN����
			fd=open("/ftp/UPYC600.BIN", O_RDONLY,0);
		else if((23 == inverter->model))		//YC600CN����
			fd=open("/ftp/UPQS1200.BIN", O_RDONLY,0);
		else
			;

		if(fd>=0){
			while((data[6]*256+data[7])>0)
			{
				lseek(fd,(data[4]*256+data[5])*64,SEEK_SET);
				memset(package_buff, 0, sizeof(package_buff));
				read(fd, package_buff, 64);
				sendbuff[4]=data[4];
				sendbuff[5]=data[5];
				for(k=0;k<64;k++){
					sendbuff[k+6]=package_buff[k];
				}

				for(i=2; i<70; i++)
					check = check + sendbuff[i];

				sendbuff[70]=check >> 8;			//У��
				sendbuff[71]=check;

				for(i=0;i<10;i++)
				{
					zb_send_cmd(inverter,(char *)sendbuff,74);
					ret = zb_get_reply((char *)data,inverter);
					if((12 == ret) && (0x24 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))
						break;
				}
				if(i>=10)
				{
					printmsg(ECU_DBG_MAIN,"Complementupdatepackage single 10 times failed");
					close(fd);
					return -1;		//������3��û��Ӧ�����
				}
				printdecmsg(ECU_DBG_MAIN,"Complement_package",(data[6]*256+data[7]));
				rt_thread_delay(3);
				check = 0x00;
			}
			close(fd);
			return 1;	//�ɹ�����
		}
		else
		{
			return -2;	//������BIN�ļ�
		}
		
	}
	else
	{
		printmsg(ECU_DBG_MAIN,"Complement checkbuff no response");
		return 0;	//������checkָ��û��Ӧ�����
	}

}
/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -1 :��ʼ����ʧ��                                         */
/*     0 : ��ʼ������ʱ                                                    */
/*    1 ����ʼ�����ɹ�                                                 */
/*****************************************************************************/
int Update_start(inverter_info *inverter)		//���͸���ָ��
{
	int ret;
	int i=0;
	unsigned char data[256];
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x05;
	sendbuff[3]=0xa0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_MAIN,"Update_start");
		ret = zb_get_reply_update_start((char *)data,inverter);
		if((8 == ret) && (0x05 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//���³ɹ�
			return 1;
		if((8 == ret) && (0xe5 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//����ʧ�ܣ���ԭ
			return -1;
	}

	return 0;						//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0



}

int Update_success_end(inverter_info *inverter)		//�˳�Զ��������
{
	int ret;
	int i=0;
	unsigned char data[256]={0x00};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x03;
	sendbuff[3]=0xc0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_MAIN,"Update_success_end");
		ret = zb_get_reply((char *)data,inverter);
		if((0 == ret%8) && (0x3C == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			break;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�-1
		return -1;
	else
	{
		printmsg(ECU_DBG_MAIN,"Update_success_end successful");
		return 1;
	}

}

int Restore(inverter_info *inverter)		//���ͻ�ԭָ��
{
	int ret;
	int i=0;
	unsigned char data[256];
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x06;
	sendbuff[3]=0x60;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_MAIN,"Restore");
		ret = zb_get_reply_restore((char *)data,inverter);
		if((8 == ret) && (0x06 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭ�ɹ�
			return 1;
		if((8 == ret) && (0xe6 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭʧ��
			return -1;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0
		return 0;
	return 0;
}

int save_sector(char *id,int sector)
{
	FILE *fp;
	fp=fopen("/home/sector.con","a");
	fprintf(fp,"%s--%d\n",id,sector);
	fclose(fp);
	return 0;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -3 :��ȡ����ʧ��                                         */
/*   -2 :�����ļ�������                                         */
/*   -1 :û�ж�Ӧ�ͺŵ������                                         */
/*     other : ��ȡ����������Ҫ����������                                                    */
/*****************************************************************************/
int set_update_new(inverter_info *inverter,int *sector_all,unsigned short *crc_update_file)
{
	int K1=1,K6=0;
	unsigned short crc=0;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	int i,ret,sector_num;
	int pos = 0;
	FILE *fp = NULL;
	
	if(remote_update_fd >= 0)
	{
		close(remote_update_fd);
		remote_update_fd = -1;
	}
	
	//���ݲ�ͬ�Ļ����룬�򿪲�ͬ���ļ�������У��ֵ
	if(inverter->model==7)
	{
		remote_update_fd=open("/ftp/UPYC600.BIN",O_RDONLY, 0);
		crc=crc_file(remote_update_fd);
		*crc_update_file = crc;
	}
	else if(inverter->model==23)
	{
		remote_update_fd=open("/ftp/UPQS1200.BIN",O_RDONLY, 0);
		crc=crc_file(remote_update_fd);
		*crc_update_file = crc;	
	}
	else if((inverter->model==5)||(inverter->model==6))
	{
		remote_update_fd=open("/ftp/UPYC1000.BIN",O_RDONLY, 0);
		crc=crc_file(remote_update_fd);
		*crc_update_file = crc;
	}
	else
	{
		printmsg(ECU_DBG_MAIN,"Inverter's model Error");
		*crc_update_file = 0;
		return -1;
	}

	
	if(remote_update_fd < 0)
	{
		printmsg(ECU_DBG_MAIN,"No BIN");
		return -2;
	}
	
	pos = lseek(remote_update_fd,0,SEEK_END);		//�鿴�ļ���С
	
	//close(remote_update_fd);
	sector_num=pos/4096;
	*sector_all=sector_num;

	fp=fopen("/yuneng/upwork.con","r");	//1:No working;0:working
	if(fp!=NULL)
	{
		K1=fgetc(fp)-0x30;
		fclose(fp);
	}
	fp=fopen("/yuneng/upoutime.con","r");		//time_out,С��5min��Ĭ��5min
	if(fp!=NULL)
	{
		K6=fgetc(fp)-0x30;
		fclose(fp);
	}


	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x08;
	sendbuff[3] = 0x10;
	sendbuff[4] = K1;
	sendbuff[5] = 0x01;
	sendbuff[6] = 0x00;
	sendbuff[7] = sector_num/256;
	sendbuff[8] = sector_num%256;
	sendbuff[9] = K6;
	sendbuff[10] = crc/256;
	sendbuff[11] = crc%256;

	crc=crc_array((unsigned char *)&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	sendbuff[72]=0xFE;
	sendbuff[73]=0xFE;

	for(i=0;i<15;i++)
	{
		memset(readbuff,'\0',256);
		zb_send_cmd(inverter,sendbuff,74);
		printhexmsg(ECU_DBG_MAIN,"set_update",sendbuff,74);
		ret = zb_get_reply(readbuff,inverter);
		if(ret%13==0)
			break;
	}
	if((0 == ret%13) && (0x06 == readbuff[2]) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0xFE == readbuff[11]) && (0xFE == readbuff[12]))
	{
		crc=crc_array((unsigned char *)&readbuff[2],7);
		if((readbuff[9]==crc/256)&&(readbuff[10]==crc%256))
		{
			return readbuff[5]*256+readbuff[6];	//���ؽ�������Ҫ������Ӧ����
		}

	}
	printmsg(ECU_DBG_MAIN,"Failed to set_update");
	return -3;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -1 :�ļ�������                                         */
/*     0 : ���ͳɹ�                                                    */
/*****************************************************************************/
int send_package_to_single_new(inverter_info *inverter, int speed,int cur_sector,char *file_name,unsigned short *crc_4k)
{

	int i, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	int package_count;
	int crc;
	unsigned short crc_4k_temp = 0xFFFF;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x40;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;
	if(remote_update_fd < 0)
	{
		close(remote_update_fd);
		remote_update_fd = -1;
		return -1;
	}else
	{
		lseek(remote_update_fd,cur_sector*4096,SEEK_SET);

		read(remote_update_fd,package_buff,speed);
		for(package_count=4096/speed;package_count>0;package_count--)
		{
			
			sendbuff[4]=(package_num+(4096*cur_sector)/speed)/256;
			sendbuff[5]=(package_num+(4096*cur_sector)/speed)%256;

			for(i=0;i<speed;i++){
				sendbuff[i+6]=package_buff[i];
			}
			crc=crc_array((unsigned char *)&sendbuff[2],68);
			sendbuff[6+speed]=crc/256;
			sendbuff[7+speed]=crc%256;


			zb_send_cmd(inverter,sendbuff,74);
			for(i = 0;i<64;i++)
			{
				crc_4k_temp = UpdateCRC(crc_4k_temp, package_buff[i]);
			}
			read(remote_update_fd,package_buff,speed);
			package_num++;
			//printdecmsg(ECU_DBG_MAIN,"package_num",package_num);
			//printhexmsg(ECU_DBG_MAIN,"sendbuff", sendbuff, 10+speed);

			
		}
		*crc_4k = crc_4k_temp;
		return 0;
	}
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -4 :CRCУ��ʧ��                                         */
/*     9 : 4KB����У׼ʧ��                                                    */
/*     10 : 4KB����У׼�ɹ�                                                    */
/*     19 :���������ļ�У׼ʧ��                                                 */
/*     20 : ���������ļ�У׼�ɹ�                                                   */
/*****************************************************************************/
int send_crc_cmd(inverter_info *inverter,unsigned short crc_result,int file_or_4k,int sector)
{
	int i,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x07;
	sendbuff[3] = 0xE0;
	sendbuff[4] = file_or_4k;
	sendbuff[5] = sector/256;
	sendbuff[6] = sector%256;
	sendbuff[7] = crc_result/256;
	sendbuff[8] = crc_result%256;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;

	crc=crc_array((unsigned char *)&sendbuff[2],68);
	sendbuff[70] = crc/256;
	sendbuff[71] = crc%256;
	//printhexmsg(ECU_DBG_MAIN,inverter->id, sendbuff, 74);
	if(file_or_4k==1)
	{
		for(i=0;i<5;i++)
		{
			//rt_thread_delay(100);
			zb_send_cmd(inverter,sendbuff,74);
			printhexmsg(ECU_DBG_MAIN,"***CRC",sendbuff,74);
			if(13 == zb_get_reply_new(readbuff,inverter,90))
				break;
		}
	}
	else
	{
		for(i=0;i<5;i++)
		{
			//rt_thread_delay(100);
			zb_send_cmd(inverter,sendbuff,74);
			printhexmsg(ECU_DBG_MAIN,"***CRC",sendbuff,74);
			if(13 == zb_get_reply(readbuff,inverter))
				break;
		}
	}

	if(i>=5)
	{
		printmsg(ECU_DBG_MAIN,"Send crc_4k over 5 times");	//��ѯ������3��û��Ӧ�����
		return -4;
	}
	crc=crc_array((unsigned char *)&readbuff[2],7);
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x06)&&
			(readbuff[3]==0x7E)&&
			(readbuff[9]==(crc/256))&&
			(readbuff[10]==(crc%256))&&
			(readbuff[11]==0xFE)&&
			(readbuff[12]==0xFE)
	)
		{
			if(readbuff[4]==0xA0)	//4KB����У׼�ɹ�
				return 10;
			else if(readbuff[4]==0xA1)	//4KB����У׼ʧ��
				return 9;
			else if(readbuff[4]==0xB0)	//���������ļ�У׼�ɹ�
				return 20;
			else if(readbuff[4]==0xB1)	//���������ļ�У׼ʧ��
				return 19;
			else return -4;
		}
	return -4;

}

int crc_4k(char *file,int sector,inverter_info *inverter,unsigned short crc_update_4k,int flag)
{
	unsigned short result = 0xFFFF;
	char *package_buff = NULL;
	int ret_size,i;

	if(flag == 0)
	{
		package_buff = malloc(4096);
		memset(package_buff,0x00,4096);

		if(remote_update_fd >= 0)
		{
			lseek(remote_update_fd,4096*sector,SEEK_SET);
			ret_size = read(remote_update_fd, package_buff, 4096);
			for(i=0;i<ret_size;i++)
			{
				result = UpdateCRC(result, package_buff[i]);
			}
		}
		
		free(package_buff);
		package_buff = NULL;
		
	}else
	{
		result = crc_update_4k;
	}

	return send_crc_cmd(inverter,result,0,sector);			//0:4k; 1:file_all  return 1����SUCCESS
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -3 :�����ļ�������                                         */
/*   -2 :������ƥ��                                         */
/*   -1 :����������ѯ֡ʧ��                                          */
/*****************************************************************************/
int resend_lost_packets_new(inverter_info *inverter, int speed,int cur_sector,char *file_name,unsigned short crc_update_4k)
{

	int i,ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	int crc;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x04;
	sendbuff[3] = 0x20;
	sendbuff[4] = cur_sector/256;
	sendbuff[5] = cur_sector%256;
	crc=crc_array((unsigned char *)&sendbuff[2],68);
	sendbuff[70] = crc/256;
	sendbuff[71] = crc%256;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;


	printhexmsg(ECU_DBG_MAIN,inverter->id, sendbuff, 74);
	printmsg(ECU_DBG_MAIN,"***bubaowenxun");
	for(i=0;i<15;i++)
	{
		//rt_thread_delay(100);
		zb_send_cmd(inverter,sendbuff,74);
		if(13==zb_get_reply(readbuff,inverter))
			break;
	}

	if(i>=15)
	{
		printmsg(ECU_DBG_MAIN,"Query the lost packet over 5 times");	//��ѯ������5��û��Ӧ�����
		return -1;
	}

	memset(sendbuff,'\0',256);		//��������֡
	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x4F;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;

	crc=crc_array((unsigned char *)&readbuff[2],7);
	//printdecmsg(ECU_DBG_MAIN,"crc",crc);
	if((readbuff[0] == 0xFB) &&
			(readbuff[1] == 0xFB) &&
			(readbuff[2] == 0x06) &&
			(readbuff[3] == 0x42) &&
			(readbuff[9] == (crc/256)) &&
			(readbuff[10] == (crc%256)) &&
			(readbuff[11] == 0xFE) &&
			(readbuff[12] == 0xFE))
	{
		if(readbuff[4] == 0x01)		//������ƥ��
		{
			if((readbuff[7]*256 + readbuff[8]) == 0)	//��ǰ������������Ϊ0
			{
				return crc_4k(file_name,cur_sector,inverter,crc_update_4k,1);		//SUCCESS

			}
			else //if((readbuff[7]*256 + readbuff[8]) > 0)//��ǰ���������������0
			{

				if(remote_update_fd >= 0)
				{
					while((readbuff[7]*256 + readbuff[8]) > 0)
					{
						//rt_thread_delay(200);
						lseek(remote_update_fd,(readbuff[5]*256 + readbuff[6])*speed, SEEK_SET);
						memset(package_buff, 0, sizeof(package_buff));
						read(remote_update_fd, package_buff, speed);
						sendbuff[4] = readbuff[5];
						sendbuff[5] = readbuff[6];
						for(i=0; i<speed; i++)
						{
							sendbuff[i+6] = package_buff[i];
						}

						crc=crc_array((unsigned char *)&sendbuff[2],68);
						sendbuff[6+speed]=crc/256;
						sendbuff[7+speed]=crc%256;

						sendbuff[8+speed] = 0xFE;
						sendbuff[9+speed] = 0xFE;

						for(i=0;i<15;i++)
						{
							zb_send_cmd(inverter,sendbuff,74);
							//printhexmsg(ECU_DBG_MAIN,"sendbuff", sendbuff, 10+speed);

							memset(readbuff, 0, sizeof(readbuff));
							mark++;
							//printdecmsg(ECU_DBG_MAIN,"mark=",mark);
							ret = zb_get_reply(readbuff,inverter);
							if((13 == ret) &&
									(readbuff[0] == 0xFB) &&
									(readbuff[1] == 0xFB) &&
									(readbuff[2] == 0x06) &&
									(readbuff[3] == 0x24) &&
									(readbuff[4] == 0x01) &&				//������ƥ��ɹ�
									(readbuff[11] == 0xFE) &&
									(readbuff[12] == 0xFE))
								break;
						}
						if(i>=15)
						{
							printmsg(ECU_DBG_MAIN,"Resend the lost packet over 5 times");
							resend_packet_flag=0;
							printdecmsg(ECU_DBG_MAIN,"resend_packet_flag", resend_packet_flag);
							return -1;		//������5��û��Ӧ�����
						}
						printdecmsg(ECU_DBG_MAIN,"Resend the lost packet", (readbuff[7]*256 + readbuff[8]));
					}
					resend_packet_flag = 1;
					print2msg(ECU_DBG_MAIN,inverter->id, "All of the lost packets have been resent");
					return crc_4k(file_name,cur_sector,inverter,crc_update_4k,1);

				}
				else
				{
					return -3;
				}
			}

		}
		else //if(readbuff[5] == 0x00)	//������ƥ��
		{
			resend_packet_flag = 1;
			return -2;
		}
	}
	else
	{
		return -1;
	}
}

int crc_bin_file_new(inverter_info *inverter,unsigned short crc_update_file)
{
	return send_crc_cmd(inverter,crc_update_file,1,0);
}


int cover_atob(inverter_info *inverter,int source_addr,int to_addr)
{
	int i,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x36;
	sendbuff[3] = 0x60;
	sendbuff[4] = source_addr;
	sendbuff[5] = to_addr;
	for(i=6;i<70;i++)
		sendbuff[i]=i;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;
	crc=crc_array((unsigned char *)&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	printhexmsg(ECU_DBG_MAIN,inverter->id, sendbuff, 74);
	for(i=0;i<5;i++)
	{
		rt_thread_delay(1000);
		zb_send_cmd(inverter,sendbuff,74);
		printhexmsg(ECU_DBG_MAIN,"***Cover",sendbuff,74);
		if(13 <= zb_get_reply_new(readbuff,inverter,90))
			break;
	}
	if(i>=5)
	{
		printmsg(ECU_DBG_MAIN,"Send Cover_addr over 5 times");	//��ѯ������3��û��Ӧ�����
		return -1;
	}
	crc=crc_array((unsigned char *)&readbuff[2],7);
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x36)&&
			(readbuff[3]==0x66)&&
			(readbuff[9]==(crc/256))&&
			(readbuff[10]==(crc%256))&&
			(readbuff[11]==0xFE)&&
			(readbuff[12]==0xFE)
	)
		{
			if(readbuff[4]==0x00)
				return 0;
			else if(readbuff[4]==0x11)
				return 1;
			else if(readbuff[4]==0x02)
				return 2;
			else if(readbuff[4]==0x10)
				return 3;
			else return -1;
		}
	return -1;

}

int stop_update_new(inverter_info *inverter)
{
	int i, ret=0,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x03;
	sendbuff[3] = 0xC0;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;
	crc=crc_array((unsigned char *)&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;


	for(i=0; i<3; i++){
		zb_send_cmd(inverter,sendbuff,74);
		rt_thread_delay(50);

		ret = zb_get_reply(readbuff,inverter);

		if((13 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x06) &&
				(readbuff[3] == 0x3C) &&
				(readbuff[11] == 0xFE) &&
				(readbuff[12] == 0xFE)
			)
		{
			if(sendbuff[4]==0x00){
				print2msg(ECU_DBG_MAIN,inverter->id, "Stop updating successfully");
				return 0;
			}
			else{
				print2msg(ECU_DBG_MAIN,inverter->id, "Failed to stop updating");
				return 0;
			}
		}
	}

	print2msg(ECU_DBG_MAIN,inverter->id, "Failed to stop updating");
	return -1;
}


void closeRemoteUpdateFD(void)
{
	if(remote_update_fd >= 0)
	{
		close(remote_update_fd);
		remote_update_fd = -1;
	}
}

/* ������̨�����
 *
 * ����ֵ    0 �����ɹ�				
 * 			1 ���Ϳ�ʼ�������ݰ�ʧ��
 * 		 	2 û�ж�Ӧ�ͺŵ������(������δ���������������Ļ����벻֧������)
 * 		 	3 �������ļ�ʧ��(�ļ�������)
 * 		 	4 ������ʼָ��ʧ��
 * 		 	5 ��������Ӧ
 * 		 	6 ����ʧ��CRCУ��ʧ��
 * 		 	7 ��������ӦCRCУ������Ӧ
 *			8 ��ȡ��������ʧ��
 *			9 ��������ʧ��
 *			>=10  ��ʾ��ǰ����ʧ�ܵ����� 
 */
int remote_update_single(inverter_info *inverter)
{
	int ret_sendsingle,ret_complement,ret_update_start=0;
	int i,ret,nxt_sector,j,sector_all,k;
	unsigned short crc_update_file=0;	//�����ļ������ļ���У��ֵ
	unsigned short crc_update_4k=0;
	int cur_crc=0;
	char update_file_name[100]={'\0'};
	if(inverter->model==7)
		sprintf(update_file_name,"/ftp/UPYC600.BIN");
	else if((inverter->model==5)||(inverter->model==6))
		sprintf(update_file_name,"/ftp/UPYC1000.BIN");
	else if(inverter->model==23)
		sprintf(update_file_name,"/ftp/UPQS1200.BIN");
	
	Update_success_end(inverter);
	ret=Sendupdatepackage_start(inverter);	//�ж������Ƿ���ϵ���������
	if(32==ret)	//���ڶϵ���������
	{
		nxt_sector=set_update_new(inverter,&sector_all,&crc_update_file);	//��ȡ��������Ҫ���͵�����
		if(nxt_sector >= 0)
		{
			for(j=nxt_sector;j<sector_all;j++)
			{
				printdecmsg(ECU_DBG_MAIN,"Sector:",j);
				if(!send_package_to_single_new(inverter,64,j,update_file_name,&crc_update_4k))	//��������֡
				{
					ret=resend_lost_packets_new(inverter, 64,j,update_file_name,crc_update_4k);	//����������ѯ֡
					if(ret==10)	//4KУ��ɹ�
					{
						cur_crc=0;
						continue;
					}
					if((ret==9)&&(cur_crc<3))
					{
						j--;		//�����ڵ�ǰ����
						cur_crc++;
						continue;
					}
				}
				//save_sector(inverter->id,j);
				stop_update_new(inverter);	//��������
				if(j<10)
					return 10;
				else
					return j;
			}
			cur_crc=0;
			ret=crc_bin_file_new(inverter,crc_update_file);
			if(ret==19)	//�����ļ�У��ʧ��
			{
				for(j=0;j<sector_all;j++)	//ÿ����������У��
				{
					if(10!=crc_4k(update_file_name,j,inverter,crc_update_4k,0))	//����У��ʧ��
					{

						if(!send_package_to_single_new(inverter,64,j,update_file_name,&crc_update_4k))
						{
							ret=resend_lost_packets_new(inverter, 64,j,update_file_name,crc_update_4k);printf("ret==%d\n",ret);
							if(ret==10)													//�����ɹ�
							{
								cur_crc=0;
								ret=crc_bin_file_new(inverter,crc_update_file);
								if(ret==20)
									break;
								else continue;
							}
							if((ret==9)&&(cur_crc<10))
							{
								j--;
								cur_crc++;
								continue;
							}
							break;
						}
					}

				}
			}
			if(ret==20)	//�����ļ�У��ɹ�
			{
				for(k = 0;k<10;k++)
				{
					ret=cover_atob(inverter,2,0);
					if(0 == ret)
						break;
				}

				if(ret!=0)
					cover_atob(inverter,1,0);	//ԭ�����򸲸�

				if(ret==0)
				{
					print2msg(ECU_DBG_MAIN,inverter->id,"UPDATE SUCCESS!");
					stop_update_new(inverter);
					return 0;
				}
				else// if(ret==1)
				{
					print2msg(ECU_DBG_MAIN,inverter->id,"RELOADING FAILED!");
					stop_update_new(inverter);
					return 6;
				}
				
			}
			stop_update_new(inverter);
			return 6;//����У�鲻��
		}
		else if(-1 == nxt_sector)
		{
			//����֡���أ��ϵ���������
			return 2;
		}else if(-2 == nxt_sector)
		{	
			return 3;
		}else //if(-3 == nxt_sector)
		{
			return 8;
		}
	}
	else if(24==ret)	//�����ڶϵ���������
	{
		printmsg(ECU_DBG_MAIN,"Sendupdatepackage_start_OK");
		ret_sendsingle = Sendupdatepackage_single(inverter);

		if(-1==ret_sendsingle)		//û�ж�Ӧ�ͺŵ������
		{
			printmsg(ECU_DBG_MAIN,"No corresponding BIN file");
			return 2;
		}
		else if(-2==ret_sendsingle)		//�������ļ�ʧ��
		{
			Restore(inverter);
			Update_success_end(inverter);
			return 3;
		}
		else //if(1==ret_sendsingle)		//�ļ�������ȫ�����
		{
			printmsg(ECU_DBG_MAIN,"Complementupdatepackage_single");
			ret_complement = Complementupdatepackage_single(inverter); //���©�������ݰ�������

			if(-1==ret_complement)		//����������3��û��Ӧ�Ͳ�������ֱ�ӳ���512�������
			{
				Restore(inverter);
				Update_success_end(inverter);
				return 5; //����ʧ��
			}
			else if(1==ret_complement)		//�ɹ�������ȫ�����
			{
				for(i=0;i<10;i++)
				{
					ret_update_start=Update_start(inverter); //���Ϳ�ʼ����ָ��
					if(1==ret_update_start)	//��ʼ����
						break;
				}
				
				if(1==ret_update_start)
				{
					printmsg(ECU_DBG_MAIN,"Update successful");
					Update_success_end(inverter);
					return 0; //�����ɹ�
				}
				else if(-1==ret_update_start)
				{
					printmsg(ECU_DBG_MAIN,"Update_failed");
					Restore(inverter);
					Update_success_end(inverter);
					return 6; //����ʧ��
				}
				else if(0==ret_update_start)
				{
					printmsg(ECU_DBG_MAIN,"Update no response");
					return 7; //��������Ӧ
				}
				else return 7;
			}
			else if(0==ret_complement)
			{
				return 4; //������ʼָ������Ӧ
			}
			else
				return 3;
		}
		
	}
	else{
		return 1; //���Ϳ�ʼ�������ݰ�ʧ��
	}
}


/* ���������
 *
 * update_result��
 * 0 �����ɹ�
 * 1 ���Ϳ�ʼ�������ݰ�ʧ��
 * 2 û�ж�Ӧ�ͺŵ������
 * 3 �������ļ�ʧ��
 * 4 ����ʧ��
 * 5 ��������Ӧ
 * 6 ����ʧ��
 * 7 ��������Ӧ
 * 1+ ��ȡ�汾��ʧ��
 */
int remote_update(inverter_info *firstinverter)
{
	int i=0, j=0,version_ret = 0,k = 0;
	int update_result = 0;
	char data[200];
	char splitdata[4][32];
	char Time[15] = {"/0"};
	char pre_Time[15] = {"/0"};
	char inverter_result[128];
	int remoteTypeRet = Remote_UpdateSuccessful; 
	inverter_info *curinverter = firstinverter;
	unsigned  updateNum = 0;	//��Ҫ���������Ĵ���

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
	{
		//��ȡ����ID��
		if(1 == read_line("/home/data/upinv",data,curinverter->id,12))
		{
			splitString(data,splitdata);
			memset(data,0x00,200);
			updateNum = atoi(splitdata[3]);
			if(updateNum >= 1)	//��Ҫ�����Ĵ�������1�����������������ɹ��˸�Ϊ0 ����ʧ�ܼ�1
			{
				printmsg(ECU_DBG_MAIN,curinverter->id);
				if(curinverter->model==7)		//YC600��Ҫ�رհ�Сʱ���ϲ��ܽ�������,������Ŀ�����Ӧ
				{
					if(curinverter->inverterstatus.updating==0)
					{
						if(1 == zb_shutdown_single(curinverter))
						{
							curinverter->inverterstatus.updating=1;
							curinverter->updating_time=acquire_time();
							printdecmsg(ECU_DBG_MAIN,"shutdown time",curinverter->updating_time);
						}
						//continue;
					}
					else
					{	
						if(compareTime(acquire_time() ,curinverter->updating_time,1800))
							continue;
						else curinverter->inverterstatus.updating=0;
					}
				}

				for(k=0;k<3;k++)
				{
					if(-1==zb_test_communication())
						zigbee_reset();
					else
						break;
				}
				apstime(pre_Time);
				update_result = remote_update_single(curinverter);
				closeRemoteUpdateFD();
				printdecmsg(ECU_DBG_MAIN,"Update",update_result);
				apstime(Time);

				if(0 == update_result)	//�����ɹ�
				{	//ֻ�гɹ��˲Ų�ѯ�汾��
					//sprintf(data,"%s,%s,%s,0\n",curinverter->id,splitdata[1],splitdata[2]);
					for(j=0;j<5;j++)
					{
						if(1 == zb_query_inverter_info(curinverter)) //��ȡ������汾��
						{
							version_ret = 1;
							break;
						}
					}

					if(1 == version_ret)	//��ȡ���汾��
					{
						sprintf(data,"%s,%d,%s,0\n",curinverter->id,curinverter->version,Time);
					}else		//δ��ȡ���汾��
					{
						sprintf(data,"%s,%d,%s,0\n",curinverter->id,0,Time);
					}
				}else	//����ʧ��
				{
					sprintf(data,"%s,%d,%s,%d\n",curinverter->id,curinverter->version,Time,(updateNum-1));
				}

				
				//ɾ��ID������
				delete_line("/home/data/upinv","/home/data/upinv.t",curinverter->id,12);
				
				for(j=0;j<3;j++)
				{		
					if(1 == insert_line("/home/data/upinv",data))
						break;
					rt_thread_delay(100);
				}
				remoteTypeRet = getResult(update_result);
				sprintf(inverter_result, "%s%02d%06d%sEND", curinverter->id, remoteTypeRet,curinverter->version, Time);
				save_inverter_parameters_result2(curinverter->id, 147,inverter_result);

#if 0
				memset(inverter_result,0x00,128);
				sprintf(inverter_result, "%s,%02d,%06d,%s,%s\n", curinverter->id, remoteTypeRet,curinverter->version, pre_Time,Time);
				for(j=0;j<3;j++)
				{		
					if(1 == insert_line("/tmp/update.tst",inverter_result))
						break;
					rt_thread_delay(RT_TICK_PER_SECOND);
				}
#endif

			}
		}
	}	
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void remote(void)
{
	remote_update(inverter);
}
FINSH_FUNCTION_EXPORT(remote, eg:remote());

void testfile1(const char * file)
{
	int fd,i;
	char Time[15] = {"/0"};
	char pre_Time[15] = {"/0"};
	apstime(pre_Time);
	for(i = 0;i<15616;i++)
	{
		fd = open(file, O_RDONLY,0);
		if(fd>=0)
		{
			lseek(fd,4096*(i/10),SEEK_SET);

			close(fd);
		}else
		{
			printf("open file failed");
		}
	}
	apstime(Time);

	printf("%s %s \n",pre_Time,Time);

}
FINSH_FUNCTION_EXPORT(testfile1, eg:testfile1());


void testfile2(const char * file)
{
	int fd,i;
	char Time[15] = {"/0"};
	char pre_Time[15] = {"/0"};
	apstime(pre_Time);
	fd = open(file, O_RDONLY,0);
	for(i = 0;i<15616;i++)
	{
		
		if(fd>=0)
		{
			lseek(fd,4096*(i/10),SEEK_SET);

			
		}else
		{
			printf("open file failed");
		}
	}
	close(fd);
	apstime(Time);

	printf("%s %s \n",pre_Time,Time);

}
FINSH_FUNCTION_EXPORT(testfile2, eg:testfile2());
#endif

