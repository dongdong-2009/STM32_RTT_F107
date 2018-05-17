/*
 * ���ļ�����Ҫ����������������ı���������ͬʱ�����µ�13����������ϵ�5�������
 * Created by Zhyf
 * Created on 2014/04/24
 */

/*********************************************************************
setpropa����ֶ�:
parameter_name,parameter_value,set_flag             primary key(parameter_name)

proparas����ֶΣ�
1 id, type,under_voltage_fast,over_voltage_fast,
2 under_voltage_slow,over_voltage_slow,under_frequency_fast,over_frequency_fast,
3 under_frequency_slow,over_frequency_slow, voltage_triptime_fast,voltage_triptime_slow, 
4 frequency_triptime_fast,frequency_triptime_slow, grid_recovery_time,regulated_dc_working_point, 
5 under_voltage_stage_2,voltage_3_clearance_time, power_factor,relay_protect,
6 start_time,set_flag               primary key(id))");
*********************************************************************/


/*****************************************************************************/
/*  File      : set_protection_parameters.c                                  */
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
#include "variation.h"
#include <string.h>
#include "debug.h"
#include "zigbee.h"
#include "myfile.h"
#include "file.h"
#include "set_protection_parameters.h"
#include "rtthread.h"
#include "rthw.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/*yc600*/
int set_protection_yc600(int order,int data,int num)
{	
    int i;
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = order;			//CMD
    sendbuff[4] = 0x00;
    sendbuff[5] = 0x00;
    sendbuff[6] = 0x00;

    if(num==3)
    {
        sendbuff[4]=data/65536;
        sendbuff[5] = (data%65536/256);
        sendbuff[6] = (data%256);
    }
    else if(num==2)
    {
        sendbuff[4] = (data/256);
        sendbuff[5] = (data%256);
    }
    else if(num==1)
        sendbuff[4] = data;
    else ;
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set yc600 all", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


/*��ȡ���õĲ���ֵ�ͱ�־λ*/
int get_value_flag(char *para_name, char *value)
{
    FILE *fp;
    char data[200] = {'\0'};
    char splitdata[3][32] = {'\0'};
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(data,0x00,200);

        while(NULL != fgets(data,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            if(1 == atoi(splitdata[2]))
            {
                strcpy(para_name, splitdata[0]);
                strcpy(value, splitdata[1]);
                fclose(fp);
                return 1;
            }
            memset(data,0x00,200);
        }
        fclose(fp);
    }else
    {
        return -1;
    }
    return 0;

}

int get_under_voltage_slow_yc1000(inverter_info *firstinverter)
{
    int under_voltage_slow=0xFFFF;
    float temp;
    FILE *fp;
    char data[200];
    char splitdata[3][32];
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(data,0x00,200);

        while(NULL != fgets(data,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            if(!strcpy(splitdata[0],"under_voltage_slow"))
            {
                temp = atof(splitdata[1]) * 11614.45;
                if((temp-(int)temp)>=0.5)
                    under_voltage_slow = temp + 1;
                else
                    under_voltage_slow = temp;

                break;
            }
            memset(data,0x00,200);
        }
        fclose(fp);
    }

    return under_voltage_slow;
}

int get_over_voltage_slow_yc1000(inverter_info *firstinverter)
{
    int over_voltage_slow=0xFFFF;

    float temp;
    FILE *fp;
    char data[200];
    char splitdata[3][32];
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(data,0x00,200);

        while(NULL != fgets(data,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            if(!strcpy(splitdata[0],"over_voltage_slow"))
            {
                temp = atof(splitdata[1]) * 11614.45;
                if((temp-(int)temp)>=0.5)
                    over_voltage_slow = temp + 1;
                else
                    over_voltage_slow = temp;

                break;
            }
            memset(data,0x00,200);
        }
        fclose(fp);
    }

    return over_voltage_slow;
}

char get_under_frequency_slow_yc1000(inverter_info *firstinverter)
{
    char under_frequency_slow=0xFF;

    FILE *fp;
    char data[200];
    char splitdata[3][32];
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(data,0x00,200);

        while(NULL != fgets(data,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            if(!strcpy(splitdata[0],"under_frequency_slow"))
            {
                under_frequency_slow = (256000/atoi(splitdata[1]));
                break;
            }
            memset(data,0x00,200);
        }
        fclose(fp);
    }

    return under_frequency_slow;
}

char get_over_frequency_slow_yc1000(inverter_info *firstinverter)
{

    char over_frequency_slow=0xFF;

    FILE *fp;
    char data[200];
    char splitdata[3][32];
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(data,0x00,200);

        while(NULL != fgets(data,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            if(!strcpy(splitdata[0],"over_frequency_slow"))
            {
                over_frequency_slow = (256000/atoi(splitdata[1]));
                break;
            }
            memset(data,0x00,200);
        }
        fclose(fp);
    }

    return over_frequency_slow;
}

int get_recovery_time_from_inverter(inverter_info *inverter)
{
    int i=0, ret;
    unsigned char sendbuff[256]={'\0'};
    unsigned char readbuff[256]={'\0'};
    int grid_recovery_time=-1;

    sendbuff[i++] = 0xFB;
    sendbuff[i++] = 0xFB;
    sendbuff[i++] = 0x06;
    sendbuff[i++] = 0xDD;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0x00;
    sendbuff[i++] = 0xE3;
    sendbuff[i++] = 0xFE;
    sendbuff[i++] = 0xFE;

    print2msg(ECU_DBG_MAIN,inverter->id, "get_recovery_time_from_inverter");
    zb_send_cmd(inverter, (char *)sendbuff, i);
    ret = zb_get_reply((char *)readbuff,inverter);
    if((33 == ret) && (0xDD == readbuff[3]) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0xFE == readbuff[31]) && (0xFE == readbuff[32]))
    {
        grid_recovery_time = readbuff[17]*256 + readbuff[18];
    }

    return grid_recovery_time;

}

int set_regulated_dc_working_point_yc1000(char *value)  			//ֱ����ѹ����
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;


    data = (int)(atof(value) * 4096/82.5);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x50;			//CMD
    sendbuff[4] = data%65536/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set dc voltage stabilization", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}




int set_undervoltage_slow_yc1000(char *value)  //��Χ��ѹ����ֵ
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;


    data = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x53;			//LENGTH
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set undervoltage slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_overvoltage_slow_yc1000(char *value)  //��Χ��ѹ����ֵ
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;


    data = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x54;			//LENGTH
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set overvoltage slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_overvoltage_fast_yc1000(char *value)		//��Χ��ѹ����ֵ
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (int)(atof(value) * 16500);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x52;			//LENGTH
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set overvoltage fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_undervoltage_fast_yc1000(char *value)		//��Χ��ѹ����ֵ
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (int)(atof(value) * 16500);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x51;			//LENGTH
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set undervoltage fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);
    return 0;
}




int set_underfrequency_fast_yc1000(char *value)				//��ΧƵ������
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (256000/atof(value));


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x57;			//LENGTH
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set underfrequency fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_overfrequency_fast_yc1000(char *value)				//��ΧƵ������
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (256000/atof(value));


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x58;			//LENGTH
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set overfrequency fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_underfrequency_slow_yc1000(char *value)				//��ΧƵ������
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (256000/atof(value));


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x59;			//LENGTH
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set underfrequency slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_overfrequency_slow_yc1000(char *value)				//��ΧƵ������
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (256000/atof(value));


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//CMD
    sendbuff[3] = 0x5A;			//LENGTH
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set overfrequency slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_voltage_triptime_fast_yc1000(char *value)				//��Χ��ѹ�ӳٱ���ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value)*50;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x55;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set voltage triptime fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}


int set_voltage_triptime_slow_yc1000(char *value)				//��Χ��ѹ�ӳٱ���ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value)*50;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x56;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set voltage triptime slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_frequency_triptime_fast_yc1000(char *value)				//��ΧƵ���ӳٱ���ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value)*50;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x5B;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set frequency triptime fast", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_frequency_triptime_slow_yc1000(char *value)				//��ΧƵ���ӳٱ���ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value)*50;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x5C;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set frequency triptime slow", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_grid_recovery_time_yc1000(char *value)						//�����ָ�ʱ������
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x5D;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid


    printhexmsg(ECU_DBG_MAIN,"Set grid recovery time", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_under_voltage_stage_2_yc1000(char *value)  			//����Χ��ѹ����
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;


    data = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x5E;			//CMD
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set voltage slow2", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}



int set_voltage_3_clearance_time_yc1000(char *value)				//����Χ��ѹ�ӳٱ���ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value)*50;


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x5F;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set voltage triptime slow2", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}

int set_start_time_yc1000(char *value)				//ֱ������ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = atof(value);


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x3D;			//CMD
    sendbuff[4] = data/256;		//DATA
    sendbuff[5] = data%256;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set DC startime ", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}

int set_active_antiisland_time_yc1000(char *value)				//ֱ������ʱ��
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;

    data = (int)(atof(value)*2.5);


    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x4E;			//CMD
    sendbuff[4] = data;		//DATA
    sendbuff[5] = 0x00;		//DATA
    sendbuff[6] = 0x00;			//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set active antiisland time ", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}

/*�������������ñ�־*/
int clear_flag(char *para_name)					//���ú�������ݿ��в��������ñ�־
{
    char data[200] = {'\0'};
    char splitdata[3][32];
    int i;

    //��ȡ��
    if(1 == read_line("/home/data/setpropa",data,para_name,strlen(para_name)))
    {
        //�������з���
        splitString(data,splitdata);
        memset(data,0x00,200);
        sprintf(data,"%s,%.2f,,0\n",para_name,atof(splitdata[1]));

        //ɾ��para_name������
        delete_line("/home/data/setpropa","/home/data/setpropa.t",para_name,strlen(para_name));
        //����������
        for(i=0; i<3; i++)
        {
            if(1 == insert_line("/home/data/setpropa",data))
            {
                break;
            }
            else
                print2msg(ECU_DBG_MAIN,"Failed to clear protection set flag", para_name);
        }
    }
    return 0;
}

int get_system_model(inverter_info *firstinverter)
{
    int i,res;
    int m0,m600,m1000,bb,b1;	//modelΪ��ȷ����YC600��YC1000��BB��600��B1��600����ͳ��
    inverter_info *curinverter = firstinverter;
    m0=m600=m1000=bb=b1=res=0;

    for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
    {
        if(curinverter->model==0)
            m0++;
        else if((curinverter->model==7)||(curinverter->model==0x17))
        {
            m600++;
            if(curinverter->inverterstatus.deputy_model==1)
                bb++;
            else if((curinverter->inverterstatus.deputy_model==2)||(curinverter->model==0x17))
                b1++;
        }
        else if((curinverter->model==5)||(curinverter->model==6))
            m1000++;
    }
    if(m0>0)
        res=res+100;
    if((m600>0)&&(m1000==0))
    {
        res=res+10;
        if((bb>0)&&(b1==0))
            res=res+1;
        else if((bb==0)&&(b1>0))
            res=res+2;
        else if((bb>0)&&(b1>0))
            res=res+9;
    }
    else if((m1000>0)&&(m600==0))
        res=res+20;
    else if((m1000>0)&&(m600>0))
        res=res+90;
    //printdecmsg("System models",res);
    //��λ��0��ȡmodel����ȫ��1��ȫ��ʮλ��1��ȡ��model�Ķ���600��2����1000��9���ֶ��У�
    //��λ��ʮλΪ1���������Ч��1 bb��600��2 b1��600��9���ֶ��У�
    return res;
}


/*����������ñ������������Ҷ�ȡ��������ú�ı�������*/
int set_protection_paras(inverter_info *firstinverter)
{
    char para_name[64] = {'\0'};
    char value[16] = {'\0'};
    int count_sum = 0;
    int data = 0;
    int res;
    int bb_or_b1=0;		//0,9,����yc600�㲥;1������bb�㲥��2������b1�㲥
    int yc600=0;
    int yc1000=0;

    res=get_system_model(firstinverter);
    if((1==res/100)||(0==res/100))		//model��ȫ��������ɵ�һϵͳ��Ҫ�����
    {
        res=res%100;
        if(1==res/10)
        {
            yc600=1;
            bb_or_b1=res%10;
        }
        else if(2==res/10)
            yc1000=1;
    }


    while(1 == get_value_flag(para_name, value))
    {
        clear_flag(para_name);					//���ú�������ݿ��в��������ñ�־
        data=0;
        count_sum++;
        if((yc600==1)&&(yc1000==1))
        {
            count_sum = 0;
            continue;
        }

        if(!strcmp("under_voltage_fast", para_name))
        {
            data = (int)(atof(value) * 1.3277);
            if(yc1000==1)
                set_undervoltage_fast_yc1000(value);
            else if(yc600==1)
            {
                if(bb_or_b1==1)
                {
                    data = (int)(atof(value) * 1.3277);
                    set_protection_yc600(0x51,data,2);
                }
                if(bb_or_b1==2)
                {
                    data = (int)(atof(value) * 1.33);
                    set_protection_yc600(0x51,data,2);
                }
            }
        }
        else if(!strcmp("over_voltage_fast", para_name))
        {
            data = (int)(atof(value) * 1.3277);
            if(yc1000==1)
                set_overvoltage_fast_yc1000(value);
            else if(yc600==1)
            {
                if(bb_or_b1==1)
                {
                    data = (int)(atof(value) * 1.3277);
                    set_protection_yc600(0x52,data,2);
                }
                if(bb_or_b1==2)
                {
                    data = (int)(atof(value) * 1.33);
                    set_protection_yc600(0x52,data,2);
                }
            }
        }
        else if(!strcmp("under_voltage_stage_2", para_name))
        {
            data = (int)(atof(value) * 1.3277);
            if(yc1000==1)
            {
                set_undervoltage_slow_yc1000(value);
                set_undervoltage_slow_yc1000_5(value);
                set_under_voltage_stage_3_yc1000(value);
            }
            else if(yc600==1)
            {
                if(bb_or_b1==1)
                {
                    data = (int)(atof(value) * 1.3277);
                    set_protection_yc600(0x73,data,2);
                }
                if(bb_or_b1==2)
                {
                    data = (int)(atof(value) * 1.33);
                    set_protection_yc600(0x73,data,2);
                }
            }
        }
        else if(!strcmp("over_voltage_slow", para_name))
        {
            data = (int)(atof(value) * 1.3277);
            if(yc1000==1)
            {
                set_overvoltage_slow_yc1000(value);
                set_overvoltage_slow_yc1000_5(value);
            }
            else if(yc600==1)
            {
                if(bb_or_b1==1)
                {
                    data = (int)(atof(value) * 1.3277);
                    set_protection_yc600(0x54,data,2);
                }
                if(bb_or_b1==2)
                {
                    data = (int)(atof(value) * 1.33);
                    set_protection_yc600(0x54,data,2);
                }
            }
        }
        else if(!strcmp("under_frequency_fast", para_name))
        {
            data = (int)(50000000/atof(value));
            if(yc600==1)
                set_protection_yc600(0x58,data,3);
            else if(yc1000==1)
                set_underfrequency_fast_yc1000(value);
        }
        else if(!strcmp("over_frequency_fast", para_name))
        {
            data = (int)(50000000/atof(value));
            if(yc600==1)
                set_protection_yc600(0x57,data,3);
            else if(yc1000==1)
                set_overfrequency_fast_yc1000(value);
        }
        else if(!strcmp("under_frequency_slow", para_name))
        {
            data = (int)(50000000/atof(value));
            if(yc600==1)
                set_protection_yc600(0x5A,data,3);
            else if(yc1000==1)
            {
                set_underfrequency_slow_yc1000(value);
                set_underfrequency_slow_yc1000_5(value);
            }
        }
        else if(!strcmp("over_frequency_slow", para_name))
        {
            data = (int)(50000000/atof(value));
            if(yc600==1)
                set_protection_yc600(0x59,data,3);
            else if(yc1000==1)
            {
                set_overfrequency_slow_yc1000(value);
                set_overfrequency_slow_yc1000_5(value);
            }
        }
        else if(!strcmp("voltage_triptime_fast", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x55,data,2);
            else if(yc1000==1)
                set_voltage_triptime_fast_yc1000(value);
        }
        else if(!strcmp("voltage_triptime_slow", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x56,data,2);
            else if(yc1000==1)
                set_voltage_triptime_slow_yc1000(value);
        }
        else if(!strcmp("frequency_triptime_fast", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x5B,data,2);
            else if(yc1000==1)
                set_frequency_triptime_fast_yc1000(value);
        }
        else if(!strcmp("frequency_triptime_slow", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x5C,data,2);
            else if(yc1000==1)
                set_frequency_triptime_slow_yc1000(value);
        }
        else if(!strcmp("grid_recovery_time", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x5D,data,2);
            else if(yc1000==1)
            {
                set_grid_recovery_time_yc1000(value);
                set_grid_recovery_time_yc1000_5(value);
            }
        }
        else if(!strcmp("regulated_dc_working_point", para_name))
        {
            data = (int)(atof(value) * 4096/82.5);
            if(yc600==1)
                set_protection_yc600(0x50,data,2);
            else if(yc1000==1)
                set_regulated_dc_working_point_yc1000(value);
        }
        else if(!strcmp("under_voltage_slow", para_name))
        {
            data = (int)(atof(value) * 1.3277);
            if(yc1000==1)
                set_under_voltage_stage_2_yc1000(value);
            else if(yc600==1)
            {
                if(bb_or_b1==1)
                {
                    data = (int)(atof(value) * 1.3277);
                    set_protection_yc600(0x5E,data,2);
                }
                if(bb_or_b1==2)
                {
                    data = (int)(atof(value) * 1.33);
                    set_protection_yc600(0x5E,data,2);
                }
            }
        }
        else if(!strcmp("voltage_3_clearance_time", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x5F,data,2);
            else if(yc1000==1)
                set_voltage_3_clearance_time_yc1000(value);
        }
        else if(!strcmp("start_time", para_name))
        {
            data = (int)(atof(value)*100);
            if(yc600==1)
                set_protection_yc600(0x3D,data,2);
            else if(yc1000==1)
                set_start_time_yc1000(value);
        }
        else if(!strcmp("power_factor", para_name))
        {
            data=(int)(atof(value));
            set_protection_yc600(0x3F,data,1);
            //			set_voltage_3_clearance_time_yc1000(value);
        }
        else if((!strcmp("relay_protect", para_name)))
        {
            data=(int)(atof(value));
            if(yc600==1)
            {
                if(data==0)
                    set_protection_yc600(0x3A,0,0);
                else if(data==1)
                    set_protection_yc600(0x3B,0,0);
                else ;
            }
        }
        else if(!strcmp("active_antiisland_time", para_name))
        {
            data = (int)(atof(value) * 1.33);
            if(yc600==1)
                set_protection_yc600(0x4E,data,1);
            else if(yc1000==1)
                set_active_antiisland_time_yc1000(value);
        }
        else
        {
            break;
        }
    }

    return count_sum;
}

/*���������������浽���ݿ���*/
int resolve_protection_paras_YC600(inverter_info *inverter, char *readbuff, int size)
{
    char *inverter_result = NULL;	//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
    int i;
    char data[400];
    int under_voltage_fast;
    int over_voltage_fast;
    int under_voltage_slow;
    int over_voltage_slow;
    int regulated_dc_working_point;
    int under_voltage_stage_2;
    float under_frequency_fast;
    float over_frequency_fast;
    float under_frequency_slow;
    float over_frequency_slow;
    float voltage_triptime_fast;
    float voltage_triptime_slow;
    float frequency_triptime_fast;
    float frequency_triptime_slow;
    float grid_recovery_time;
    float voltage_3_clearance_time;
    float start_time;
    float active_antiisland_time;
    int power_factor;
    float relay_protect;
    float temp;
    inverter_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
    memset(inverter_result,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);


    if(58 == size)								//17�����
    {
        temp = (readbuff[6+3]*256 + readbuff[7+3])/1.3277;
        if((temp-(int)temp)>=0.5)
            under_voltage_fast = (int)temp +1;
        else
            under_voltage_fast = (int)temp;

        temp = (readbuff[9+3]*256 + readbuff[10+3])/1.3277;

        if((temp-(int)temp)>=0.5)
            over_voltage_fast = (int)temp +1;
        else
            over_voltage_fast = (int)temp;

        temp = (readbuff[12+3]*256 + readbuff[13+3])/1.3277;
        if((temp-(int)temp)>0.5)
            under_voltage_stage_2 = (int)temp +1;
        else
            under_voltage_stage_2 = (int)temp;

        temp = (readbuff[15+3]*256 + readbuff[16+3])/1.3277;
        if((temp-(int)temp)>0.5)
            over_voltage_slow = (int)temp +1;
        else
            over_voltage_slow = (int)temp;

        over_frequency_fast = 50000000.0/(readbuff[17+3]*256*256 + readbuff[18+3]*256+readbuff[19+3]);

        under_frequency_fast = 50000000.0/(readbuff[20+3]*256*256 + readbuff[21+3]*256+readbuff[22+3]);

        over_frequency_slow = 50000000.0/(readbuff[23+3]*256*256 + readbuff[24+3]*256+readbuff[25+3]);

        under_frequency_slow = 50000000.0/(readbuff[26+3]*256*256 + readbuff[27+3]*256+readbuff[28+3]);

        voltage_triptime_fast = (readbuff[29+3]*256 + readbuff[30+3])/100.0;

        voltage_triptime_slow = (readbuff[33+3]*256 + readbuff[34+3])/100.0;

        frequency_triptime_fast = (readbuff[31+3]*256 + readbuff[32+3])/100.0;

        frequency_triptime_slow = (readbuff[35+3]*256 + readbuff[36+3])/100.0;

        grid_recovery_time = (readbuff[37+3]*256 + readbuff[38+3])/100.0;  //pages:read 17 protection intval=>number_format

        temp = (readbuff[39+3]*256 + readbuff[40+3])/1.3277;
        if((temp-(int)temp)>0.5)
            under_voltage_slow = (int)temp +1;
        else
            under_voltage_slow = (int)temp;

        voltage_3_clearance_time = (readbuff[41+3]*256 + readbuff[42+3])/100.0;

        temp = (readbuff[43+3]*256 + readbuff[44+3])*82.5/4096;
        if((temp-(int)temp)>0.5)
            regulated_dc_working_point = (int)temp +1;
        else
            regulated_dc_working_point = (int)temp;

        start_time = (readbuff[48+3]*256 + readbuff[49+3])/100.0;

        active_antiisland_time = readbuff[50+3]*2.5;

        relay_protect = readbuff[51+3]*1.0;

        power_factor = (int)(readbuff[52+3]&0x1F);


        delete_line("/home/data/proparas","/home/data/proparas.t",inverter->id,12);
        //id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, power_factor,relay_protect,active_antiisland_time
        sprintf(data, "%s,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%d,%.2f,%.2f\n", inverter->id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, power_factor, relay_protect, active_antiisland_time);
        for(i=0; i<3; i++)
        {
            if(1 == insert_line("/home/data/proparas",data))
                break;
            rt_thread_delay(100);
        }

        sprintf(inverter_result, "%s018AA%06dAC%06dAD%06dAE%06dAF%06dAG%06dAH%06dAI%06dAJ%06dAK%06dAL%06dAM%06dAN%06dAO%06dAP%06dAQ%06dAR%06dAS%06dEND",
                inverter->id,
                (int)(active_antiisland_time*10),
                under_voltage_slow,
                over_voltage_slow,
                (int)(under_frequency_slow*10),
                (int)(over_frequency_slow*10),
                (int)grid_recovery_time,
                under_voltage_fast,
                over_voltage_fast,
                (int)(under_frequency_fast*10),
                (int)(over_frequency_fast*10),
                (int)(voltage_triptime_fast*100),
                (int)(voltage_triptime_slow*100),
                (int)(frequency_triptime_fast*100),
                (int)(frequency_triptime_slow*100),
                regulated_dc_working_point,
                under_voltage_stage_2,
                (int)(voltage_3_clearance_time*100),
                (int)(start_time));
        save_inverter_parameters_result(inverter, 161, inverter_result);

        memset(inverter_result,'\0',sizeof(inverter_result));
        if(power_factor==31)
            sprintf(inverter_result,"%sA100END",inverter->id);
        else if(power_factor>10)
            sprintf(inverter_result,"%sA%03dEND",inverter->id,(power_factor-11)*10);
        else
            sprintf(inverter_result,"%sB%03dEND",inverter->id,(power_factor-1)*10);
        save_inverter_parameters_result(inverter, 145, inverter_result);
        free(inverter_result);
        inverter_result = NULL;
        return 0;
    }
    else{
        free(inverter_result);
        inverter_result = NULL;
        return -1;
    }


}

/*���������������浽���ݿ���*/
int resolve_protection_paras_YC1000(inverter_info *inverter, char *readbuff, int size)
{
    char *inverter_result = NULL;	//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
    int i;
    char data[400];

    int under_voltage_fast;
    int over_voltage_fast;
    int under_voltage_slow;
    int over_voltage_slow;
    int regulated_dc_working_point;
    int under_voltage_stage_2;
    float under_frequency_fast;
    float over_frequency_fast;
    float under_frequency_slow;
    float over_frequency_slow;
    float voltage_triptime_fast;
    float voltage_triptime_slow;
    float frequency_triptime_fast;
    float frequency_triptime_slow;
    int grid_recovery_time;
    float voltage_3_clearance_time;
    int start_time;
    float active_antiisland_time;
    float temp;
    inverter_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
    memset(inverter_result,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);


    if(58 == size)								//17�����
    {
        temp = (readbuff[5+3]*65536 + readbuff[6+3]*256 + readbuff[7+3])/16500;
        if((temp-(int)temp)>=0.5)
            under_voltage_fast = (int)temp +1;
        else
            under_voltage_fast = (int)temp;

        temp = (readbuff[8+3]*65536 + readbuff[9+3]*256 + readbuff[10+3])/16500;

        if((temp-(int)temp)>=0.5)
            over_voltage_fast = (int)temp +1;
        else
            over_voltage_fast = (int)temp;

        temp = (readbuff[11+3]*65536 + readbuff[12+3]*256 + readbuff[13+3])/11614.45;
        if((temp-(int)temp)>0.5)
            under_voltage_stage_2 = (int)temp +1;
        else
            under_voltage_stage_2 = (int)temp;

        temp = (readbuff[14+3]*65536 + readbuff[15+3]*256 + readbuff[16+3])/11614.45;
        if((temp-(int)temp)>0.5)
            over_voltage_slow = (int)temp +1;
        else
            over_voltage_slow = (int)temp;


        under_frequency_fast = 256000.0/(readbuff[17+3]*256 + readbuff[18+3]);

        over_frequency_fast = 256000.0/(readbuff[19+3]*256 + readbuff[20+3]);

        under_frequency_slow = 256000.0/(readbuff[21+3]*256 + readbuff[22+3]);

        over_frequency_slow = 256000.0/(readbuff[23+3]*256 + readbuff[24+3]);

        voltage_triptime_fast = (readbuff[25+3]*256 + readbuff[26+3])/50.0;

        voltage_triptime_slow = (readbuff[27+3]*256 + readbuff[28+3])/50.0;

        frequency_triptime_fast = (readbuff[29+3]*256 + readbuff[30+3])/50.0;

        frequency_triptime_slow = (readbuff[31+3]*256 + readbuff[32+3])/50.0;

        grid_recovery_time = readbuff[33+3]*256 + readbuff[34+3];

        temp = (readbuff[35+3]*65536 + readbuff[36+3]*256 + readbuff[37+3])/11614.45;
        if((temp-(int)temp)>0.5)
            under_voltage_slow = (int)temp +1;
        else
            under_voltage_slow = (int)temp;

        voltage_3_clearance_time = (readbuff[38+3]*256 + readbuff[39+3])/50.0;

        temp = (readbuff[40+3]*256 + readbuff[41+3])*82.5/4096;
        if((temp-(int)temp)>0.5)
            regulated_dc_working_point = (int)temp +1;
        else
            regulated_dc_working_point = (int)temp;

        start_time = readbuff[45+3]*256 + readbuff[46+3];

        active_antiisland_time = readbuff[47+3]*2.5;

        delete_line("/home/data/proparas","/home/data/proparas.t",inverter->id,12);
        //id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, power_factor,relay_protect,active_antiisland_time
        sprintf(data, "%s,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%.2f,%d,,,%.2f\n", inverter->id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, active_antiisland_time);
        for(i=0; i<3; i++)
        {
            if(1 == insert_line("/home/data/proparas",data))
                break;
            rt_thread_delay(100);
        }

        //		//A161����A131
        sprintf(inverter_result, "%s018AA%06dAC%06dAD%06dAE%06dAF%06dAG%06dAH%06dAI%06dAJ%06dAK%06dAL%06dAM%06dAN%06dAO%06dAP%06dAQ%06dAR%06dAS%06dEND",
                inverter->id,
                (int)(active_antiisland_time*10),
                under_voltage_slow,
                over_voltage_slow,
                (int)(under_frequency_slow*10),
                (int)(over_frequency_slow*10),
                (int)grid_recovery_time,
                under_voltage_fast,
                over_voltage_fast,
                (int)(under_frequency_fast*10),
                (int)(over_frequency_fast*10),
                (int)(voltage_triptime_fast*100),
                (int)(voltage_triptime_slow*100),
                (int)(frequency_triptime_fast*100),
                (int)(frequency_triptime_slow*100),
                regulated_dc_working_point,
                under_voltage_stage_2,
                (int)(voltage_3_clearance_time*100),
                (int)(start_time));
        save_inverter_parameters_result(inverter, 161, inverter_result);
        free(inverter_result);
        inverter_result = NULL;
        return 0;
    }
    else{
        free(inverter_result);
        inverter_result = NULL;
        return -1;
    }
}


/*���������������浽���ݿ���*/
int resolve_protection_paras5(inverter_info *inverter, char *readbuff, int size)
{
    char *inverter_result = NULL;	//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
    char data[300];
    int i;

    int under_voltage_slow;
    int over_voltage_slow;
    float under_frequency_slow;
    float over_frequency_slow;
    int grid_recovery_time;
    float temp;
    inverter_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
    memset(inverter_result,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);

    if(33 == size)
    {
        temp = (readbuff[4+3]*65536 + readbuff[5+3]*256 + readbuff[6+3])/11614.45;
        under_voltage_slow = (int)(temp + 0.5);

        temp = (readbuff[7+3]*65536 + readbuff[8+3]*256 + readbuff[9+3])/11614.45;
        over_voltage_slow = (int)(temp + 0.5);

        under_frequency_slow = 256000.0/(readbuff[10+3]*256 + readbuff[11+3]);

        over_frequency_slow = 256000.0/(readbuff[12+3]*256 + readbuff[13+3]);

        grid_recovery_time = readbuff[14+3]*256 + readbuff[15+3];


        sprintf(data, "%s,,,,%d,%d,,,%.2f,%.2f,,,,,%d,,,,,,,\n)",
                inverter->id,
                under_voltage_slow,
                over_voltage_slow,
                under_frequency_slow,
                over_frequency_slow,
                grid_recovery_time);
        for(i=0; i<3; i++)
        {
            if(1 == insert_line("/home/data/proparas",data))
                break;
            rt_thread_delay(100);
        }

        //A161����A131
        sprintf(inverter_result, "%s018AC%06dAD%06dAE%06dAF%06dAG%06dEND",
                inverter->id,
                under_voltage_slow,
                over_voltage_slow,
                (int)(under_frequency_slow*10),
                (int)(over_frequency_slow*10),
                (int)grid_recovery_time);
        save_inverter_parameters_result(inverter, 161, inverter_result);
        free(inverter_result);
        inverter_result = NULL;
        return 0;
    }
    else{
        free(inverter_result);
        inverter_result = NULL;
        return -1;
    }
}

//���������ȡ��������������ֵ
int get_parameters_from_inverter(inverter_info *inverter)
{
    unsigned char sendbuff[256] = {'\0'};
    unsigned char readbuff[256] = {'\0'};
    unsigned short check = 0x00;
    int i, res;

    clear_zbmodem();
    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x06;
    sendbuff[3] = 0xDD;
    sendbuff[4] = 0x00;
    sendbuff[5] = 0x00;
    sendbuff[6] = 0x00;
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;
    for(i=2; i<9; i++)
    {
        check = check + sendbuff[i];
    }
    sendbuff[9] = check >> 8;
    sendbuff[10] = check;
    sendbuff[11] = 0xFE;
    sendbuff[12] = 0xFE;
    print2msg(ECU_DBG_MAIN,"Query Inverter's Protection Parameters", inverter->id);
    zb_send_cmd(inverter, (char *)sendbuff, 13);

    //���շ�����Ϣ
    res = zb_get_reply((char *)readbuff,inverter);
    if((33 == res) &&
            (0xFB == readbuff[0]) &&
            (0xFB == readbuff[1]) &&
            (0xDD == readbuff[3]) &&
            (0xFE == readbuff[31]) &&
            (0xFE == readbuff[32]))
    {
        //5�����
        resolve_protection_paras5(inverter, (char *)readbuff, res);
        return 0;
    }

    if((58 == res) &&
            (0xFB == readbuff[0]) &&
            (0xFB == readbuff[1]) &&
            (0xDA == readbuff[3]) &&
            (0xFE == readbuff[56]) &&
            (0xFE == readbuff[57]))
    {
        //17�����
        if((inverter->model==5)||(inverter->model==6))
            resolve_protection_paras_YC1000(inverter, (char *)readbuff, res);
        else if((inverter->model==7)||(inverter->model==0x17))
            resolve_protection_paras_YC600(inverter, (char *)readbuff, res);
        else ;
        return 0;
    }

    return -1;
}

/*��ȡÿһ̨������ı�������*/
int read_protection_parameters(inverter_info *firstinverter)
{
    int i, j;
    char *set_result = NULL;		//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL];
    inverter_info *inverter = firstinverter;
    set_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
    memset(set_result, '\0', sizeof(set_result));
    for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
    {
        for(j=0; j<3; j++)
        {
            if(!get_parameters_from_inverter(inverter))
                break;
        }
    }
    free(set_result);
    set_result= NULL;
    return 0;
}

/*����������ı�������*/
int set_protection_parameters(inverter_info *firstinverter)
{
    FILE *fp;
    char buff[16];
    int flag;
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
    if(set_protection_paras(firstinverter) > 0)
    {
        read_protection_parameters(firstinverter);
    }

    fp = fopen("/tmp/presdata.con", "r");
    if(fp)
    {
        fgets(buff, 255, fp);
        fclose(fp);

        if(!strlen(buff))
            flag = 0;
        if('0' == buff[0])
            flag = 0;
        if('1' == buff[0])
            flag = 1;
        if('2' == buff[0])
            flag = 2;

        if(2 == flag)
        {
            read_protection_parameters(firstinverter);	//	OK

            fp = fopen("/tmp/presdata.con", "w");
            fprintf(fp, "0");
            fclose(fp);
        }
    }
    rt_mutex_release(record_data_lock);
    return 0;
}

//��ȡ�㲥���õĽ�������������5�
int read_protection_parameters_5(char *data)
{
    int grid_recovery_time = 0;
    int under_voltage_slow = 0;
    int over_voltage_slow = 0;
    int under_frequency_slow = 0;
    int over_frequency_slow = 0;
    float temp;

    //��ѯ����
    FILE *fp;
    char linedata[200];
    char splitdata[3][32];
    fp = fopen("/home/data/setpropa", "r");
    if(fp)
    {
        memset(linedata,0x00,200);

        while(NULL != fgets(linedata,200,fp))
        {
            memset(splitdata,0x00,3*32);
            splitString(data,splitdata);
            temp = atof(splitdata[1]);
            if (!strcmp(splitdata[0], "under_voltage_slow")) {
                under_voltage_slow = (int)(temp * 11614.45);
            }
            else if (!strcmp(splitdata[0], "over_voltage_slow")) {
                over_voltage_slow = (int)(temp * 11614.45);
            }
            else if (!strcmp(splitdata[0], "under_frequency_slow")) {
                under_frequency_slow = (256000/temp);
            }
            else if (!strcmp(splitdata[0], "over_frequency_slow")) {
                over_frequency_slow = (256000/temp);
            }
            else if (!strcmp(splitdata[0], "grid_recovery_time")) {
                grid_recovery_time = (int)temp;
            }

            memset(linedata,0x00,200);
        }
        fclose(fp);
    }

    //ƴ������
    data[0] = under_voltage_slow/65535;
    data[1] = under_voltage_slow%65535/256;
    data[2] = under_voltage_slow%256;

    data[3] = over_voltage_slow/65535;
    data[4] = over_voltage_slow%65535/256;
    data[5] = over_voltage_slow%256;

    data[6] = under_frequency_slow/256;
    data[7] = under_frequency_slow%256;

    data[8] = over_frequency_slow/256;
    data[9] = over_frequency_slow%256;

    data[10] = grid_recovery_time/256;
    data[11] = grid_recovery_time%256;

    data[12] = 0x00;
    data[13] = 0x00;
    data[14] = 0x00;

    return 0;
}

//����Χ��ѹ����ֵ(5��)
int set_undervoltage_slow_yc1000_5(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int under_voltage_slow;
    char protection_data[16] = {'\0'};

    read_protection_parameters_5(protection_data);
    under_voltage_slow = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x10;
    sendbuff[3] = 0xAC;
    sendbuff[4] = under_voltage_slow/65536;
    sendbuff[5] = under_voltage_slow%65536/256;
    sendbuff[6] = under_voltage_slow%256;
    sendbuff[7] = protection_data[3];
    sendbuff[8] = protection_data[4];
    sendbuff[9] = protection_data[5];
    sendbuff[10] = protection_data[6];
    sendbuff[11] = protection_data[7];
    sendbuff[12] = protection_data[8];
    sendbuff[13] = protection_data[9];
    sendbuff[14] = protection_data[10];
    sendbuff[15] = protection_data[11];
    sendbuff[16] = protection_data[12];
    sendbuff[17] = protection_data[13];
    sendbuff[18] = protection_data[14];

    for (i=2; i<19; i++) {
        check = check + sendbuff[i];
    }

    sendbuff[19] = check >> 8;
    sendbuff[20] = check;
    sendbuff[21] = 0xFE;
    sendbuff[22] = 0xFE;

    printhexmsg(ECU_DBG_MAIN,"Set undervoltage slow (5)", (char *)sendbuff, 23);

    zb_broadcast_cmd((char *)sendbuff, 23);
    rt_thread_delay(1000);

    return 0;
}

//��Χ��ѹ����ֵ(5��)
int set_overvoltage_slow_yc1000_5(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int over_voltage_slow;
    char protection_data[16] = {'\0'};

    read_protection_parameters_5(protection_data);
    over_voltage_slow = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x10;
    sendbuff[3] = 0xAC;
    sendbuff[4] = protection_data[0];
    sendbuff[5] = protection_data[1];
    sendbuff[6] = protection_data[2];
    sendbuff[7] = over_voltage_slow/65536;
    sendbuff[8] = over_voltage_slow%65536/256;
    sendbuff[9] = over_voltage_slow%256;
    sendbuff[10] = protection_data[6];
    sendbuff[11] = protection_data[7];
    sendbuff[12] = protection_data[8];
    sendbuff[13] = protection_data[9];
    sendbuff[14] = protection_data[10];
    sendbuff[15] = protection_data[11];
    sendbuff[16] = protection_data[12];
    sendbuff[17] = protection_data[13];
    sendbuff[18] = protection_data[14];

    for(i=2; i<19; i++){
        check = check + sendbuff[i];
    }

    sendbuff[19] = check >> 8;
    sendbuff[20] = check;
    sendbuff[21] = 0xFE;
    sendbuff[22] = 0xFE;

    printhexmsg(ECU_DBG_MAIN,"Set overvoltage slow (5)", (char *)sendbuff, 23);

    zb_broadcast_cmd((char *)sendbuff, 23);
    rt_thread_delay(1000);

    return 0;
}

//��ΧƵ������(5��)
int set_underfrequency_slow_yc1000_5(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int under_frequency_slow;
    char protection_data[16] = {'\0'};

    read_protection_parameters_5(protection_data);
    under_frequency_slow = (256000/atof(value));

    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x10;
    sendbuff[3] = 0xAC;
    sendbuff[4] = protection_data[0];
    sendbuff[5] = protection_data[1];
    sendbuff[6] = protection_data[2];
    sendbuff[7] = protection_data[3];
    sendbuff[8] = protection_data[4];
    sendbuff[9] = protection_data[5];
    sendbuff[10] = under_frequency_slow/256;
    sendbuff[11] = under_frequency_slow%256;
    sendbuff[12] = protection_data[8];
    sendbuff[13] = protection_data[9];
    sendbuff[14] = protection_data[10];
    sendbuff[15] = protection_data[11];
    sendbuff[16] = protection_data[12];
    sendbuff[17] = protection_data[13];
    sendbuff[18] = protection_data[14];

    for (i=2; i<19; i++) {
        check = check + sendbuff[i];
    }

    sendbuff[19] = check >> 8;
    sendbuff[20] = check;
    sendbuff[21] = 0xFE;
    sendbuff[22] = 0xFE;

    printhexmsg(ECU_DBG_MAIN,"Set underfrequency slow (5)", (char *)sendbuff, 23);

    zb_broadcast_cmd((char *)sendbuff, 23);
    rt_thread_delay(1000);

    return 0;
}

//��ΧƵ������(5��)
int set_overfrequency_slow_yc1000_5(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int over_frequency_slow;
    char protection_data[16] = {'\0'};

    read_protection_parameters_5(protection_data);
    over_frequency_slow = (256000/atof(value));

    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x10;
    sendbuff[3] = 0xAC;
    sendbuff[4] = protection_data[0];
    sendbuff[5] = protection_data[1];
    sendbuff[6] = protection_data[2];
    sendbuff[7] = protection_data[3];
    sendbuff[8] = protection_data[4];
    sendbuff[9] = protection_data[5];
    sendbuff[10] = protection_data[6];
    sendbuff[11] = protection_data[7];
    sendbuff[12] = over_frequency_slow/256;
    sendbuff[13] = over_frequency_slow%256;
    sendbuff[14] = protection_data[10];
    sendbuff[15] = protection_data[11];
    sendbuff[16] = protection_data[12];
    sendbuff[17] = protection_data[13];
    sendbuff[18] = protection_data[14];

    for (i=2; i<19; i++) {
        check = check + sendbuff[i];
    }

    sendbuff[19] = check >> 8;
    sendbuff[20] = check;
    sendbuff[21] = 0xFE;
    sendbuff[22] = 0xFE;

    printhexmsg(ECU_DBG_MAIN,"Set overfrequency slow (5)", (char *)sendbuff, 23);

    zb_broadcast_cmd((char *)sendbuff, 23);
    rt_thread_delay(1000);

    return 0;
}

//�����ָ�ʱ������(5��)
int set_grid_recovery_time_yc1000_5(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int grid_recovery_time;
    char protection_data[16] = {'\0'};

    read_protection_parameters_5(protection_data);
    grid_recovery_time = atof(value);

    sendbuff[0] = 0xFB;
    sendbuff[1] = 0xFB;
    sendbuff[2] = 0x10;
    sendbuff[3] = 0xAC;
    sendbuff[4] = protection_data[0];
    sendbuff[5] = protection_data[1];
    sendbuff[6] = protection_data[2];
    sendbuff[7] = protection_data[3];
    sendbuff[8] = protection_data[4];
    sendbuff[9] = protection_data[5];
    sendbuff[10] = protection_data[6];
    sendbuff[11] = protection_data[7];
    sendbuff[12] = protection_data[8];
    sendbuff[13] = protection_data[9];
    sendbuff[14] = grid_recovery_time/256;
    sendbuff[15] = grid_recovery_time%256;
    sendbuff[16] = protection_data[12];
    sendbuff[17] = protection_data[13];
    sendbuff[18] = protection_data[14];

    for (i=2; i<19; i++) {
        check = check + sendbuff[i];
    }

    sendbuff[19] = check >> 8;
    sendbuff[20] = check;
    sendbuff[21] = 0xFE;
    sendbuff[22] = 0xFE;

    printhexmsg(ECU_DBG_MAIN,"Set grid recovery time (5)", (char *)sendbuff, 23);

    zb_broadcast_cmd((char *)sendbuff, 23);
    rt_thread_delay(1000);

    return 0;
}

//Ƿѹ����3������
int set_under_voltage_stage_3_yc1000(char *value)
{
    unsigned char sendbuff[256]={'\0'};
    unsigned short check=0x00;
    int i;
    int data;


    data = (int)(atof(value) * 11614.45);

    sendbuff[0] = 0xFB;			//HEAD
    sendbuff[1] = 0xFB;			//HEAD
    sendbuff[2] = 0x06;			//LENGTH
    sendbuff[3] = 0x73;			//CMD
    sendbuff[4] = data/65536;			//DATA
    sendbuff[5] = data%65536/256;		//DATA
    sendbuff[6] = data%256;		//DATA
    sendbuff[7] = 0x00;
    sendbuff[8] = 0x00;

    for(i=2; i<9; i++){
        check = check + sendbuff[i];
    }

    sendbuff[9] = check >> 8;	//CHK
    sendbuff[10] = check;		//CHK
    sendbuff[11] = 0xFE;		//ccuid
    sendbuff[12] = 0xFE;		//ccuid

    printhexmsg(ECU_DBG_MAIN,"Set under voltage (stage 3)", (char *)sendbuff, 13);

    zb_broadcast_cmd((char *)sendbuff, 13);
    rt_thread_delay(1000);

    return 0;
}
