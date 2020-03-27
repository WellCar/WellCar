#include "config.h"
#include "common.h"
#include "ser.h"



unsigned int G_ID;

int epoll_fd;
int socket_fd;
int listen_fd_init;
socklen_t clilen ;
struct sockaddr_in clientaddr,serveraddr;
struct epoll_event ev;
struct epoll_event events[MAX_EVENT];

static struct keep_alive * keep_alive_buffer_send;
static struct keep_alive * keep_alive_buffer_recv;
static int keep_alive_buffer_len;
static struct keep_alive_confirm * keep_alive_confirm_buffer_send;
static struct keep_alive_confirm * keep_alive_confirm_buffer_recv;
static int keep_alive_confirm_buffer_len;
static int tcp_init();
static int add_init_epoll();
static void epoll_loop();
static int add_timer_pthread(pthread_t * tid);
static int add_server_pthread(pthread_t * tid);
static void * ser_main(void);

static void recv_msg_handle();

int add_timer_pthread(pthread_t * tid)
{
	int ret = pthread_create(tid,NULL,(void*)timer_main,NULL);
    if (ret)
    {
        printf("[%s %d] Create pthread failed\n",__FUNCTION__, __LINE__);
        return 0;
    }
    return 1;
}

int add_server_pthread(pthread_t * tid)
{
	int ret = pthread_create(tid,NULL,(void*)ser_main,NULL);
    if (ret)
    {
        printf("[%s %d] Create pthread failed\n",__FUNCTION__,__LINE__);
        return 0;
    }
    return 1;
}

int main()
{
	pthread_t timer_tid,ser_tid;
	add_timer_pthread(&timer_tid);
	add_server_pthread(&ser_tid);
    pthread_join(timer_tid,NULL);
    pthread_join(ser_tid,NULL);
}

void * timer_main(void)
{
    while(1)
    {
        sleep_ms(100);

    }
}

void * ser_main(void)
{
	tcp_init();
	add_init_epoll();
	epoll_loop();
}


int set_nonblocking(int p_nsock)
{   
    int nopts;   
    nopts=fcntl(p_nsock,F_GETFL);   
    if(nopts<0)   
    {   
        printf("[%s %d] Fcntl Sock GETFL fail!\n",__FUNCTION__,__LINE__);
        return -1;
    }   

    nopts = nopts|O_NONBLOCK;   
    if(fcntl(p_nsock,F_SETFL,nopts)<0)   
    {  
        printf("[%s %d] Fcntl Sock SETFL fail!\n",__FUNCTION__,__LINE__);
        return -1;   
    } 

    return 0;
} 

int tcp_init()
{

	keep_alive_buffer_send = (struct keep_alive *)malloc(sizeof(struct keep_alive));
	keep_alive_buffer_recv = (struct keep_alive *)malloc(sizeof(struct keep_alive));
	keep_alive_buffer_len = sizeof(struct keep_alive);
    keep_alive_confirm_buffer_send = (struct keep_alive_confirm *)malloc(sizeof(struct keep_alive_confirm));
	keep_alive_confirm_buffer_recv = (struct keep_alive_confirm *)malloc(sizeof(struct keep_alive_confirm));
	keep_alive_confirm_buffer_len = sizeof(struct keep_alive_confirm);

    clilen = sizeof(struct sockaddr_in);
    epoll_fd=epoll_create(MAX_EVENT);
    if (epoll_fd <= 0)
    {
        printf("[%s %d] Epoll Create fail return:%d!\n",__FUNCTION__,__LINE__,epoll_fd);
        return 0;
	}
    listen_fd_init = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_init < 0)
    {
        printf("[%s %d] Socket Create fail return:%d!\n",__FUNCTION__,__LINE__,listen_fd_init);
        return 0;
	}
    if (set_nonblocking(listen_fd_init) < 0)
    {
        return 0;
    }
    memset(&serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;   
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");   
    serveraddr.sin_port=htons(SERV_PORT);   
    if (bind(listen_fd_init,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("[%s %d] Bind fd fail!\n",__FUNCTION__,__LINE__);
        return 0;
    }

    if (listen(listen_fd_init,MAX_LISTENQ) < 0)
    {
        printf("[%s %d] Listen fd fail!\n",__FUNCTION__,__LINE__);
        return 0;
    } 
} 

int add_init_epoll()
{
    ev.data.fd=listen_fd_init;        
    ev.events=EPOLLIN;
    if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd_init,&ev) < 0)
    {
        printf("[%s %d] Epoll ctl error!\n",__FUNCTION__,__LINE__);
        return 0;
    }
} 


void epoll_loop()
{
    int res,i,accept_fd,event_num;
	while(1)
    {   
        // memset(keep_alive_buffer_send,0,keep_alive_buffer_len);
        // memset(keep_alive_buffer_recv,0,keep_alive_buffer_len);
        event_num = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
        for ( i = 0;  i<event_num; i++ )
        {   

            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))  
            {  
                printf("[%s %d] Epoll Event Error!\n",__FUNCTION__,__LINE__);
                close (events[i].data.fd);  
                continue;  
            }
            else if (listen_fd_init == events[i].data.fd)  
            {
                printf("[%d] Listenfd:%d,events[i].data.fd:%d\n", __LINE__,listen_fd_init,events[i].data.fd);
                accept_fd = accept(listen_fd_init, (struct sockaddr*)&clientaddr, &clilen);
                if(accept_fd < 0)
                {
                    printf("[%s %d] Accept fd fail return:%d!\n",__FUNCTION__,__LINE__,accept_fd);
                    continue;
                }
                printf("[%d] Acceptfd:%d,IP:%s,Port:%d\n",__LINE__,accept_fd,inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
                if (set_nonblocking(accept_fd) < 0)
                {
                    continue;
                }

                ev.data.fd = accept_fd;
                ev.events = EPOLLIN | EPOLLET ;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &ev)< 0)
                {
                    printf("[%s %d] Epoll ctl error!\n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }

            if(events[i].events&EPOLLIN)                   
            {   
                if ((socket_fd = events[i].data.fd) < 0)
                {
                    continue;
                }
                memset(keep_alive_buffer_recv, 0x00, keep_alive_buffer_len);
                if ( (res = recv(socket_fd, keep_alive_buffer_recv, keep_alive_buffer_len,0)) < 0)   
                {  
                    if (errno == ECONNRESET)
                    {
                        close(socket_fd);  
                        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,socket_fd,&ev);  
                        events[i].data.fd = -1;
                    }
                    continue;
                } 
                else if (res == 0)   
                {   
                    printf("[%s %d] Recv error, client had closed!\n",__FUNCTION__,__LINE__);
                    close(socket_fd);  
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,socket_fd,&ev);  
                    events[i].data.fd = -1;
                    continue;
                }
				printf("[%s %d] Recv %d bytes Data,Do data deal\n",__FUNCTION__,__LINE__,res);
				/* Condition  */
                recv_msg_handle();
            }
            else if(events[i].events&EPOLLOUT)                 
            {    
                if ((socket_fd = events[i].data.fd) < 0)
                    continue;
                switch (MSG_TYPE)
                {
                    case MSG_KEEP_ALIVE:
                    {
                        printf("[%s %d] Keep Alive Package Pesponse.\n", __FUNCTION__, __LINE__);
                        res = send(socket_fd, keep_alive_confirm_buffer_send, keep_alive_buffer_len, 0);
                        set_epoll_status(epoll_fd,ev,socket_fd,EPOLLIN);
                        break;
                    }
                    case MSG_MESSAGE:
                    {
                        printf("[%s %d] Message Package Pesponse.\n", __FUNCTION__, __LINE__);
                        res = send(socket_fd, keep_alive_buffer_recv, keep_alive_buffer_len, 0);
                        set_epoll_status(epoll_fd,ev,socket_fd,EPOLLIN);
                        break;
                    }
                    default:
                        break;
                }
            }   
        }   
    }  
    close(listen_fd_init);
}

void recv_msg_handle()
{
    byte *  tmp = (byte *)keep_alive_buffer_recv;
    /* This is Keep alive msg */
    if (keep_alive_buffer_recv->msg_type == 0x3)
    {
        printf("[%s %d] Keep Alive Package...\n", __FUNCTION__, __LINE__);
        /* Do Keep Alive Response */
        get_keep_alive_data_confirm(keep_alive_confirm_buffer_send,keep_alive_buffer_recv,keep_alive_confirm_buffer_len);
        set_epoll_status(epoll_fd,ev,socket_fd,EPOLLOUT);
        MSG_TYPE = MSG_KEEP_ALIVE;
    }
    else if (keep_alive_buffer_recv->msg_type == 0x1)
    {
        printf("[%s %d] Message Package...\n", __FUNCTION__, __LINE__);
        /* InterFace */
        get_keep_alive_data(keep_alive_buffer_recv,keep_alive_buffer_len);
        set_epoll_status(epoll_fd,ev,socket_fd,EPOLLOUT);
        MSG_TYPE = MSG_MESSAGE;
    }
    /*
    for(int i=0; i<keep_alive_buffer_len; i++)
    {
        printf("%X ",*tmp++);
    }
    printf("\n");
    */
}

#if 0
printf("[%s %d] EPOLLOUT Sockfd:%d\n", __FUNCTION__, __LINE__, socket_fd);
#endif
