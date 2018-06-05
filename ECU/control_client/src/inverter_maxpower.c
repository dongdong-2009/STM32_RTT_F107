/*****************************************************************************/
/* File      : inverter_maxpower.c                                           */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-03 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"
#include "dfs_posix.h"

/*********************************************************************
power����ֶΣ�
id,limitedpower,limitedresult,stationarypower,stationaryresult,flag
**********************************************************************/
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define MAXPOWER_RANGE "020300"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock ;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/* ����ָ��̨������������ */
int set_maxpower_num(const char *msg, int num)
{
    int i, maxpower, err_count = 0;
    char inverter_id[13] = {'\0'};
    char str[100];
    int fd = 0;
    fd = open("/home/data/power", O_WRONLY | O_TRUNC | O_CREAT,0);
    if(fd >= 0)
    {
        for(i=0; i<num; i++)
        {
            //��ȡһ̨�������ID��
            strncpy(inverter_id, &msg[i*18], 12);
            //��ȡ�����
            maxpower = msg_get_int(&msg[i*18 + 12], 3);
            if(maxpower < 0)
                continue;

            sprintf(str,"%s,%3d,,,,1\n",inverter_id,maxpower);
            //��������
            if(write(fd,str,strlen(str)) <= 0)
            {
                err_count++;
            }
        }
        close(fd);
    }

    return err_count;
}

/* ������������������ */
int set_maxpower_all(int maxpower)
{
    char inverter_ids[MAXINVERTERCOUNT][13] = {"\0"};
    int i,num,ret;
    char *msg = NULL;
    msg = malloc(2048);
    memset(msg,0x00,2048);

    //��ѯ���������ID��
    num = get_num_from_id(inverter_ids);


    //�����������ƴ�ӳ����õ�̨����ʽ
    for(i=0;i<num;i++){
        msgcat_s(msg, 12, inverter_ids[i]);
        msgcat_d(msg, 3, maxpower);
        msgcat_s(msg, 3, "END");
    }
    ret = set_maxpower_num( msg, strlen(msg)/18);
    free(msg);
    msg = NULL;
    return ret ;
}

/* ��A110��EMA�������������� */
int set_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{
    int ack_flag = SUCCESS;
    int type, maxpower, num;
    char timestamp[15] = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    //��ȡ�������ͱ�־λ: 0����ȫ��, 1����ָ�������
    type = msg_get_int(&recvbuffer[30], 1);
    //��ȡ���������
    num = msg_get_int(&recvbuffer[31], 4);
    //��ȡʱ���
    strncpy(timestamp, &recvbuffer[35], 14);

    {
        switch(type)
        {
        case 0:
            maxpower = msg_get_int(&recvbuffer[52], 3);
            if(maxpower >= 0){
                if(set_maxpower_all(maxpower) > 0)
                    ack_flag = DB_ERROR;
            }
            break;
        case 1:
            //����ʽ
            if(!msg_num_check(&recvbuffer[52], num, 15, 1)){
                ack_flag = FORMAT_ERROR;
            }
            else{
                if(set_maxpower_num(&recvbuffer[52], num) > 0)
                    ack_flag = DB_ERROR;
            }
            break;
        default:
            ack_flag = FORMAT_ERROR;
            break;
        }

    }
    //ƴ��Ӧ����Ϣ
    msg_ACK(sendbuffer, "A110", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 0;
}

/* ��A117����ȡ���������ʼ���Χ */
int response_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{

    int ack_flag = SUCCESS;

    char timestamp[15] = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    //���ö�ȡ�������������ʵ�ָ��
    if(file_set_one("ALL", "/tmp/maxpower.con") < 0){
        ack_flag = FILE_ERROR;
    }

    strncpy(timestamp, &recvbuffer[34], 14);

    //ƴ��Ӧ����Ϣ
    msg_ACK(sendbuffer, "A117", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 0;
}
