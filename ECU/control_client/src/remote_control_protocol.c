/*****************************************************************************/
/* File      : remote_control_protocol.c                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "myfile.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define VERSION 13

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* ����Э��ͷ */
int msg_Header(char *sendbuffer, const char *cmd_id)
{
	memset(sendbuffer, 0, sizeof(sendbuffer));
	snprintf(sendbuffer, 18+1, "APS%02d00000%.4sAAA0", VERSION, cmd_id);
	return 0;
}

/*
 * ECU��EMA������Ϣ
 *
 * ���룺�����ͻ���ָ�룺sendbuffer
 * 		ECU���룺ecuid
 * */
int msg_REQ(char *sendbuffer)
{
	char msg_length[6] = {'\0'};
	msg_Header(sendbuffer, "A101");
	strcat(sendbuffer, ecu.id);
	strcat(sendbuffer, "A10100000000000000END\n");
	
	sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	strncpy(&sendbuffer[5], msg_length, 5);
		
	return 0;
}

/*
 * ECU��EMAӦ����Ϣ
 *
 * ���룺�����ͻ���ָ�룺sendbuffer
 * 		ECU���룺ecuid
 * 		Э������ţ�cmd_id
 * 		Э��ʱ�����timestamp
 * 		Ӧ���־λ��ack_flag
 * */
int msg_ACK(char *sendbuffer,
		const char *cmd_id, const char *timestamp, int ack_flag)
{
	char msg_body[35] = {'\0'};
	char msg_length[6] = {'\0'};

	msg_Header(sendbuffer, "A100");
	sprintf(msg_body, "%.12s%.4s%.14s%1dEND\n", ecu.id, cmd_id, timestamp, ack_flag);
	strcat(sendbuffer, msg_body);
	sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	strncpy(&sendbuffer[5], msg_length, 5);
	
	return 0;
}

/*
 *  ��Э���н��������ͱ���(�����ԡ�A����������)
 *
 *  ���룺���ͱ������ַ����е���ʼָ�룺s
 *  	 ���ͱ�����ռ�ַ����ĳ��ȣ�len *
 *  ��������ͱ���
 */
int msg_get_int(const char *s, int len)
{
	int i, count = 0;
	char buffer[16] = {'\0'};

	strncpy(buffer, s, len);
	for(i=0; i<len; i++)
	{
		if('A' == buffer[i]){
			buffer[i] = '0';
			count++;
		}
	}
	if(count >= len)
		return -1;
	else
		return atoi(buffer);
}

/* ��ȡЭ����Ϣ�汾�� */
int msg_version(const char *msg)
{
	return msg_get_int(&msg[3], 2);
}

/* Э����Ϣ���� */
int msg_length(const char *msg)
{
	return msg_get_int(&msg[5], 5);
}

/* ��ȡЭ������� */
int msg_cmd_id(const char *msg)
{
	int cmd_id;

	cmd_id = msg_get_int(&msg[10], 4);
	if(cmd_id == 101){
		return msg_get_int(&msg[30], 4);
	}
	return cmd_id;
}

/* ��ȡЭ����Ϣ��ˮ�� */
int msg_seq_id(const char *msg)
{
	return msg_get_int(&msg[14], 4);
}

/*
 * 	���ÿ����Ϣͨ�ò��ֵĸ�ʽ
 *
 * 	���룺��Ϣ�ַ���ָ�룺msg
 * 	�������ȷ��0 , ����-1
 * */
int msg_format_check(const char *msg)
{
	//Э��ͷAPS
	if(strncmp(msg, "APS", 3)){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: APS");
		return -1;
	}

	//�汾��
	//msg_version(msg);

	//Э�鳤��
	if(msg_length(msg) != strlen(msg)){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: length");
		return -1;
	}

	//ECU_ID
	if(strncmp(&msg[18], ecu.id, 12)){
		if(msg_get_int(&msg[10], 4) != 123){ //A123Ӧ��û��ECU_ID
			printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: ecu_id");
			return -1;
		}
	}

	//Э��βEND

	return 0;
}

/*
 * 	����������Ϣ�����Ƿ���ECU��Ϣ�е�NUM������ͬ
 *
 * 	���룺�������Ϣ��ʼ����ַ���ָ�룺s
 * 		 ECU��Ϣ�е�NUM������num
 * 		 ÿ���������Ϣ�ĳ���(������END)��len
 *		 �Ƿ�ÿ���������Ϣ����END��β��flag
 * 	�������ȷ��true , ����false
 * */
int msg_num_check(const char *s, int num, int len, int flag)
{
	if(flag){
		return (strlen(s) == (num*(len+3)));
	}
	else{
		return (strlen(s) == (num*len + 3));
	}
}

/* ���ַ������ӵ�Э���ַ��� */
char *msgcat_s(char *s, int size, const char *value)
{
	int i, length;

	length = strlen(value);
	if(size < length){
		strncat(s, value, size);
		return s;
	}
	for(i=size; i>length; i--){
		strcat(s, "0");
	}
	strcat(s, value);
	return s;
}

/* �������������ӵ�Э���ַ��� */
char *msgcat_d(char *s, int size, int value)
{
	int i, length;
	char buffer[32] = {'\0'};

	if(value < 0){
		for(i=0; i<size; i++)
			strcat(s, "A");
		return s;
	}
	sprintf(buffer, "%d", value);
	length = strlen(buffer);
	if(size < length){
		strncat(s, buffer, size);
		return s;
	}
	for(i=size; i>length; i--){
		strcat(s, "0");
	}
	strcat(s, buffer);
	return s;
}

/* �Ӹ����ַ����и��Ƴ�һ�ε�ENDΪֹ���ַ��� */
int msg_get_one_section(char *s, const char *msg)
{
	memset(s, 0, sizeof(s));
	if(strstr(msg, "END") == NULL){
		return -1;
	}
	strncpy(s, msg, (int)(strstr(msg, "END")-msg));
	return strlen(s);
}

