/*
This client for 4g wireless test */
#include "common.h"
#include "config.h"

#define RECV_BUFFER_LEN (1024*1)

int epoll_fd;
int socket_fd;
struct epoll_event ev;
struct epoll_event events[MAX_EVENT];
unsigned int G_ID;

static struct keep_alive * keep_alive_buffer_send;
static struct keep_alive * keep_alive_buffer_recv;
static int keep_alive_buffer_len;
static struct keep_alive_confirm * keep_alive_confirm_buffer_send;
static struct keep_alive_confirm * keep_alive_confirm_buffer_recv;
static int keep_alive_confirm_buffer_len;
static void epoll_loop();
static int do_send(int fd);
static int do_recv(int fd);
static void * timer_msg(void);
static void * timer_keep_alive(void);
static void * link_main(void);

int main(void)
{
	/* init */
	keep_alive_buffer_send = (struct keep_alive *)malloc(sizeof(struct keep_alive));
	keep_alive_buffer_recv = (struct keep_alive *)malloc(sizeof(struct keep_alive));
	keep_alive_buffer_len = sizeof(struct keep_alive);
    keep_alive_confirm_buffer_send = (struct keep_alive_confirm *)malloc(sizeof(struct keep_alive_confirm));
	keep_alive_confirm_buffer_recv = (struct keep_alive_confirm *)malloc(sizeof(struct keep_alive_confirm));
	keep_alive_confirm_buffer_len = sizeof(struct keep_alive_confirm);

	int ret=0;
	pthread_t tcp_tid,timer_tid,timer_message_tid;
	ret = pthread_create(&tcp_tid,NULL,(void *)link_main,NULL);
	if(ret){
		printf("Create error\n");exit(0);
	}
	ret = pthread_create(&timer_tid,NULL,(void *)timer_keep_alive,NULL);
	if(ret){
		printf("Create error\n");exit(0);
	}
	ret = pthread_create(&timer_message_tid,NULL,(void *)timer_msg,NULL);
	if(ret){
		printf("Create error\n");exit(0);
	}
	pthread_join(timer_tid,NULL);
	pthread_join(tcp_tid,NULL);
	pthread_join(timer_message_tid,NULL);
	return 0;
}

int set_nonblocking(int * p_nsock)
{
	int nopts;
	
	nopts =  fcntl(*p_nsock, F_GETFL);
	if (nopts < 0)
	{
		printf("[%s %d] Fcntl Sock GETFL fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    nopts = nopts | O_NONBLOCK;
    if (fcntl(*p_nsock, F_SETFL, nopts)<0)
    {
        printf("[%s %d] Fcntl Sock SETFL fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

void * timer_msg(void)
{
	while(1)
	{	
		sleep_ms(MSG_SEND_FREQ);
		set_epoll_status(epoll_fd,ev,socket_fd,EPOLLOUT);
		FSM = FSM_MESSAGE;
	}
}


void * timer_keep_alive(void)
{
	while(1)
	{
		sleep_ms(KEEP_ALIVE_FREQ);
		set_epoll_status(epoll_fd,ev,socket_fd,EPOLLOUT);
		FSM = FSM_KEEP_ALIVE;
	}
}

void connect_to_ser(struct sockaddr_in *addr_serv, int *socket_fd)
{
    addr_serv->sin_family = AF_INET;
    addr_serv->sin_port =  htons(DEST_PORT);
    addr_serv->sin_addr.s_addr = inet_addr(DEST_IP_ADDRESS);
	/* Set No Blocking */
    set_nonblocking(socket_fd);

    if( connect(*socket_fd,(struct sockaddr *)addr_serv,sizeof(struct sockaddr)) < 0)
    {
        printf("[%s %d] Client connect errno:%d!\n",__FUNCTION__,__LINE__,errno);
        if (errno != EINPROGRESS) 
        {
            printf("[%s %d] Connnect Remote Server fail.\n",__FUNCTION__, __LINE__);
        } 
    } 
}

void * link_main(void)
{
    struct sockaddr_in addr_serv;
	int socket_fd_init;
	socket_fd_init = socket(AF_INET, SOCK_STREAM,0);

	if (socket_fd_init <0)
	{
		printf("[%s %d] Socket Create fail return:%d!\n", __FUNCTION__, __LINE__, socket_fd_init);
	}

    memset(&addr_serv,0,sizeof(addr_serv));
	connect_to_ser(&addr_serv,&socket_fd_init);
    epoll_fd=epoll_create(MAX_EVENT);

    if (epoll_fd <= 0)
    {
        printf("[%s %d] Epoll create fail return:%d!\n",__FUNCTION__,__LINE__,epoll_fd);
    }

    ev.data.fd = socket_fd_init;        
    ev.events = EPOLLET|EPOLLOUT;     

    if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd_init,&ev) < 0)
    {
        printf("[%s %d] Epoll ctl error!\n",__FUNCTION__,__LINE__);
    }

	epoll_loop();
}

void epoll_loop()
{
	FSM = FSM_KEEP_ALIVE;
    int event_num,send_num,recv_num,i;
	while(1)
    {
        event_num = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
        for (i = 0; i<event_num; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))  
            {  
                close (events[i].data.fd);  
                continue;  
            }
            if (events[i].events & EPOLLOUT) 
            {
				if ((socket_fd = events[i].data.fd) < 0)
                    continue;
				/* Do Send Logic */
				do_send(socket_fd);
				/* There is need some codition
				set_epoll_status(epoll_fd,ev,socket_fd,EPOLLIN);
				*/
            }
            else if (events[i].events&EPOLLIN)
            {
                if ((socket_fd = events[i].data.fd) < 0)
                    continue;
				/* There is need some codition
				set_epoll_status(epoll_fd,ev,socket_fd,EPOLLOUT);
				*/
				do_recv(socket_fd);
            }
        }
    }
    close(socket_fd);
}

int do_send(int fd)
{
	int res =0;
	switch(FSM)
	{
		case FSM_KEEP_ALIVE:
		{
			printf("[%s %d] Send Keep Alive Packge...\n",__FUNCTION__,__LINE__);
			get_keep_alive_data(keep_alive_buffer_send,keep_alive_buffer_len);
			res = send(fd,keep_alive_buffer_send,keep_alive_buffer_len,0);
			set_epoll_status(epoll_fd,ev,fd,EPOLLIN);
			FSM = FSM_IDLE;
			break;
		}
		case FSM_MESSAGE:
		{
			printf("[%s %d] Send [ID=%d]Message Packge...\n",__FUNCTION__,__LINE__,G_ID);
			get_message_data(keep_alive_buffer_send,keep_alive_buffer_len);
			res = send(fd,keep_alive_buffer_send,keep_alive_buffer_len,0);
			set_epoll_status(epoll_fd,ev,fd,EPOLLIN);
			FSM = FSM_IDLE;
			G_ID++;
			break;
		}
		case FSM_IDLE:
		{
			/* Do nothing */
			break;
		}
		default:
		{
			/* Do nothing */
			break;
		}

	}
	return res;
}

int do_recv(int fd)
{
	int res = 0;
	int id;
	res = recv(socket_fd,keep_alive_confirm_buffer_recv,keep_alive_confirm_buffer_len,0);
	if (keep_alive_confirm_buffer_recv->msg_type == 0x1)
	{
		printf("[%s %d] KeepAlice Response.\n",__FUNCTION__,__LINE__);
	}else if(keep_alive_confirm_buffer_recv->msg_type == 0x3)
	{
		/* code */
		memcpy(&id,keep_alive_confirm_buffer_recv->id,4);
		printf("[%s %d] Message[%d] Response.\n",__FUNCTION__,__LINE__,id);
		/* Reconnect Setting */
	}
	
}

#if 0
printf("[%s %d] Debug \n",__FUNCTION__,__LINE__);
#endif
