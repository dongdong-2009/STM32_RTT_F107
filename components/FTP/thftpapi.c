//
//  THFTPAPI.c
//  MyFTP
//
//  Created by TanHao on 13-6-6.
//  Copyright (c) 2013�� http://www.tanhao.me. All rights reserved.
//
 
#include "thftpapi.h"
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"
#include "datetime.h"


 
//����һ��socket������
int socket_connect(char *host,int port)
{
    struct sockaddr_in address;
    int s, opvalue;
    socklen_t slen;
    //���ý��պͷ��ͳ�ʱ
    struct timeval timeo = {15, 0}; 
		struct hostent* server;
    opvalue = 8;
    slen = sizeof(opvalue);
    memset(&address, 0, sizeof(address));
     
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        setsockopt(s, IPPROTO_IP, IP_TOS, &opvalue, slen) < 0)
        return -1;
     
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
     
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
     
    server = gethostbyname(host);
    if (!server)
        return -1;

    memcpy(&address.sin_addr.s_addr, server->h_addr, server->h_length); 
    if (connect(s, (struct sockaddr*) &address, sizeof(address)) == -1)
        return -1;
     
    return s;
}
 
//���ӵ�һ��ftp�ķ�����������socket
int connect_server( char *host, int port )
{   
    int       ctrl_sock;
    char      buf[512];
    int       result;
    ssize_t   len;
    fd_set rd;
		struct timeval timeout;
	
    ctrl_sock = socket_connect(host, port);
    if (ctrl_sock == -1) {
        return -1;
    }
		FD_ZERO(&rd);
		FD_SET(ctrl_sock, &rd);
		timeout.tv_sec = 12;
		timeout.tv_usec = 0;
		len = select(ctrl_sock+1, &rd, NULL, NULL, &timeout);
		if(len <= 0){
			return -1;
		}else
		{
			memset(buf,0x00,sizeof(buf));
			len = recv( ctrl_sock, buf, 512, 0 );
			buf[len] = 0;
			sscanf( buf, "%d", &result );
			if ( result != 220 ) {
					closesocket( ctrl_sock );
					return -1;
			}
			 
			return ctrl_sock;
		}
}
 
//��������,���ؽ��
int ftp_sendcmd_re( int sock, char *cmd, void *re_buf, ssize_t *len)
{
    char        buf[512];
    ssize_t     r_len;
    fd_set rd;
		struct timeval timeout;	
    if ( send( sock, cmd, strlen(cmd), 0 ) == -1 )
        return -1;
     
		FD_ZERO(&rd);
		FD_SET(sock, &rd);
		timeout.tv_sec = 12;
		timeout.tv_usec = 0;
		r_len = select(sock+1, &rd, NULL, NULL, &timeout);
		if(len <= 0){
			return -1;
		}else
		{
			r_len = recv( sock, buf, 512, 0 );
			if ( r_len < 1 ) return -1;
			buf[r_len] = 0;
			 
			if (len != NULL) *len = r_len;
			if (re_buf != NULL) sprintf(re_buf, "%s", buf);
			 
			return 0;
		}
}
 
//��������,���ر��
int ftp_sendcmd( int sock, char *cmd )
{
    char     buf[512];
    int      result;
    ssize_t  len;
     
    result = ftp_sendcmd_re(sock, cmd, buf, &len);
    if (result == 0)
    {
        sscanf( buf, "%d", &result );
    }
     
    return result;
}
 
//��¼ftp������
int login_server( int sock, char *user, char *pwd )
{
    char    buf[128];
    int     result;
     
    sprintf( buf, "USER %s\r\n", user );
    result = ftp_sendcmd( sock, buf );
    if ( result == 230 ) return 0;
    else if ( result == 331 ) {
        sprintf( buf, "PASS %s\r\n", pwd );
        if ( ftp_sendcmd( sock, buf ) != 230 ) return -1;
        return 0;
    }
    else
        return -1;
}
 
int create_datasock( int ctrl_sock )
{
    int     lsn_sock;
    int     port;
    int     len;
    struct sockaddr_in sin;
    char    cmd[128];
     
    lsn_sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( lsn_sock == -1 ) return -1;
    memset( (char *)&sin, 0, sizeof(sin) );
    sin.sin_family = AF_INET;
    if( bind(lsn_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1 ) {
        closesocket( lsn_sock );
        return -1;
    }
     
    if( listen(lsn_sock, 2) == -1 ) {
        closesocket( lsn_sock );
        return -1;
    }
     
    len = sizeof( struct sockaddr );
    if ( getsockname( lsn_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        closesocket( lsn_sock );
        return -1;
    }
    port = sin.sin_port;
     
    if( getsockname( ctrl_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        closesocket( lsn_sock );
        return -1;
    }
     
    sprintf( cmd, "PORT %d,%d,%d,%d,%d,%d\r\n",
            sin.sin_addr.s_addr&0x000000FF,
            (sin.sin_addr.s_addr&0x0000FF00)>>8,
            (sin.sin_addr.s_addr&0x00FF0000)>>16,
            (sin.sin_addr.s_addr&0xFF000000)>>24,
            port>>8, port&0xff );
     
    if ( ftp_sendcmd( ctrl_sock, cmd ) != 200 ) {
        closesocket( lsn_sock );
        return -1;
    }
    return lsn_sock;
}
 
//���ӵ�PASV�ӿ�
int ftp_pasv_connect( int c_sock )
{
    int     r_sock;
    int     send_re;
    ssize_t len;
    int     addr[6];
    char    buf[512];
    char    re_buf[512];
     
    //����PASV����ģʽ
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "PASV\r\n");
    send_re = ftp_sendcmd_re( c_sock, buf, re_buf, &len);
    if (send_re == 0) {
        sscanf(re_buf, "%*[^(](%d,%d,%d,%d,%d,%d)",&addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
    }
     
    //����PASV�˿�
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
    r_sock = socket_connect(buf,addr[4]*256+addr[5]);
     
    return r_sock;
}
 
//��ʾ����
int ftp_type( int c_sock, char mode )
{
    char    buf[128];
    sprintf( buf, "TYPE %c\r\n", mode );
    if ( ftp_sendcmd( c_sock, buf ) != 200 )
        return -1;
    else
        return 0;
}
 
//�ı乤��Ŀ¼
int ftp_cwd( int c_sock, char *path )
{
    char    buf[128];
    int     re;
    sprintf( buf, "CWD %s\r\n", path );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 )
        return -1;
    else
        return 0;
}
 
//�ص���һ��Ŀ¼
int ftp_cdup( int c_sock )
{
    int     re;
    re = ftp_sendcmd( c_sock, "CDUP\r\n" );
    if ( re != 250 )
        return re;
    else
        return 0;
}
 
//����Ŀ¼
int ftp_mkd( int c_sock, char *path )
{
    char    buf[512];
    int     re;
    sprintf( buf, "MKD %s\r\n", path );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 257 )
        return re;
    else
        return 0;
}
 
//�б�
int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len)
{
    int     d_sock;
    char    buf[512];
    int     send_re;
    int     result;
    ssize_t     len,buf_len,total_len;
		void *re_buf;
     
    //���ӵ�PASV�ӿ�
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1) {
        return -1;
    }
     
    //����LIST����
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "LIST %s\r\n", path);
    send_re = ftp_sendcmd( c_sock, buf );
    if (send_re >= 300 || send_re == 0)
        return send_re;
     
    len=total_len = 0;
    buf_len = 512;
    re_buf = malloc(buf_len);
    while ( (len = recv( d_sock, buf, 512, 0 )) > 0 )
    {
        if (total_len+len > buf_len)
        {
						void *re_buf_n;
            buf_len *= 2;
            re_buf_n = malloc(buf_len);
            memcpy(re_buf_n, re_buf, total_len);
            free(re_buf);
            re_buf = re_buf_n;
        }
//        memcpy(re_buf+total_len, buf, len);
        total_len += len;
    }
    closesocket( d_sock );
     
    //����������շ���ֵ
    memset(buf,0x00, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result != 226 )
    {
        free(re_buf);
        return result;
    }
     
    *data = re_buf;
    *data_len = total_len;
     
    return 0;
}
 
//�����ļ�
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock = 0;
    ssize_t     len,write_len,sum=0;
    char    buf[1461];
    int     handle;
    int     result =0;
    fd_set rd;
		struct timeval timeout;	
	  //char time[20];
    //�򿪱����ļ�
    handle = fileopen( d,  O_WRONLY | O_CREAT | O_TRUNC, 0 );
    if ( handle == -1 ) 
		{
			printf("fffffffff\n");
			return -1;
		}
    //���ô���ģʽ
    ftp_type(c_sock, 'I');
     
    //���ӵ�PASV�ӿ�
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
			printf("qqqqqqqqq\n");
        fileclose(handle);
        return -1;
    }
     
    //����STOR����
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "RETR %s\r\n", s );
    result = ftp_sendcmd( c_sock, buf );
    if (result >= 300 || result == 0)
    {
			printf("111111111\n");
        fileclose(handle);
        return result;
    }
     
    //��ʼ��PASV��ȡ����
    memset(buf,0x00, sizeof(buf));
		FD_ZERO(&rd);
		FD_SET(d_sock, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		while (1) {
			len = select(d_sock+1, &rd, NULL, NULL, &timeout);
			if(len <= 0){
				printf("select out\n");
				closesocket( d_sock );
				fileclose( handle );
				return -1;
			}else
			{
				//getcurrenttime(time);
				//printf("%03d :time:%s  :len1:%7d ",++i,time,len);
				if((len = recv( d_sock, buf, 1460, MSG_DONTWAIT )) > 0 )
				{
					sum += len;
					//printf("len2:%7d sum:%7d\n",len,sum);

					write_len = fileWrite( handle, buf, len );
					if (write_len != len || (stop != NULL && *stop))
					{
							closesocket( d_sock );
							fileclose( handle );
							return -1;
					}
					 
					if (stor_size != NULL)
					{
							*stor_size += write_len;
					}
					memset(buf,0x00, sizeof(buf));
					if(len < 1){	

						printf("\ntransfer:%d\n",len);
						break;
					}
				}
				else
				{
					break;
				}
			}				
		}
		closesocket( d_sock );
		fileclose( handle );
		
		
    //����������շ���ֵ
    memset(buf,0x00, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result >= 300 ) {
        return result;
    }
    return 0;
}


//�ϴ��ļ�
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock;
    ssize_t     len,send_len;
    char    buf[512];
    int     handle;
    int send_re;
    int result = 0;
	
    //�򿪱����ļ�
    handle = fileopen( s,  O_RDONLY,0);
    if ( handle == -1 ) return -1;
     
    //���ô���ģʽ
    ftp_type(c_sock, 'I');
     
    //���ӵ�PASV�ӿ�
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
        fileclose(handle);
        return -1;
    }

    //����STOR����
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "STOR %s\r\n", d );
    send_re = ftp_sendcmd( c_sock, buf );
    if (send_re >= 300 || send_re == 0)
    {
        fileclose(handle);
        return send_re;
    }
     
    //��ʼ��PASVͨ��д����
    memset(buf,0x00, sizeof(buf));
    while ( (len = fileRead( handle, buf, 512)) > 0)
    {
        send_len = send(d_sock, buf, len, 0);
			
        if (send_len != len ||
            (stop != NULL && *stop))
        {
            closesocket( d_sock );
            fileclose( handle );
            return -1;
        }
         
        if (stor_size != NULL)
        {
            *stor_size += send_len;
        }
    }
    closesocket( d_sock );
    fileclose( handle );
     
    //����������շ���ֵ
    memset(buf,0x00, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result >= 300 ) {
        return result;
    }
    return 0;
}
 
//�޸��ļ���&�ƶ�Ŀ¼
int ftp_renamefile( int c_sock, char *s, char *d )
{
    char    buf[512];
    int     re;
     
    sprintf( buf, "RNFR %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 350 ) return re;
    sprintf( buf, "RNTO %s\r\n", d );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}
 
//ɾ���ļ�
int ftp_deletefile( int c_sock, char *s )
{
    char    buf[512];
    int     re;
     
    sprintf( buf, "DELE %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}
 
//ɾ��Ŀ¼
int ftp_deletefolder( int c_sock, char *s )
{
    char    buf[512];
    int     re;
     
    sprintf( buf, "RMD %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}
 
//���ӷ�����
int ftp_connect( char *host, int port, char *user, char *pwd )
{
    int     c_sock;
    c_sock = connect_server( host, port );
    if ( c_sock == -1 ) return -1;
    if ( login_server( c_sock, user, pwd ) == -1 ) {
        closesocket( c_sock );
        return -1;
    }
    return c_sock;
}
 
//�Ͽ�������
int ftp_quit( int c_sock)
{
    int re = 0;
    re = ftp_sendcmd( c_sock, "QUIT\r\n" );
    closesocket( c_sock );
    return re;
}

//�����ļ�
int ftpgetfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile)
{
	unsigned long long stor_size = 0;
	int stop = 0,ret = 0;
	int sockfd = ftp_connect( host, port, user, pwd  );
	if(sockfd != -1)
	{
		printf("ftp connect successful %d\n",sockfd);	
	}
	
	ret = ftp_retrfile(sockfd, remotefile, localfile ,&stor_size, &stop);
	printf("\nret :%d\nstor_size:%lld  stop:%d\n",ret,stor_size,stop);
	ftp_quit( sockfd);
	return ret;
}

//�ϴ��ļ�
int ftpputfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile)
{
	unsigned long long stor_size = 0;
	int stop = 0,ret = 0;
	int sockfd = ftp_connect( host, port, user, pwd  );

	if(sockfd != -1)
	{
		printf("ftp connect successful %d\n",sockfd);	
	}

  ret = ftp_storfile(sockfd, localfile,remotefile ,&stor_size, &stop);
	printf("stor_size:%lld  stop:%d\n",stor_size,stop);
	ftp_quit( sockfd);
	return ret;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void getfile(char *remoteFile, char *localFile)
{
	ftpgetfile("192.168.1.107",21,"admin","admin",remoteFile,localFile);
}
FINSH_FUNCTION_EXPORT(getfile,get file from ftp.)

void putfile(char *remoteFile, char *localFile)
{
	ftpputfile("192.168.1.107",21,"admin","admin",remoteFile,localFile);
}
FINSH_FUNCTION_EXPORT(putfile,put file from ftp.)
#endif

 
