/**
 * 客户端程序
 * 1.创建socket
 * 2.建立连接
 * 3.发送数据
 * 4.接受数据
 * 5.关闭连接
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
//#include <syswait.h>
#include <unistd.h>

int cl_socket;
struct sockaddr_in serv_addr;
char recvBuff[1024];
char* kvrecv;

//创建socket
int createSocket() {
	cl_socket = 0;
	int n = 0;
	char recvBuff[1024];
	if ((cl_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Error : Could not create socket \n");
		return 1;
	}
	return cl_socket;
}
//连接服务器
int cli_connect(char* ip, int port) {
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		printf("\n inet_pton error occured\n");
		return 1;
	}
	if (connect(cl_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("\n Error : Connect Failed \n");
		return 1;
	}
	return 0;
}
//发送数据
int cli_send(char* data, int dataLength) {
	int length = write(cl_socket, data, dataLength);
	printf("length %d",dataLength);
	printf("cl_socket %d",cl_socket);
	return length;
}
//接受数据
char* cli_recv() {
	int n = 0;
	printf("\n %d\n",n);

	if((n = read(cl_socket, recvBuff, sizeof(recvBuff) ))>0) {
		if (n > 0) {
			recvBuff[n] = 0;
			if (fputs(recvBuff, stdout) == EOF) {
				printf("\n Error : Fputs error\n");
			}
		}
	}else if(n<0) {
		if (errno == ECONNRESET) {
			printf("\nECONNRESET\n");
			close(cl_socket);
			//			local_events[i].data.fd = -1;
		} else {
			printf("readline error");
		}
	}else{
	}
	printf("\n recv num %d\n",n);

	return recvBuff;
}
//关闭连接
int cli_close() {
	return 0;
}


char** str_split(char* a_str, const char a_delim)
{
	char** result    = 0;
	size_t count     = 0;
	char* tmp        = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx  = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
//			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
//		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

char* kv_get(char* key){
	char* data = "*2\r\n$3\r\nGET\r\n$4\r\nkey1\r\n";
	int len = cli_send(data, strlen(data));
	printf("len %d\n",len);

	kvrecv = cli_recv();
	return kvrecv;
}
char* kv_set(char* key,char* val){
	char* data = "*3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$4\r\nval1\r\n";
	int len = cli_send(data, strlen(data));
	printf("len %d\n",len);

	kvrecv = cli_recv();
	return kvrecv;
}

int main(int argc, char *argv[])
{
//	sleep(10);
	char* ip = "127.0.0.1";
	int port = 9001;
	cl_socket = createSocket();
	printf("cl_socket %d\n",cl_socket);
//	printf("len %d\n",len);

	int n = cli_connect(ip,port);
	char* key = "key1";
	char* val = "val1";

	char* resp;
	int len = (int)strlen("test");
	char* val2 = "test";
    sprintf(resp, "$%d\r\n%s\r\n", len, val2);
    printf("resp %s \n",resp);

//	kvrecv = kv_set(key,val);
	kvrecv = kv_get(key);



	printf("recvBuff %s\n",kvrecv);

	char *p;
	char *d = "\r\n";
	p = strtok(kvrecv,d);

	char *ret[2];
	ret[0] = p;
	int i = 1;
	while(p)
	{
		p=strtok(NULL,d);
		if(sizeof(p)>0){
			printf("\n n %lu \n",sizeof(p));
			printf(" p %s \n",p);

			ret[i] = p;
			i++;
		}
	}
	printf("\n p %s\n",ret[1]);



}