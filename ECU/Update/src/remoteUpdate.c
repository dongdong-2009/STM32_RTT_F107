/*****************************************************************************/
/* File      : remoteUpdate.c                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-11 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "remoteUpdate.h"
#include <rtthread.h>
#include "thftpapi.h"
#include "flash_if.h"
#include "dfs_posix.h"
#include "rthw.h"
#include "myfile.h"
#include "version.h"
#include "threadlist.h"
#include "debug.h"
#include "file.h"
#include "datetime.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define UPDATE_PATH_SUFFIX "ecu-r-m3.bin"
#define UPDATE_PATH "/FTP/ecu.bin"

#define UPDATE_PATH_YC600_SUFFIX "YC600.BIN"
#define UPDATE_PATH_YC600_TEMP "/FTP/YC600UP.BIN"
#define UPDATE_PATH_YC600 "/ftp/UPYC600.BIN"

#define UPDATE_PATH_YC1000_SUFFIX "YC1000.BIN"
#define UPDATE_PATH_YC1000_TEMP "/FTP/YC1000UP.BIN"
#define UPDATE_PATH_YC1000 "/ftp/UPYC1000.BIN"

#define UPDATE_PATH_QS1200_SUFFIX "QS1200.BIN"
#define UPDATE_PATH_QS1200_TEMP "/FTP/QS1200.BIN"
#define UPDATE_PATH_QS1200 "/ftp/UPQS1200.BIN"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//IDWrite ��������ECU  (ͨ���汾��)
//����0��ʾ�ɹ�
int updateECUByVersion_Local(char *Domain,char *IP,int port,char *User,char *passwd)
{
	int ret = 0;
	char remote_path[100] = {'\0'};

	print2msg(ECU_DBG_UPDATE,"Domain",Domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IP);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",User);
	print2msg(ECU_DBG_UPDATE,"password",passwd);
	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/V%s.%s/%s",MAJORVERSION,MINORVERSION,UPDATE_PATH_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"VER Path",remote_path);
	ret=ftpgetfile(Domain,IP, port, User, passwd,remote_path,UPDATE_PATH);
	if(!ret)
	{
		//��ȡ���ļ������и���
		FLASH_Unlock();
		FLASH_If_Erase_APP2();
		FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
		unlink(UPDATE_PATH);

	}else
	{
		unlink(UPDATE_PATH);
	}
	return ret;
}

//IDWrite ��������ECU  (ͨ��ID��)
//����0��ʾ�ɹ�
int updateECUByID_Local(char *Domain,char *IP,int port,char *User,char *passwd)
{
	int ret = 0;
	char remote_path[100] = {'\0'};

	print2msg(ECU_DBG_UPDATE,"Domain",Domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IP);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",User);
	print2msg(ECU_DBG_UPDATE,"password",passwd);	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/%s/%s",ecu.id,UPDATE_PATH_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"ID Path",remote_path);
	ret=ftpgetfile(Domain,IP, port, User, passwd,remote_path,UPDATE_PATH);
	if(!ret)
	{
		//��ȡ���ļ������и���
		FLASH_Unlock();
		FLASH_If_Erase_APP2();
		FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
		deletefile(remote_path);
		unlink(UPDATE_PATH);
	}else
	{
		unlink(UPDATE_PATH);
	}
	return ret;
}


int updateECUByVersion(void)
{
	int ret = 0;
	char domain[100]={'\0'};
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port=0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	getFTPConf(domain,IPFTPadd,&port,user,password);

	print2msg(ECU_DBG_UPDATE,"Domain",domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IPFTPadd);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);
	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/V%s.%s/%s",MAJORVERSION,MINORVERSION,UPDATE_PATH_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"VER Path",remote_path);
	ret=ftpgetfile(domain,IPFTPadd, port, user, password,remote_path,UPDATE_PATH);
	if(!ret)
	{
		//��ȡ���ļ������и���
		FLASH_Unlock();
		FLASH_If_Erase_APP2();
		FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
		unlink(UPDATE_PATH);
		echo("/TMP/ECUUPVER.CON","1");
		reboot();
	}else
	{
		unlink(UPDATE_PATH);
	}
	return ret;
}

int updateECUByID(void)
{
	int ret = 0;
	char domain[100]={'\0'};
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port = 0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	
	getFTPConf(domain,IPFTPadd,&port,user,password);

	print2msg(ECU_DBG_UPDATE,"Domain",domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IPFTPadd);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/%s/%s",ecu.id,UPDATE_PATH_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"ID Path",remote_path);
	ret=ftpgetfile(domain,IPFTPadd, port, user, password,remote_path,UPDATE_PATH);
	if(!ret)
	{
		//��ȡ���ļ������и���
		FLASH_Unlock();
		FLASH_If_Erase_APP2();
		FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
		deletefile(remote_path);
		unlink(UPDATE_PATH);
		echo("/TMP/ECUUPVER.CON","1");
		reboot();
	}else
	{
		unlink(UPDATE_PATH);
	}
	return ret;
}



int updateYC600ByID(void)	//��ȡYC600������
{
	int ret = 0;
	char domain[100]={'\0'};		//����������
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port = 0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	
	
	getFTPConf(domain,IPFTPadd,&port,user,password);
	print2msg(ECU_DBG_UPDATE,"Domain",domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IPFTPadd);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);
	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/%s/%s",ecu.id,UPDATE_PATH_YC600_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"ID Path",remote_path);
	ret=ftpgetfile(domain,IPFTPadd, port, user, password,remote_path,UPDATE_PATH_YC600_TEMP);
	if(!ret)
	{
		unlink(UPDATE_PATH_YC600);
		rename(UPDATE_PATH_YC600_TEMP,UPDATE_PATH_YC600);
		deletefile(remote_path);
	}else
	{

	}
	return ret;
}

int updateYC1000ByID(void)	//��ȡYC1000������
{
	int ret = 0;
	char domain[100]={'\0'};		//����������
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port = 0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	
	
	getFTPConf(domain,IPFTPadd,&port,user,password);
	print2msg(ECU_DBG_UPDATE,"Domain",domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IPFTPadd);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);
	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/%s/%s",ecu.id,UPDATE_PATH_YC1000_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"ID Path",remote_path);
	ret=ftpgetfile(domain,IPFTPadd, port, user, password,remote_path,UPDATE_PATH_YC1000_TEMP);
	if(!ret)
	{
		unlink(UPDATE_PATH_YC1000);
		rename(UPDATE_PATH_YC1000_TEMP,UPDATE_PATH_YC1000);
		deletefile(remote_path);
	}else
	{

	}
	return ret;
}

int updateQS1200ByID(void)	//��ȡQS1200������
{
	int ret = 0;
	char domain[100]={'\0'};		//����������
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port = 0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	
	
	getFTPConf(domain,IPFTPadd,&port,user,password);
	print2msg(ECU_DBG_UPDATE,"Domain",domain);
	print2msg(ECU_DBG_UPDATE,"FTPIP",IPFTPadd);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);
	
	//��ȡ������IP��ַ
	sprintf(remote_path,"/ECU_R_M3/%s/%s",ecu.id,UPDATE_PATH_QS1200_SUFFIX);
	print2msg(ECU_DBG_UPDATE,"ID Path",remote_path);
	ret=ftpgetfile(domain,IPFTPadd, port, user, password,remote_path,UPDATE_PATH_QS1200_TEMP);
	if(!ret)
	{
		unlink(UPDATE_PATH_QS1200);
		rename(UPDATE_PATH_QS1200_TEMP,UPDATE_PATH_QS1200);
		deletefile(remote_path);
	}else
	{

	}
	return ret;
}
void remote_update_thread_entry(void* parameter)
{
	int i = 0;
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_UPDATE);
	
	while(1)
	{
		for(i = 0;i<2;i++)
		{
			if(-1 != updateECUByVersion())
				break;
		}
		for(i = 0;i<2;i++)
		{
			if(-1 != updateECUByID())
				break;
		}
				for(i = 0;i<2;i++)
		{
			if(-1 != updateYC600ByID())
				break;
		}
						for(i = 0;i<2;i++)
		{
			if(-1 != updateYC1000ByID())
				break;
		}
								for(i = 0;i<2;i++)
		{
			if(-1 != updateQS1200ByID())
				break;
		}
		
		//rt_thread_delay(RT_TICK_PER_SECOND*10);		
		rt_thread_delay(RT_TICK_PER_SECOND*86400);		
	}	

}
