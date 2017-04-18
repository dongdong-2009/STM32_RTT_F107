#ifndef __THFTPAPI_H__
#define __THFTPAPI_H__
//
//  THFTPAPI.h
//  MyFTP
//

 
//���ӷ�����  OK
int ftp_connect( char *host, int port, char *user, char *pwd );
//�Ͽ�������  OK
int ftp_quit( int c_sock);
 
//���ñ�ʾ���� OK
int ftp_type( int c_sock, char mode );
 
//�ı乤��Ŀ¼ OK
int ftp_cwd( int c_sock, char *path );
//�ص���һ��Ŀ¼  OK
int ftp_cdup( int c_sock );
//����Ŀ¼  OK
int ftp_mkd( int c_sock, char *path );
//�б�
int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len);
 
//�����ļ�
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//�ϴ��ļ�
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
 
//�޸��ļ���&�ƶ�Ŀ¼ OK
int ftp_renamefile( int c_sock, char *s, char *d );
//ɾ���ļ�  OK
int ftp_deletefile( int c_sock, char *s );
//ɾ��Ŀ¼  OK
int ftp_deletefolder( int c_sock, char *s );

//�����ļ�
int ftpgetfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
	
//�ϴ��ļ�
int ftpputfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);


#endif /*__THFTPAPI_H__*/
