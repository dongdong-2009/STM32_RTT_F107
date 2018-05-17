/*****************************************************************************/
/* File      : comm_config.c                                                 */
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
#include <string.h>
#include <stdlib.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "threadlist.h"
#include "rtthread.h"
#include "mycommand.h"
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define NUM 6

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];
extern ecu_info ecu;

typedef struct comm_config
{
    int port1;
    int port2;
    int timeout;
    int report_interval;
    int socket_type;
    char socket_addr[32];
}Comm_Cfg;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/* �����ļ��ж�ȡ�ļ�ֵ�Ա��浽ͨ�����ò����ṹ���� */
int get_cfg(Comm_Cfg *cfg, MyArray *array)
{
    int i;

    for(i=0; i<NUM; i++){
        if(!strlen(array[i].name))break;
        //��ʱʱ��
        if(!strcmp(array[i].name, "Timeout")){
            cfg->timeout = atoi(array[i].value);
        }
        //��ѯʱ��
        else if(!strcmp(array[i].name, "Report_Interval")){
            cfg->report_interval = atoi(array[i].value);
        }
        //����
        else if(!strcmp(array[i].name, "Domain")){
            if(strlen(array[i].value)){
                cfg->socket_type = 1;
                strncpy(cfg->socket_addr, array[i].value, 32);
            }
            else{
                cfg->socket_type = 0;
            }
        }
        //IP��ַ
        else if(!strcmp(array[i].name, "IP")){
            if(cfg->socket_type != 1){
                strncpy(cfg->socket_addr, array[i].value, 32);
            }
        }
        //�˿�1
        else if(!strcmp(array[i].name, "Port1")){
            cfg->port1 = atoi(array[i].value);
        }
        //�˿�2
        else if(!strcmp(array[i].name, "Port2")){
            cfg->port2 = atoi(array[i].value);
        }
    }
    return 0;
}

/* ��ͨ�����ò����ṹ���еĲ������浽����д���ļ��ļ�ֵ���� */
int save_cfg(Comm_Cfg *cfg, MyArray *array, const char *buffer)
{
    int i;

    cfg->port1 = msg_get_int(&buffer[1], 5);
    cfg->port2 = msg_get_int(&buffer[6], 5);
    cfg->timeout = msg_get_int(&buffer[11], 3);
    cfg->report_interval = msg_get_int(&buffer[14], 2);
    cfg->socket_type = msg_get_int(&buffer[16], 1);
    strncpy(cfg->socket_addr, &buffer[17], 32);

    for(i=0; i<NUM; i++){
        if(!strcmp(array[i].name, "Timeout")){
            snprintf(array[i].value, sizeof(array[i].value), "%d", cfg->timeout);
        }
        else if(!strcmp(array[i].name, "Report_Interval")){
            snprintf(array[i].value, sizeof(array[i].value), "%d", cfg->report_interval);
        }
        else if(!strcmp(array[i].name, "Domain")){
            if(1 == cfg->socket_type){
                snprintf(array[i].value, sizeof(array[i].value), "%s", cfg->socket_addr);
            }
        }
        else if(!strcmp(array[i].name, "IP")){
            if(0 == cfg->socket_type){
                snprintf(array[i].value, sizeof(array[i].value), "%s", cfg->socket_addr);
            }
        }
        else if(!strcmp(array[i].name, "Port1")){
            snprintf(array[i].value, sizeof(array[i].value), "%d", cfg->port1);
        }
        else if(!strcmp(array[i].name, "Port2")){
            snprintf(array[i].value, sizeof(array[i].value), "%d", cfg->port2);
        }
    }
    return 0;
}

/* ��A106��ECU�ϱ�ͨ�����ò��� */
int response_comm_config(const char *recvbuffer, char *sendbuffer)
{
    int comm_cfg_num = 0;
    int comm_cfg_type[3] = {0};
    char timestamp[15] = {'\0'}; //ʱ���
    MyArray array[NUM] = {'\0'}; //ͨ�����ò����ṹ������
    Comm_Cfg cfg1 = {'\0'};
    Comm_Cfg cfg2 = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    //ʱ���
    strncpy(timestamp, &recvbuffer[34], 14);

    //[1]�������������ͨ������
    if(file_get_array(array, NUM, "/yuneng/client.con") == 0){
        get_cfg(&cfg1, array);
    }
    else{
        cfg1.timeout = 10;
        cfg1.report_interval = 5;
    }
    if(file_get_array(array, NUM, "/yuneng/datacent.con") == 0){
        comm_cfg_num++;
        comm_cfg_type[1] = 1;
        get_cfg(&cfg1, array);
    }

    //[2]Զ�̿���ͨ������
    if(file_get_array(array, NUM, "/yuneng/control.con") == 0){
        comm_cfg_num++;
        comm_cfg_type[2] = 1;
        get_cfg(&cfg2, array);
    }

    /* ƴ��Э�� */
    msg_Header(sendbuffer, "A106");
    msgcat_s(sendbuffer, 12, ecu.id);
    msgcat_d(sendbuffer, 1, comm_cfg_num);
    msgcat_s(sendbuffer, 14, timestamp);
    strcat(sendbuffer, "END");
    if(comm_cfg_type[1]){
        strcat(sendbuffer, "1");
        msgcat_d(sendbuffer, 5, cfg1.port1);
        msgcat_d(sendbuffer, 5, cfg1.port2);
        msgcat_d(sendbuffer, 3, cfg1.timeout);
        msgcat_d(sendbuffer, 2, cfg1.report_interval);
        msgcat_d(sendbuffer, 1, cfg1.socket_type);
        strcat(sendbuffer, cfg1.socket_addr);
        strcat(sendbuffer, "END");
    }
    if(comm_cfg_type[2]){
        strcat(sendbuffer, "2");
        msgcat_d(sendbuffer, 5, cfg2.port1);
        msgcat_d(sendbuffer, 5, cfg2.port2);
        msgcat_d(sendbuffer, 3, cfg2.timeout);
        msgcat_d(sendbuffer, 2, cfg2.report_interval);
        msgcat_d(sendbuffer, 1, cfg2.socket_type);
        strcat(sendbuffer, cfg2.socket_addr);
        strcat(sendbuffer, "END");
    }
    rt_mutex_release(record_data_lock);
    return 0;
}

/* ��A107��EMA����ͨ�����ò��� */
int set_comm_config(const char *recvbuffer, char *sendbuffer)
{
    int ack_flag = SUCCESS;
    int comm_cfg_num = 0;
    int comm_cfg_type = 0;
    int cfg_begin = 48;
    char timestamp[15] = {'\0'};
    char buffer[256] = {'\0'};
    Comm_Cfg cfg1 = {'\0'};
    Comm_Cfg cfg2 = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

    MyArray array[NUM] = {
        {"Timeout", ""},
        {"Report_Interval", ""},
        {"Domain", ""},
        {"IP", ""},
        {"Port1", ""},
        {"Port2", ""},
    };

    //��ȡͨ����������
    comm_cfg_num = msg_get_int(&recvbuffer[30], 1);
    //��ȡʱ���
    strncpy(timestamp, &recvbuffer[31], 14);
    while(comm_cfg_num--){
        //���Ƴ���"END"Ϊֹ���ַ���
        memset(buffer, 0, sizeof(buffer));
        cfg_begin += msg_get_one_section(buffer, &recvbuffer[cfg_begin]) + 3;

        //ͨ��Э������
        comm_cfg_type = msg_get_int(buffer, 1);
        printdecmsg(ECU_DBG_CONTROL_CLIENT,"comm_cfg_type",comm_cfg_type);
        //[1]�������������ͨ������
        if(comm_cfg_type == 1){
            file_get_array(&array[2], 4, "/yuneng/datacent.con");
            save_cfg(&cfg1, array, buffer);
            file_set_array(array, 2, "/yuneng/client.con");
            file_set_array(&array[2], 4, "/yuneng/datacent.con");
            threadRestartTimer(10,TYPE_CLIENT);
            reboot_timer(10);
        }
        //[2]Զ�̿���ͨ������
        else if(comm_cfg_type == 2){
            file_get_array(array, 6, "/yuneng/control.con");
            save_cfg(&cfg2, array, buffer);
            file_set_array(array, 6, "/yuneng/control.con");
            reboot_timer(10);
        }
        else{
            ack_flag = FORMAT_ERROR;
        }

    }
    //ƴ��Ӧ����Ϣ
    msg_ACK(sendbuffer, "A107", timestamp, ack_flag);
    rt_mutex_release(record_data_lock);
    return 106;

}
