#ifndef __NTPCOMM_H__
#define __NTPCOMM_H__

int create_socket( void );
/********************************����NTP������*******************************/
int connecttoserver(int sockfd, struct sockaddr_in * serversocket_in);
/*********************************����Э���**********************************/
void send_packet(int sockfd);
int receive_packet(int sockfd, NTPPACKET *recvpacked, struct sockaddr_in * serversocket_in);
void gettimepacket(NTPPACKET *receivepacket, struct timeval * new_time);
void update_time(struct timeval * new_time);

#endif /*__NTPCOMM_H__*/ 
