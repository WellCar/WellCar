#include "common.h"
#include "config.h"

extern unsigned int G_ID;

time_t time_p;
struct tm *p;

/* Delay */
void sleep_ms(unsigned int secs)
{
	struct timeval tval;
	tval.tv_sec=secs/1000;
	tval.tv_usec=(secs*1000)%1000000;
	select(0,NULL,NULL,NULL,&tval);

}

/* Time FUN */ 


void show_time()
{
	struct tm * p;
	time(&time_p);
	p = gmtime(&time_p);
	printf("%d %d %d\n",(1900+p->tm_year), (1+p->tm_mon),p->tm_mday);
	printf("%d %d %d\n", p->tm_hour, p->tm_min, p->tm_sec);
}

void get_time_p()
{
	time(&time_p);
	p = gmtime(&time_p);
}

byte get_year_hex()
{
	get_time_p();
	//printf("YEAR:%X\n",(1900+p->tm_year)-2000);
	return (1900+p->tm_year)-2000;
}

byte get_month_hex()
{
	get_time_p();
	return (1+p->tm_mon);
}

byte get_day_hex()
{
	get_time_p();
	return (1+p->tm_mday);
}

byte get_hour_hex()
{
	get_time_p();
	return p->tm_hour;
}

byte get_min_hex()
{
	get_time_p();
	return p->tm_min;
}

byte get_sec_hex()
{
	get_time_p();
	return p->tm_sec;
}

unsigned short get_usec_hex()
{
	struct timeval tval;
	gettimeofday(&tval,NULL);
	//printf("USEC: %d \n", (int )(tval.tv_sec*1000+tval.tv_usec/1000));
	return tval.tv_sec*1000+tval.tv_usec/1000;
}


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
	memcpy(buffer->header,header,3);
	byte msg_type = 0x3; 
	memcpy(&buffer->msg_type,&msg_type,1);
	int  id = 0;
	memcpy(buffer->id,(byte *)&id,4);
	byte proto_type = 0x02;
	memcpy(&buffer->proto_type,&proto_type,1);
	byte proto_version = 0x10;
	memcpy(&buffer->proto_version,&proto_version,0x10);
	int data_len = 45-2+N;
	memcpy(buffer->data_len,(byte *)&data_len,4);
	byte city_id = 0x01;
	memcpy(&buffer->city_id,&city_id,1);
	byte way_id = 0x0;
	memcpy(&buffer->way_id,&way_id,1);
	byte group_id[2] = {0x0,0x1};
	memcpy(buffer->group_id,group_id,2);
	byte device_id = 0x1;
	memcpy(&buffer->device_id,&device_id,1);
	byte data_year = get_year_hex();;
	memcpy(&buffer->data_year,&data_year,1);
	byte data_month = get_month_hex();
	memcpy(&buffer->data_month,&data_month,1);
	byte data_day = get_day_hex();
	memcpy(&buffer->data_day,&data_day,1);
	byte data_hour = get_hour_hex();
	memcpy(&buffer->data_hour,&data_hour,1);
	byte data_min = get_min_hex();
	memcpy(&buffer->data_min,&data_min,1);
	byte data_sec = get_sec_hex();
	memcpy(&buffer->data_sec,&data_sec,1);
	unsigned short data_msec = get_usec_hex();
	memcpy(buffer->data_msec,&data_msec,2);
	byte data_reserved[18] ={0x0};
	memcpy(buffer->data_reserved,data_reserved,18);
	byte data[N] = {0x0};
	memcpy(buffer->data,data,N);
	unsigned short crc_checksum =  CRC16_MODBUS(&(buffer->msg_type),(35+N-1-7));
	memcpy(buffer->crc_checksum,&crc_checksum,2);
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->tail,tail,3);
}

void get_keep_alive_data_confirm(struct keep_alive_confirm * buffer,struct keep_alive * recv,int len)
{
	memset(buffer,0,len);
	byte header[3] = {0xAA,0xAB,0xAC};
	memcpy(buffer->header,header,3);
	byte msg_type = 0x3; 
	memcpy(&buffer->msg_type,&msg_type,1);
	int  id = 0;
	memcpy(buffer->id,(byte *)&id,4);
	byte proto_type = 0x02;
	memcpy(&buffer->proto_type,&proto_type,1);
	byte proto_version = 0x10;
	memcpy(&buffer->proto_version,&proto_version,0x10);
	int data_len = 45-2+N;
	memcpy(buffer->data_len,(byte *)&data_len,4);
	byte city_id = 0x01;
	memcpy(&buffer->city_id,&city_id,1);
	byte way_id = 0x0;
	memcpy(&buffer->way_id,&way_id,1);
	byte group_id[2] = {0x0,0x1};
	memcpy(buffer->group_id,group_id,2);
	byte device_id = 0x1;
	memcpy(&buffer->device_id,&device_id,1);
	byte data_year = get_year_hex();;
	memcpy(&buffer->data_year,&data_year,1);
	byte data_month = get_month_hex();
	memcpy(&buffer->data_month,&data_month,1);
	byte data_day = get_day_hex();
	memcpy(&buffer->data_day,&data_day,1);
	byte data_hour = get_hour_hex();
	memcpy(&buffer->data_hour,&data_hour,1);
	byte data_min = get_min_hex();
	memcpy(&buffer->data_min,&data_min,1);
	byte data_sec = get_sec_hex();
	memcpy(&buffer->data_sec,&data_sec,1);
	unsigned short data_msec = get_usec_hex();
	memcpy(buffer->data_msec,&data_msec,2);
	byte data_reserved[18] ={0x0};
	memcpy(buffer->data_reserved,data_reserved,18);
	unsigned short crc_checksum =  CRC16_MODBUS(&(buffer->msg_type),(35+N-1-7));
	memcpy(buffer->crc_checksum,&crc_checksum,2);
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->tail,tail,3);
}

void get_message_data(struct keep_alive * buffer,int len)
{
	memset(buffer,0,len);
	byte header[3] = {0xAA,0xAB,0xAC};
	memcpy(buffer->header,header,3);
	byte msg_type = 0x1; 
	memcpy(&buffer->msg_type,&msg_type,1);
	int  id = G_ID;
	memcpy(buffer->id,(byte *)&id,4);
	byte proto_type = 0x02;
	memcpy(&buffer->proto_type,&proto_type,1);
	byte proto_version = 0x10;
	memcpy(&buffer->proto_version,&proto_version,0x10);
	int data_len = 45-2+N;
	memcpy(buffer->data_len,(byte *)&data_len,4);
	byte city_id = 0x01;
	memcpy(&buffer->city_id,&city_id,1);
	byte way_id = 0x0;
	memcpy(&buffer->way_id,&way_id,1);
	byte group_id[2] = {0x0,0x1};
	memcpy(buffer->group_id,group_id,2);
	byte device_id = 0x1;
	memcpy(&buffer->device_id,&device_id,1);
	byte data_year = get_year_hex();;
	memcpy(&buffer->data_year,&data_year,1);
	byte data_month = get_month_hex();
	memcpy(&buffer->data_month,&data_month,1);
	byte data_day = get_day_hex();
	memcpy(&buffer->data_day,&data_day,1);
	byte data_hour = get_hour_hex();
	memcpy(&buffer->data_hour,&data_hour,1);
	byte data_min = get_min_hex();
	memcpy(&buffer->data_min,&data_min,1);
	byte data_sec = get_sec_hex();
	memcpy(&buffer->data_sec,&data_sec,1);
	unsigned short data_msec = get_usec_hex();
	memcpy(buffer->data_msec,&data_msec,2);
	byte data_reserved[18] ={0x0};
	memcpy(buffer->data_reserved,data_reserved,18);
	byte data[N] = {0x0};
	memcpy(buffer->data,data,N);
	unsigned short crc_checksum =  CRC16_MODBUS(&(buffer->msg_type),(35+N-1-7));
	memcpy(buffer->crc_checksum,&crc_checksum,2);
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->tail,tail,3);
}

void get_message_data_confirm(struct keep_alive * buffer,int len)
{
	memset(buffer,0,len);
	byte header[3] = {0xAA,0xAB,0xAC};
	memcpy(buffer->header,header,3);
	byte msg_type = 0x1; 
	memcpy(&buffer->msg_type,&msg_type,1);
	int  id = G_ID;
	memcpy(buffer->id,(byte *)&id,4);
	byte proto_type = 0x02;
	memcpy(&buffer->proto_type,&proto_type,1);
	byte proto_version = 0x10;
	memcpy(&buffer->proto_version,&proto_version,0x10);
	int data_len = 45-2+N;
	memcpy(buffer->data_len,(byte *)&data_len,4);
	byte city_id = 0x01;
	memcpy(&buffer->city_id,&city_id,1);
	byte way_id = 0x0;
	memcpy(&buffer->way_id,&way_id,1);
	byte group_id[2] = {0x0,0x1};
	memcpy(buffer->group_id,group_id,2);
	byte device_id = 0x1;
	memcpy(&buffer->device_id,&device_id,1);
	byte data_year = get_year_hex();;
	memcpy(&buffer->data_year,&data_year,1);
	byte data_month = get_month_hex();
	memcpy(&buffer->data_month,&data_month,1);
	byte data_day = get_day_hex();
	memcpy(&buffer->data_day,&data_day,1);
	byte data_hour = get_hour_hex();
	memcpy(&buffer->data_hour,&data_hour,1);
	byte data_min = get_min_hex();
	memcpy(&buffer->data_min,&data_min,1);
	byte data_sec = get_sec_hex();
	memcpy(&buffer->data_sec,&data_sec,1);
	unsigned short data_msec = get_usec_hex();
	memcpy(buffer->data_msec,&data_msec,2);
	byte data_reserved[18] ={0x0};
	memcpy(buffer->data_reserved,data_reserved,18);
	byte data[N] = {0x0};
	memcpy(buffer->data,data,N);
	unsigned short crc_checksum =  CRC16_MODBUS(&(buffer->msg_type),(35+N-1-7));
	memcpy(buffer->crc_checksum,&crc_checksum,2);
	byte tail[3] = {0xBC,0xBB,0xBA};
	memcpy(buffer->tail,tail,3);
}