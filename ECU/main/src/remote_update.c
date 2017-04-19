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

int Sendupdatepackage_start(inverter_info *inverter)	//���͵��㿪ʼ���ݰ�
{
	int ret;
	int i=0;
	unsigned char data[256];
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x01;
	sendbuff[3]=0x80;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<10;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg("main","Sendupdatepackage_start");
		ret = zb_get_reply((char *)data,inverter);
		if((0 == ret%8) && (0x18 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			break;
	}

	if(i>=10)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�-1
		return -1;
	else
		return 1;

}

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

	if((1 == inverter->model)||(2 == inverter->model))	//YC250����
		fd=open("/home/UPDATE_YC250.BIN", O_RDONLY,0);
	else if((3 == inverter->model)||(4 == inverter->model))	//YC500����
		fd=open("/home/UPDATE_YC500.BIN", O_RDONLY,0);
	else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN����
		fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY,0);
	else if((7 == inverter->model))		//YC600CN����
		fd=open("/home/UPDATE_YC600.BIN", O_RDONLY,0);
	else
		return -1;		//û�и������

	if(fd>0)
	{
		while(read(fd,package_buff,64)>0){
			sendbuff[4]=package_num/256;
			sendbuff[5]=package_num%256;
			for(i=0;i<64;i++){
				sendbuff[i+6]=package_buff[i];
			}

			for(i=2; i<70; i++)
				check = check + sendbuff[i];

			printdecmsg("main","check",check);

			sendbuff[70]=check >> 8;			//У��
			sendbuff[71]=check;

			zb_send_cmd(inverter,(char *)sendbuff,74);
			package_num++;
			printdecmsg("main","package_num",package_num);
			printhexmsg("main","package_msg", (char *)sendbuff, 74);
			memset(package_buff, 0, sizeof(package_buff));
			check = 0x00;
		}
		close(fd);
		return 1;
	}
	else
		return -2;		//���ļ�ʧ��
}

int Complementupdatepackage_single(inverter_info *inverter)	//���©�������ݰ�������
{
	int fd;
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
		printmsg("main","Complementupdatepackage_single_checkbuff");
		ret = zb_get_reply((char *)data,inverter);
		printdecmsg("main","ret",ret);
		i++;
	}while((12!=ret)&&(i<5));		//(-1==ret)

	if((12 == ret) && (0x42 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))
	{

		if((1 == inverter->model)||(2 == inverter->model))	//YC250����
			fd=open("/home/UPDATE_YC250.BIN", O_RDONLY,0);
		else if((3 == inverter->model)||(4 == inverter->model))	//YC500����
			fd=open("/home/UPDATE_YC500.BIN", O_RDONLY,0);
		else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN����
			fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY,0);
		else if((7 == inverter->model))		//YC600CN����
			fd=open("/home/UPDATE_YC600.BIN", O_RDONLY,0);

		if(fd>0){
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
					printmsg("main","Complementupdatepackage single 10 times failed");
					return -1;		//������3��û��Ӧ�����
				}
				printdecmsg("main","Complement_package",(data[6]*256+data[7]));
				rt_hw_us_delay(30000);
				check = 0x00;
			}
		return 1;	//�ɹ�����
		}
		close(fd);
	}
	else if((12 == ret) && (0x45 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))//�����������512����ֱ�ӻ�ԭ
	{
		printmsg("main","Complementupdatepackage over 512");
		return -1;
	}
	else
	{
		printmsg("main","Complement checkbuff no response");
		return 0;	//������checkָ��û��Ӧ�����
	}

}

int Update_start(inverter_info *inverter)		//���͸���ָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={};
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
		printmsg("main","Update_start");
		ret = zb_get_reply_update_start((char *)data,inverter);
		if((8 == ret) && (0x05 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//���³ɹ�
			return 1;
		if((8 == ret) && (0xe5 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//����ʧ�ܣ���ԭ
			return -1;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0
		return 0;

}

int Update_success_end(inverter_info *inverter)		//���³ɹ�����ָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={};
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
		printmsg("main","Update_success_end");
		ret = zb_get_reply((char *)data,inverter);
		if((0 == ret%8) && (0x3C == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			break;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�-1
		return -1;
	else
	{
		printmsg("main","Update_success_end successful");
		return 1;
	}

}

int Restore(inverter_info *inverter)		//���ͻ�ԭָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={};
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
		printmsg("main","Restore");
		ret = zb_get_reply_restore((char *)data,inverter);
		if((8 == ret) && (0x06 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭ�ɹ�
			return 1;
		if((8 == ret) && (0xe6 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭʧ��
			return -1;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0
		return 0;
}

/* ������̨�����
 *
 * ����ֵ��	0 �����ɹ�
 * 			1 ���Ϳ�ʼ�������ݰ�ʧ��
 * 		 	2 û�ж�Ӧ�ͺŵ������
 * 		 	3 �������ļ�ʧ��
 * 		 	4 ����ʧ��
 * 		 	5 ��������Ӧ
 * 		 	6 ����ʧ��
 * 		 	7 ��������Ӧ
 */
int remote_update_single(inverter_info *inverter)
{
	int ret_sendsingle,ret_complement,ret_update_start=0;
	char flag;
	int i;

	Update_success_end(inverter);
	if(1==Sendupdatepackage_start(inverter))
	{
		printmsg("main","Sendupdatepackage_start_OK");
		ret_sendsingle = Sendupdatepackage_single(inverter);

		if(-1==ret_sendsingle)		//û�ж�Ӧ�ͺŵ������
		{
			printmsg("main","No corresponding BIN file");
			return 2;
		}
		else if(-2==ret_sendsingle)		//�������ļ�ʧ��
		{
			Restore(inverter);
			Update_success_end(inverter);
			return 3;
		}
		else if(1==ret_sendsingle)		//�ļ�������ȫ�����
		{
			printmsg("main","Complementupdatepackage_single");
			ret_complement = Complementupdatepackage_single(inverter); //���©�������ݰ�������

			if(-1==ret_complement)		//����������3��û��Ӧ�Ͳ�������ֱ�ӳ���512�������
			{
				Restore(inverter);
				Update_success_end(inverter);
				return 4; //����ʧ��
			}
			else if(1==ret_complement)		//�ɹ�������ȫ�����
			{
				for(i=0;i<10;i++)
				{
					ret_update_start=Update_start(inverter); //���Ϳ�ʼ����ָ��
					if(1==ret_update_start)
						break;
				}
				if(1==ret_update_start)
				{
					printmsg("main","Update start successful");
					Update_success_end(inverter);
					return 0; //�����ɹ�
				}
				else if(-1==ret_update_start)
				{
					printmsg("main","Update_start_failed");
					Restore(inverter);
					Update_success_end(inverter);
					return 6; //����ʧ��
				}
				else if(0==ret_update_start)
				{
					printmsg("main","Update start no response");
					return 7; //��������Ӧ
				}
			}
			else if(0==ret_complement)
			{
				return 5; //��������Ӧ
			}
		}
	}
	else{
		return 1; //���Ϳ�ʼ�������ݰ�ʧ��
	}
}

int remote_update_result(char *id, int version, int update_result)
{
	char inverter_result[128];

	sprintf(inverter_result, "%s%05d%02dEND", id, version, update_result);
	save_inverter_parameters_result2(id, 135, inverter_result);
	return 0;
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
	sqlite3 *db=NULL;
	char *zErrMsg = 0;
	int i=0, nrow = 0, ncolumn = 0, j=0;
	char **azResult;
	char sql[1024]={'\0'};
	int update_result = 0;

	inverter_info *curinverter = firstinverter;

	sqlite3_open("/home/database.db", &db);


	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
	{
		sprintf(sql, "SELECT id FROM update_inverter WHERE id='%s' AND update_flag=1", curinverter->id);
		sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		//print2msg("zErrMsg=",zErrMsg);

		if(1==nrow)
		{
			printmsg(curinverter->id);
			if(curinverter->updating==0)
			{
				if(1 == zb_shutdown_single(curinverter))
				{
					curinverter->updating=1;
					curinverter->updating_time=time(NULL);
				}
				continue;
			}
			else
			{
				if((time(NULL)-curinverter->updating_time)<1800)
					continue;
				else curinverter->updating=0;
			}
			update_result = remote_update_single(curinverter);
			sprintf(sql,"UPDATE update_inverter SET update_flag=0 WHERE id='%s' ",curinverter->id);
			for(j=0;j<3;j++)
			{
				if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
					break;
				sleep(1);
			}
			for(j=0;j<3;j++)
			{
				if(1 == zb_query_inverter_info(curinverter)) //��ȡ������汾��
				{
					update_inverter_model_version(curinverter);
					break;
				}
			}
			if(j>=3)update_result += 10;
			remote_update_result(curinverter->id, curinverter->version, update_result);
		}
		sqlite3_free_table( azResult );

	}

	sqlite3_close( db );

}

