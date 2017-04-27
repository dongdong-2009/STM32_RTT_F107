#ifndef __CLIENT_H__
#define __CLIENT_H__

int createsocket(void);
int connect_socket(int fd_sock);
void close_socket(int fd_sock);
int clear_send_flag(char *readbuff);
int recv_response(int fd_sock, char *readbuff);
int detection_resendflag2(void);
int change_resendflag(char *time,char flag);
int send_record(int fd_sock, char *sendbuff, char *send_date_time);
int preprocess(void);
void client_thread_entry(void* parameter);
int search_readflag(char *data,char * time, int *flag,char sendflag);
#endif	/*__CLIENT_H__*/
