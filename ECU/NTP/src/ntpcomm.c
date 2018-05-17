/*****************************************************************************/
/* File      : ntpcomm.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <rtthread.h>
#include <lwip/netdb.h> /* Ϊ�˽�������������Ҫ����netdb.hͷ�ļ� */
#include <lwip/sockets.h> /* ʹ��BSD socket����Ҫ����sockets.hͷ�ļ� */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ntpapp.h"
#include "rtc.h"
#include "file.h"
#include "debug.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

#define NTPFRAC(x) (4294 * (x) + ((1981 * (x))>>11))

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/****************************����socket***************************************/
int create_socket_ntp( void )
{
    int sockfd;
    int addr_len;
    struct sockaddr_in addr_src;
    int ret;

    addr_len = sizeof( struct sockaddr_in );
    memset( &addr_src, 0, addr_len );
    addr_src.sin_family = AF_INET;
    addr_src.sin_addr.s_addr = htonl( INADDR_ANY );
    addr_src.sin_port = htons( 0 );

    if(-1==(sockfd = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP )))
    {
        printmsg(ECU_DBG_NTP,"Create socket error");
        return -1;
    }

    ret = bind(sockfd , (struct sockaddr*)&addr_src , addr_len);
    if(-1==ret)
    {
        printmsg(ECU_DBG_NTP,"socket Bind error!");
        return -1;
    }

    return sockfd;
}

/********************************����NTP������*******************************/
int connecttoserver(int sockfd, struct sockaddr_in * serversocket_in)
{
    int addr_len;
    struct sockaddr_in addr_dst;
    struct hostent * host;
    int ret;
    
    addr_len = sizeof(struct sockaddr_in);
    memset(&addr_dst, 0, addr_len);
    addr_dst.sin_family = AF_INET;
    host = gethostbyname("cn.pool.ntp.org");

    memcpy(&(addr_dst.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr_dst.sin_port = htons(123);

    memcpy(serversocket_in, &addr_dst, sizeof(struct sockaddr_in));
    ret = connect(sockfd, (struct sockaddr *)&addr_dst, addr_len);
    if(-1==ret)
    {
        printmsg(ECU_DBG_NTP,"Socket Connect error!");

        closesocket(sockfd);
        return -1;
    }
    else{
        printmsg(ECU_DBG_NTP,"Connect successfully!");
    }

    return sockfd;
}

/*********************************����Э���**********************************/
void send_packet(int sockfd)
{
    int bytes=0;
    NTPPACKET sendpacked;
    struct timeval now;
    memset(&sendpacked, 0, sizeof(sendpacked));
    sendpacked.header.NTPData.NTPMode.LI = 0;
    sendpacked.header.NTPData.NTPMode.VN = 3;
    sendpacked.header.NTPData.NTPMode.mode = 3;

    sendpacked.header.NTPData.NTPMode.stratum = 0;
    sendpacked.header.NTPData.NTPMode.poll = 0;
    sendpacked.header.NTPData.NTPMode.precision = 0;
    sendpacked.root_delay = 0;
    sendpacked.root_dispersion = 0;

    sendpacked.header.NTPData.head = htonl(sendpacked.header.NTPData.head);
    sendpacked.root_delay = htonl(sendpacked.root_dispersion);
    sendpacked.root_dispersion = htonl(sendpacked.root_dispersion);

    sendpacked.tratimestamp.sec = htonl(now.tv_sec)+0x83aa7e80; /* Transmit Timestamp coarse */

    if((rt_bool_t)(bytes=send(sockfd, &sendpacked, sizeof(sendpacked), 0)))
    {
        printmsg(ECU_DBG_NTP,"send successfully!");
        printdecmsg(ECU_DBG_NTP,"send bytes",bytes);
    }
    else{
        printmsg(ECU_DBG_NTP,"send failure!");
    }
}

int receive_packet(int sockfd, NTPPACKET *recvpacked, struct sockaddr_in * serversocket_in)
{
    int receivebytes=0;
    int addr_len = sizeof(struct sockaddr_in);

    {
        receivebytes = recvfrom(sockfd, recvpacked, sizeof(NTPPACKET), 0, (struct sockaddr *)serversocket_in, (socklen_t *)&addr_len);
        printdecmsg(ECU_DBG_NTP,"recevicing",receivebytes);
    }

    if(-1==receivebytes)
    {
        printmsg(ECU_DBG_NTP,"Receive error!");
        closesocket(sockfd);
        return -1;
    }

    return receivebytes;
}

void gettimepacket(NTPPACKET *receivepacket, struct timeval * new_time)
{
    NTPTIME trantime;
    trantime.sec = ntohl(receivepacket->tratimestamp.sec)-0x83aa7e80;
    new_time->tv_sec = trantime.sec;
    printdecmsg(ECU_DBG_NTP,"new time:",trantime.sec);

}

extern int day_tab[2][12];

void transfer_time(struct tm *timenow,int timezone)
{
    int leapflag = 0;


    leapflag = leap((timenow->tm_year+1900));
    //ת��Сʱʱ��
    timenow->tm_hour  = timenow->tm_hour + timezone;//ʱ�����  ����24  ����  С��0

    if(timenow->tm_hour > 24)
    {
        timenow->tm_hour  -= 24;
        //����24�㣬ʱ������һ��
        //�ж��Ƿ��ǵ��µ����һ��
        if(timenow->tm_mday == day_tab[leapflag][timenow->tm_mon])
        {
            //��������һ�죬��ת���¸��µĵ�һ��
            timenow->tm_mday = 1;

            //�ж��Ƿ������һ����
            if(11 == timenow->tm_mon)
            {
                timenow->tm_mon = 0;
                timenow->tm_year++;
            }else
            {
                timenow->tm_mon++;
            }

        }
        else
        {
            timenow->tm_mday++;
        }
    }
    else if(timenow->tm_hour < 0)
    {
        //С�� 0�㣬ʱ�����һ��
        timenow->tm_hour  += 24;
        //С��24�㣬ʱ�����һ��
        //�ж��Ƿ��ǵ��µĵ�һ��
        if(1 == day_tab[leapflag][timenow->tm_mon])
        {


            //�ж��Ƿ��ǵ�һ����
            if(0 == timenow->tm_mon)
            {
                //����ǵ�һ�죬��ת���ϸ��µ����һ��
                timenow->tm_mday = 31;
                timenow->tm_mon = 11;
                timenow->tm_year--;
            }else
            {
                //����ǵ�һ�죬��ת���ϸ��µ����һ��
                timenow->tm_mday = day_tab[leapflag][timenow->tm_mon-1];
                timenow->tm_mon--;
            }

        }
        else
        {
            timenow->tm_mday--;
        }
    }
}

void update_time(struct timeval * new_time)
{
    char nowtime[15] = {'\0'};
    int timezone = 8;
    struct tm *timenow;
    time_t newtime = (time_t)new_time->tv_sec;
    rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
    timenow = localtime(&newtime);
    //��ȡʱ��	�ļ���Ϊ/yuneng/timezone.con
    timezone = 	getTimeZone();
    printdecmsg(ECU_DBG_NTP,"timezone",timezone);
    transfer_time(timenow,timezone);
    sprintf(nowtime,"%04d%02d%02d%02d%02d%02d",(timenow->tm_year+1900),(timenow->tm_mon+1),timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    print2msg(ECU_DBG_NTP,"nowtime",nowtime);
    set_time(nowtime);
    rt_mutex_release(record_data_lock);

}

