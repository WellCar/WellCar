#include "config.h"
#include "common.h"

int epoll_fd;
int socket_fd;
int listen_fd_init;
socklen_t clilen ;
struct sockaddr_in clientaddr,serveraddr;
struct epoll_event ev;
struct epoll_event events[MAX_EVENT];

static int tcp_init();
static int add_init_epoll();
static void epoll_loop();

int main()
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
    char szRecvBuf[MAX_BUFSIZE];
    memset(szRecvBuf,0x0,MAX_BUFSIZE); 
	while(1)
    {   
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
				printf("[%s %d] EPOLLIN Sockfd:%d\n", __FUNCTION__, __LINE__, socket_fd);
                if ((socket_fd = events[i].data.fd) < 0)
                {
                    continue;
                }
                memset(szRecvBuf, 0x00, sizeof(szRecvBuf));
                if ( (res = recv(socket_fd, szRecvBuf, MAX_BUFSIZE,0)) < 0)   
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
				printf("[%s %d] Recv Data res:%d\n",__FUNCTION__,__LINE__,res);
				/* Condition  */
            }
            else if(events[i].events&EPOLLOUT)                 
            {    
				printf("[%s %d] EPOLLOUT Sockfd:%d\n", __FUNCTION__, __LINE__, socket_fd);
                if ((socket_fd = events[i].data.fd) < 0)
                    continue;
                res = send(socket_fd, szRecvBuf, strlen(szRecvBuf), 0);
            }   
        }   
    }  
    close(listen_fd_init);
}

#if 0
printf("[%s %d] EPOLLOUT Sockfd:%d\n", __FUNCTION__, __LINE__, socket_fd);
#endif
