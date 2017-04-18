#include "remoteUpdate.h"
#include <rtthread.h>
#include "thftpapi.h"
#include "flash_if.h"
#include "dfs_posix.h"
#include "rthw.h"

#define REMOTE_PATH "/Result/ecu1_0.bin"
#define UPDATE_PATH "/FTP/ecu.bin"

void remote_update_thread_entry(void* parameter)
{
	rt_thread_delay(RT_TICK_PER_SECOND * 5);
	while(1)
	{
		if(!ftpgetfile("192.168.1.118", 21, "admin", "admin",REMOTE_PATH,UPDATE_PATH))
		{
			//��ȡ���ļ������и���
			FLASH_Unlock();
			FLASH_If_Erase_APP2();
			FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
			unlink(UPDATE_PATH);
			reboot();
		}else
		{
			unlink(UPDATE_PATH);
		}

		rt_thread_delay(RT_TICK_PER_SECOND*86400);		
	}	

}
