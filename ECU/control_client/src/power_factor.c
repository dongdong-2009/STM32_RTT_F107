#include <stdio.h>
#include <string.h>
#include <remote_control_protocol.h>
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"
#include "dfs_posix.h"

extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];
extern ecu_info ecu;

/*��A145���ϱ�������Ĺ�������*/ //��ͬʱ����A131��������A118��δ����A145����ΪA131Ҳ�ᴥ��A145
int response_inverter_power_factor(const char *recvbuffer, char *sendbuffer)
{
    int ack_flag = SUCCESS;
    char timestamp[15] = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
    //��ȡ�����������������+��������
    if(file_set_one("2", "/tmp/presdata.con")){
        ack_flag = FILE_ERROR;
    }

    //��ȡʱ���
    strncpy(timestamp, &recvbuffer[34], 14);

    //ƴ��Ӧ����Ϣ
    msg_ACK(sendbuffer, "A145", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 0;

}

int set_all_inverter_power_factor(char *recvbuffer, char *sendbuffer)
{
    int ack_flag = SUCCESS;
    char timestamp[15] = {'\0'};
    char str[100];
    int temp = 0;
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    if(recvbuffer[30]=='A')
    {
        temp = msg_get_int(&recvbuffer[31],3);
        if(temp == 100)
            temp = 31;
        else
            temp = temp/10+11;
    }
    else if(recvbuffer[30]=='B')
    {
        temp = temp/10+1;
    }
    else
    {
        ack_flag = FORMAT_ERROR;
    }
    strncpy(timestamp, &recvbuffer[34], 14);
    if((recvbuffer[30]=='A')||(recvbuffer[30]=='B'))
    {

        delete_line("/home/data/setpropa","/home/data/setpropa.t","power_factor",12);
        sprintf(str,"power_factor,%d,1",temp);
        //插入数据
        if(-1 == insert_line("/home/data/setpropa",str))
        {
            ack_flag=DB_ERROR;
        }

    }
    msg_ACK(sendbuffer, "A146", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 0;


}

