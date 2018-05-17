#include <stdio.h>
#include <string.h>
#include <remote_control_protocol.h>
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"
#include "dfs_posix.h"
/*********************************************************************
clrgfdi����ֶΣ�
id, set_flag
**********************************************************************/
extern rt_mutex_t record_data_lock;


/* ��������������GFDI */
int clear_all()
{
    char inverter_ids[MAXINVERTERCOUNT][13] = {"\0"};
    int i, err_count = 0,num = 0;
    char str[50];
    int fd = 0;
    fd = open("/home/data/clrgfdi", O_WRONLY | O_TRUNC | O_CREAT,0);
    if(fd >= 0)
    {
        //��ѯ���������ID��
        num = get_num_from_id(inverter_ids);

        for(i=1; i<=num; i++)
        {
            sprintf(str,"%s,1\n",inverter_ids[i-1]);

            //��������
            if(write(fd,str,strlen(str)) <= 0)
            {
                err_count++;
            }
        }
        close(fd);
        printdecmsg(ECU_DBG_CONTROL_CLIENT,"clear_all",err_count);
    }


    return err_count;
}

/* ���ָ��̨���������GFDI */
int clear_num(const char *msg, int num)
{
    int i, err_count = 0;
    char inverter_id[13] = {'\0'};
    char str[50];
    int fd = 0;
    fd = open("/home/data/clrgfdi", O_WRONLY | O_TRUNC | O_CREAT,0);
    if(fd >= 0)
    {
        for(i=0; i<num; i++)
        {
            //��ȡһ̨�������ID��
            strncpy(inverter_id, &msg[i*16], 12);
            //��ȡ���GFDI��־(ע��:Э����0��ά��ԭ��,1�������־;����ECU���ݿ���1�������־����û��0)
            if(msg_get_int(&msg[i*16 + 12], 1)){
                //����һ����������ػ�ָ��
                sprintf(str,"%s,1\n",inverter_id);
                //��������
                if(write(fd,str,strlen(str)) <= 0)
                {
                    err_count++;
                }
            }
        }
        close(fd);
    }

    return err_count;
}

/* ��A112��EMA���������GFDI */
int clear_inverter_gfdi(const char *recvbuffer, char *sendbuffer)
{

    int ack_flag = SUCCESS;
    int type, num;
    char timestamp[15] = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    //��ȡ�������ͱ�־λ: 0ȫ�����, 1ָ����������
    type = msg_get_int(&recvbuffer[30], 1);
    //��ȡ���������
    num = msg_get_int(&recvbuffer[31], 4);
    //��ȡʱ���
    strncpy(timestamp, &recvbuffer[35], 14);

    switch(type)
    {
    case 0:
        if(clear_all() > 0)
            ack_flag = DB_ERROR;
        break;
    case 1:
        //����ʽ
        if(!msg_num_check(&recvbuffer[52], num, 13, 1)){
            ack_flag = FORMAT_ERROR;
        }
        else{
            printdecmsg(ECU_DBG_CONTROL_CLIENT,"num",num);
            if(clear_num(&recvbuffer[52], num) > 0)
                ack_flag = DB_ERROR;
        }
        break;
    default:
        ack_flag = FORMAT_ERROR;
        break;
    }

    //ƴ��Ӧ����Ϣ
    msg_ACK(sendbuffer, "A112", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 0;
}
