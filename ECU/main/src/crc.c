/*
 * Created by zhyf
 * Created on 2016.07.06
 * Description: �˰汾CRCУ������ECU-C�ϣ���Sensor�ĵ�Ƭ��ͨ��
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define POLY 0x1021

unsigned short UpdateCRC(unsigned short CRC_acc, unsigned char CRC_input)
{
    unsigned short i;
    CRC_acc = CRC_acc ^ (CRC_input << 8);

    for (i = 0; i < 8; i++)
    {
        // Check if the MSB is set (if MSB is 1, then the POLY can "divide"
        // into the "dividend")
        if ((CRC_acc & 0x8000) == 0x8000)
        {
            // if so, shift the CRC value, and XOR "subtract" the poly
            CRC_acc = CRC_acc << 1;
            CRC_acc ^= POLY;
        }
        else
        {
            // if not, just shift the CRC value
            CRC_acc = CRC_acc << 1;
        }
    }

    return CRC_acc;
}

unsigned short crc_array(char *buff, int len)
{
    unsigned short result = 0xFFFF;
    int i = 0;

    for(i=0; i<len; i++)
        result = UpdateCRC(result, buff[i]);

    return result;
}

unsigned short crc_file(int crc_fd)
{
    unsigned short result = 0xFFFF;
    char *package_buff =NULL;
    int ret_size,i;
    package_buff = malloc(4096);
    memset(package_buff,0x00,4096);

    if(crc_fd>=0)
    {
        while((ret_size = read(crc_fd, package_buff, 4096))>0)
        {
            for(i = 0;i<ret_size;i++)
            {
                result = UpdateCRC(result, package_buff[i]);
            }
        }
    }
    free(package_buff);
    package_buff = NULL;
    return result;
}
