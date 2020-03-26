#ifndef _MAIN_H
#define _MAIN_H

#include <asm/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define N 1

typedef unsigned char byte;

enum fsm {
	KEEP_ALIVE = 0,
	MESSAGE = 1,
	IDLE = 2,
	OTHER = 3
}FSM;


extern void* timer_main(void);
struct keep_alive
{
	byte header[3];
	byte msg_type;
	byte id[4];
	byte proto_type;
	byte proto_version;
	byte data_len[4];
	byte city_id;
	byte way_id;
	byte group_id[2];
	byte device_id;
	byte data_year;
	byte data_month;
	byte data_day;
	byte data_hour;
	byte data_min;
	byte data_sec;
	byte data_msec[2];
	byte data_reserved[18];
	byte data[N];
	byte crc_checksum[2];
	byte tail[3];
};

/* FUNCTION */
extern void* link_main(void);
extern void set_epoll_status(int epoll_fd,struct epoll_event ev,int socket_fd, unsigned int EV);
extern void get_keep_alive_data(struct keep_alive * buffer,int len);
extern void get_message_data(struct keep_alive * buffer,int len);

#endif
