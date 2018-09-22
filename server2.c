
//TODO
/**
 * 	1. new socked
 * 	2. new listen
 * 	3. bind port
 * 	4. epoll wait
 * 	5. epoll_wait>0 then read
 * 	6. throw to php
 * 	7.
 */

//#include <sys/event.h>
#include "cedis/ty_server.h"
#include "cedis/kvdb.h"
#include <stdlib.h>  // exit
#include <string.h> //memset.h
#include <assert.h>

#define TEST_VERBOSE 1
#define TEST_EXIT exit(1)

#define TDB_SIZE SMALL
#define TDB_KEYSIZE 30
#define TDB_VALSIZE 31

#define ZEROLENVAL_KEY  "Zerolen Value's key%d"
#define ZEROLENKEY_VAL  "zero length key's value"
#define UPD_ZEROLENKEY_VAL  "Updated zerolength key'svalue"
#define ZLVSIZE 5

#define exit_if(r, ...) if(r) {printf(__VA_ARGS__); printf("error no: %d error msg %s\n", errno, strerror(errno)); exit(1);}

//设置非阻塞描述符
int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}


char response[MAXLENGTH];
int resLength;

//tyWorker变量保存worker进程信息
//tyWorker  workers[WORKER_NUM];
//tyWorker worker;
//tyReactor 保存reactor线程信息
//tyReactor  reactors[REACTOR_NUM];


//通过连接fd，获取主进程管道(后续用于获取reactor线程管道)
//int connFd2WorkerId[1000];

//保存本进程信息


//struct epoll_event ev,events[20];//ev用于注册事件,数组用于回传要处理的事件
int i, maxi, listenfd, new_fd, sockfd,epfd,nfds;
const int kReadEvent = 1;
const int kWriteEvent = 2;

//int setOutPut(char * data,int fd,int length){
//	printf("setOutPut fd %d \n",fd);
//	printf("epfd fd %d \n",epfd);
//
//	resLength =length;
//
//	memcpy(response, data, resLength);
//	//strcpy(response, data);	//data 中包含 \0(可能) 不能使用strcpy
//	ev.data.fd=fd;//设置用于写操作的文件描述符
//	ev.events=EPOLLOUT|EPOLLET;//设置用于注测的写操作事件
//	epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);//修改sockfd上要处理的事件为EPOLLOUT
//}
//
//int epollCreate(){
//	int tmpEpFd;
//    //生成用于处理accept的epoll专用的文件描述符
//	tmpEpFd=epoll_create(512);
//    return tmpEpFd;
//}
//
//int epollAdd(int epollfd,int readfd, int fdtype){
//	struct epoll_event e;
//	setnonblocking(readfd);
//	//生成用于处理accept的epoll专用的文件描述符
////	epfd=epoll_create(256);
//	//设置与要处理的事件相关的文件描述符
//	e.data.fd=readfd;
//	//设置要处理的事件类型
//	e.events=fdtype;
////	e.events=SW_FD_PIPE | SW_EVENT_READ;
//	//注册epoll事件
//	epoll_ctl(epollfd,EPOLL_CTL_ADD,readfd,&e);
//
//	return SW_OK;
//}
//
//
//int epollEventSet(int efd, int fd, int events) {
//    struct epoll_event ev;
//    memset(&ev, 0, sizeof(ev));
//    ev.events = events;
//    ev.data.fd = fd;
//
//    int r = epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
//
//    return SW_OK;
//}

void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    exit_if(flags < 0, "fcntl failed");
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    exit_if(r < 0, "fcntl failed");
}


void updateEvents(int efd, int fd, int events, bool modify) {
	struct kevent ev[2];
	int n = 0;
	if (events & kReadEvent) {
		EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void *) (intptr_t) fd);
	} else if (modify) {
		EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void *) (intptr_t) fd);
	}
	if (events & kWriteEvent) {
		EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void *) (intptr_t) fd);
	} else if (modify) {
		EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void *) (intptr_t) fd);
	}
	printf("%s fd %d events read %d write %d\n", modify ? "mod" : "add", fd, events & kReadEvent, events & kWriteEvent);
	int r = kevent(efd, ev, n, NULL, 0, NULL);
	exit_if(r, "kevent failed ");
}

//mac下的端口监听
int createListenOnMac(int epfd, char* ip,int port)
{

//	ssize_t n;
//	socklen_t clilen;
	//struct epoll_event ev,events[20];//ev用于注册事件,数组用于回传要处理的事件
	struct sockaddr_in  serveraddr; //clientaddr,
	//生成socket文件描述符
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//把socket设置为非阻塞方式
	setnonblocking(listenfd);
	//生成用于处理accept的epoll专用的文件描述符
//	epfd = epollCreate();
	//添加监听事件，监听端口
//	int fdtype =kReadEvent|kWriteEvent;
//	kqueueAdd(epfd,listenfd,fdtype);
	updateEvents(epfd, listenfd, kReadEvent, false);



	//设置服务器端地址信息
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//监听的地址
	char *local_addr= ip;
	inet_aton(local_addr,&(serveraddr.sin_addr));
	serveraddr.sin_port=htons(port);
	//绑定socket连接
	bind(listenfd, ( struct sockaddr* )&serveraddr, sizeof(serveraddr));
	//监听
	listen(listenfd, LISTENQ);
	maxi = 0;
	printf("listenfd %d epfd %d \n",listenfd,epfd);

	return listenfd;

}
//
////服务启动
//int mainReactorRun(char* ip,int port)
//{
//	printf("mainReactorRun ip %s port %d \n",ip,port);
//
//    ssize_t n;
//    char line[MAXLENGTH];
//    socklen_t clilen;
//    //struct epoll_event ev,events[20];//ev用于注册事件,数组用于回传要处理的事件
//    struct sockaddr_in clientaddr, serveraddr;
//    //生成socket文件描述符
//    listenfd = socket(AF_INET, SOCK_STREAM, 0);
//    //把socket设置为非阻塞方式
//    setnonblocking(listenfd);
//    //生成用于处理accept的epoll专用的文件描述符
//    epfd = epollCreate();
//    //添加监听事件，监听端口
//    int fdtype =EPOLLIN|EPOLLET;
//    epollAdd(epfd,listenfd,fdtype);
//
//    //设置服务器端地址信息
//    bzero(&serveraddr, sizeof(serveraddr));
//    serveraddr.sin_family = AF_INET;
//    //监听的地址
//    char *local_addr= ip;
//    inet_aton(local_addr,&(serveraddr.sin_addr));
//    serveraddr.sin_port=htons(port);
//    //绑定socket连接
//    bind(listenfd, ( struct sockaddr* )&serveraddr, sizeof(serveraddr));
//    //监听
//    listen(listenfd, LISTENQ);
//    maxi = 0;
//    printf("listenfd %d epfd %d \n",listenfd,epfd);
//
//    return listenfd;
//}
int recieveData(sockfd){
	char line[MAXLENGTH];
	ssize_t n;

	if ( (n = recv(sockfd, line, MAXLENGTH, 0)) < 0){
		if (errno == ECONNRESET)
		{
			close(sockfd);
//			local_events[i].data.fd = -1;
		}else{
			printf("readline error");
		}
	}else if (n == 0){
		printf("read error \n");
		close(sockfd);
//		local_events[i].data.fd = -1;
	}
	printf("line1 %c \n",line[20]);
	printf("line2 %zu n %zu \n",sizeof(line),n);
	//TODO 解析上报的数据

	//TODO 操作kv数据
    return 1;
}

char* g_name = NULL;
char *data[6];
char *resp;
kvdb_s* test_create_database(char* name){
	if (name == NULL){
		g_name = "./kv001.db";
	}else{
		g_name = name;
	}
	kvdb_s* kdb = create_kvdb(g_name, TDB_SIZE,
							  TDB_KEYSIZE, TDB_VALSIZE);
	assert(kdb != NULL);
	//printf("TEST: Succesfully created db\n");
//	if(TEST_VERBOSE) printf("Test step: %d completed\n", ++g_teststep);
	return kdb;
}


kvdb_s* test_load_database(char* name){
    if (name == NULL){
        g_name = "./kv001.db";
    }else{
        g_name = name;
    }
    kvdb_s* kdb = load_kvdb(g_name);
    assert(kdb != NULL);
    //printf("TEST: Succesfully loaded db\n");
//    if(TEST_VERBOSE) printf("Test step: %d completed\n", ++g_teststep);
    return kdb;
}


void handleAccept(int efd, int fd) {
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cfd = accept(fd, (struct sockaddr *) &raddr, &rsz);
    exit_if(cfd < 0, "accept failed");
//    struct sockaddr_in peer, local;
//    socklen_t alen = sizeof(peer);
//    int r = getpeername(cfd, (sockaddr *) &peer, &alen);
//    exit_if(r < 0, "getpeername failed");
    printf("accept a connection from %s\n", inet_ntoa(raddr.sin_addr));
    setNonBlock(cfd);
    updateEvents(efd, cfd, kReadEvent | kWriteEvent, false);
}
//
//
//char * strtolower(char * old)
//{
//	char xx[1000];
//	int ii, length=0;
//	length=strlen(old);
//	for(ii=0; ii<length; ii++)
//	{
//		xx[ii]=tolower(old[ii]);
//	}
//	xx[ii]='\0';
//	return xx;
//
//}
//
//
//char * strtoupper(char * old)
//{
//	char xx[1000];
//	int ii, length=0;
//	length=strlen(old);
//	for(ii=0; ii<length; ii++)
//	{
//		xx[ii]=toupper(old[ii]);
//	}
//	xx[ii]='\0';
//	return xx;
//
//}

/**
 * 字符串分割
 * @param buf
 * @return
 */
char** split(char* buf,char *d){
	char *p;

	p = strtok(buf,d);


	data[0] = p;
	int i = 1;
	while(p)
	{
		p=strtok(NULL,d);
		if(p!= NULL){
			printf("\n n %lu i %d \n",sizeof(p),i);
			printf(" p %s \n",p);

			data[i] = p;
			i++;
		}
	}
	return data;
}
char* parseCmd(char* buf, kvdb_s* kdb){
	char *d = "\r\n";
//	printf("read2 %s \n", buf);

	char **ret = split(buf,d);
//	int paramNum = ret[0];
	char *cmd = ret[2];
	printf("\n cmd %s \n",cmd);
	printf("\n cmd n %lu \n", sizeof(cmd));
//	cmd = strtolower(cmd);
//	printf("\n cmd2 %s \n",cmd);

	//TODO 兼容linux与mac strcasecmp stricmp

	if (strcasecmp("get", cmd) == 0){
		printf("\n get \n");

		char *key = ret[4];
		printf("\n get key %s\n",key);

		char* val = get_kvdb(kdb, key);
        printf("\n get val %s\n",val);

        if(val!=NULL){
            int len = (int)strlen(val);
            printf("\n get val2 %s len %d \n",val,len);
            char resp2[100];
            sprintf(resp2, "$%d\r\n%s\r\n", len, val);
            printf("\n get resp %s\n",resp);
            resp = resp2;

        }else{
            resp = "$4\r\nfalse\r\n";
        }


        return resp;

	}
	if (strcasecmp("set", cmd) == 0){
		printf("\n set \n");

		char *key = ret[4];
		char *val = ret[6];
		add_kvdb(kdb, key, val); //int ret =
		return "+OK";
	}
	if (strcasecmp("hget", cmd) == 0){
		printf("\n hget \n");
	}
	if (strcasecmp("hset", cmd) == 0){
		printf("\n hset \n");
	}
	return "+Error";
}

void handleWrite(int efd, int fd) {
    //实际应用应当实现可写时写出数据，无数据可写才关闭可写事件
    updateEvents(efd, fd, kReadEvent, true);
}

void handleRead(int efd, int fd, kvdb_s* kdb) {
    char buf[4096];
    int n = 0;
    if ((n = read(fd, buf, sizeof buf)) > 0) {
		buf[4095] = 0;
        printf("read %d bytes\n", n);
        printf("read %s \n", buf);
		//解析请求，执行相应命令
		char* reps = parseCmd(buf,kdb);
		int r = write(fd, reps, n);  //写出读取的数据
        printf("write r %d \n", r);
        //实际应用中，写出数据可能会返回EAGAIN，此时应当监听可写事件，当可写时再把数据写出
        exit_if(r <= 0, "write error");
    }
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;
//    exit_if(n < 0, "read error");  //实际应用中，n<0应当检查各类错误，如EINTR
    printf("fd %d closed\n", fd);
    close(fd);
}

void loop_once(int efd, int lfd, int waitms, kvdb_s* kdb) {
	struct timespec timeout;
	timeout.tv_sec = waitms / 1000;
	timeout.tv_nsec = (waitms % 1000) * 1000 * 1000;
	const int kMaxEvents = 20;
	struct kevent activeEvs[kMaxEvents];
	int n = kevent(efd, NULL, 0, activeEvs, kMaxEvents, &timeout);
	printf("kqueue return %d\n", n);
	for (int i = 0; i < n; i++) {
		int fd = (int) (intptr_t) activeEvs[i].udata;
		int events = activeEvs[i].filter;
		if (events == EVFILT_READ) {
			if (fd == lfd) {
				handleAccept(efd, fd);
			} else {
				handleRead(efd, fd, kdb);
			}
		} else if (events == EVFILT_WRITE) {
            printf("EVFILT_WRITE \n");

//            handleWrite(efd, fd);
		} else {
			exit_if(1, "unknown event");
		}
	}
}

//int runServer(char* ip,int port){
int main(int argc, char *argv[])
{
//	int ret;
    char* ip = "0.0.0.0";
    int port = 9001;
//	ret = socketpair(AF_UNIX, SOCK_DGRAM, 0, masterSocks);


	char *kvname = "./kv002.db";
//	kvdb_s* kdb = test_create_database(kvname);
	kvdb_s* kdb = test_load_database(kvname);

	int pid =getpid();
//	int forkPid ;
//	int mainEpollFd;
	printf("pid  %d\n",pid);

	//创建kqueue，与epoll类似
	int epfd = kqueue();
//	mainEpollFd = mainReactorRun(ip, port);
	listenfd = createListenOnMac(epfd,ip, port);

	//主进程开始循环监听
//	mainReactorWait(mainEpollFd);
	while(1) {  //实际应用应当注册信号处理函数，退出时清理资源
		loop_once(epfd, listenfd, 10000, kdb);
	}

}

//
//int mainReactorWait(int mainEpollFd){
//	int readfd,writefd;
//	int nfds;
//	socklen_t clilen;
//	struct sockaddr_in clientaddr;
//	while(1)
//	    {
//			/* epoll_wait：等待epoll事件的发生，并将发生的sokct fd和事件类型放入到events数组中；
//			* nfds：为发生的事件的个数。可用描述符数量
//			*/
//	        nfds=epoll_wait(epfd,events,20,500);
//	        //处理可用描述符的事件
//	        for(i=0;i<nfds;++i)
//	        {
//	        	//当监听端口描述符可用时，接收链接的时候
//	            if(events[i].data.fd==mainEpollFd)
//	            {
//	            	/* 获取发生事件端口信息，存于clientaddr中；
//	                *new_fd：返回的新的socket描述符，用它来对该事件进行recv/send操作*/
//	                new_fd = accept(mainEpollFd,(struct sockaddr *)&clientaddr, &clilen);
//	                printf("new_fd %d \n",new_fd);
//	                if(new_fd<0)
//				   {
//	                    perror("new_fd<0\n");
//	                    return 1;
//	                }
//	                perror("setnonblocking\n");
//	                setnonblocking(new_fd);
//	                char *str = inet_ntoa(clientaddr.sin_addr);
//
//	                //给reactor线程添加监听事件，监听本次连接
//	                int reactor_id = new_fd%REACTOR_NUM; //连接fd对REACTOR_NUM取余，决定抛给哪个reactor线程
//	                int reactor_epfd = reactors[reactor_id].epfd;
//	                printf("reactor_epfd %d new_fd %d \n",reactor_epfd,new_fd);
//				   int fdtype =EPOLLIN|EPOLLET;
////				   epollAdd(reactor_epfd,new_fd,fdtype);
//					recieveData(new_fd)
//	            }
//	        }
//	    }
//}
//
//
