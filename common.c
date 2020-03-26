#include "common.h"

void set_epoll_status(int epoll_fd,struct epoll_event ev,int socket_fd, unsigned int EV)
{
	ev.data.fd = socket_fd;
	ev.events = EV | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd,&ev);
}

void get_keep_alive_data(struct keep_alive * buffer,int len)
{
	memset(buffer,0,len);
	byte header[3] = {0xAA,0xAB,0xAC};
	byte msg_type = 0x3; 
	int  id = 0;;
	byte proto_type = 0x02;
	byte proto_version = 0x10;
	int data_len = 45-2+N;
	byte city_id = 0x01;
	byte way_id = 0x0;
	byte group_id[2] = {0x0,0x1};
	byte device_id = 0x1;
	byte data_year = 0x14;
	byte data_month = 0x3;
	byte data_day = 0x18;
	byte data_hour = 0x0;
	byte data_min = 0x0;
	byte data_sec = 0x0;
	byte data_msec[2] = {0x0,0x2};
	byte data_reserved[18] ={0x0};
	byte data[N] = {0x0};
	byte crc_checksum[2];
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->header,header,3);
	memcpy(&buffer->msg_type,&msg_type,1);
	memcpy(buffer->id,(byte *)&id,4);
	memcpy(&buffer->proto_type,&proto_type,1);
	memcpy(&buffer->proto_version,&proto_version,0x10);
	memcpy(buffer->data_len,(byte *)&data_len,4);
	memcpy(&buffer->city_id,&city_id,1);
	memcpy(&buffer->way_id,&way_id,1);
	memcpy(buffer->group_id,group_id,2);
	memcpy(&buffer->device_id,&device_id,1);
	memcpy(&buffer->data_year,&data_year,1);
	memcpy(&buffer->data_month,&data_month,1);
	memcpy(&buffer->data_day,&data_day,1);
	memcpy(&buffer->data_hour,&data_hour,1);
	memcpy(&buffer->data_min,&data_min,1);
	memcpy(&buffer->data_sec,&data_sec,1);
	memcpy(buffer->data_msec,data_msec,2);
	memcpy(buffer->data_reserved,data_reserved,18);
	memcpy(buffer->data,data,N);
	memcpy(buffer->crc_checksum,crc_checksum,2);
	memcpy(buffer->tail,tail,3);
}


void get_message_data(struct keep_alive * buffer,int len)
{
	memset(buffer,0,len);
	byte header[3] = {0xAA,0xAB,0xAC};
	byte msg_type = 0x3; 
	int  id = 0;;
	byte proto_type = 0x02;
	byte proto_version = 0x10;
	int data_len = 45-2+N;
	byte city_id = 0x01;
	byte way_id = 0x0;
	byte group_id[2] = {0x0,0x1};
	byte device_id = 0x1;
	byte data_year = 0x14;
	byte data_month = 0x3;
	byte data_day = 0x18;
	byte data_hour = 0x0;
	byte data_min = 0x0;
	byte data_sec = 0x0;
	byte data_msec[2] = {0x0,0x2};
	byte data_reserved[18] ={0x0};
	byte data[N] = {0x0};
	byte crc_checksum[2];
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->header,header,3);
	memcpy(&buffer->msg_type,&msg_type,1);
	memcpy(buffer->id,(byte *)&id,4);
	memcpy(&buffer->proto_type,&proto_type,1);
	memcpy(&buffer->proto_version,&proto_version,0x10);
	memcpy(buffer->data_len,(byte *)&data_len,4);
	memcpy(&buffer->city_id,&city_id,1);
	memcpy(&buffer->way_id,&way_id,1);
	memcpy(buffer->group_id,group_id,2);
	memcpy(&buffer->device_id,&device_id,1);
	memcpy(&buffer->data_year,&data_year,1);
	memcpy(&buffer->data_month,&data_month,1);
	memcpy(&buffer->data_day,&data_day,1);
	memcpy(&buffer->data_hour,&data_hour,1);
	memcpy(&buffer->data_min,&data_min,1);
	memcpy(&buffer->data_sec,&data_sec,1);
	memcpy(buffer->data_msec,data_msec,2);
	memcpy(buffer->data_reserved,data_reserved,18);
	memcpy(buffer->data,data,N);
	memcpy(buffer->crc_checksum,crc_checksum,2);
	memcpy(buffer->tail,tail,3);
}

