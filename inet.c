#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/time.h>
#include "inet.h"
int init_cli(void)
{
	int sockfd;
	int SERV_TCP_PORT;
	char SERV_HOST_ADDR[MAX_LINE];
	FILE *fd;
	struct sockaddr_in serv_addr;

	fd=fopen("config","r");
	fgets(SERV_HOST_ADDR,MAX_LINE,fd);
	fscanf(fd,"%d",&SERV_TCP_PORT);
	fclose(fd);

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port= htons(SERV_TCP_PORT);
	if((sockfd= socket(AF_INET,SOCK_STREAM,0)) < 0) return 0;
	if ((connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)) return 0;
	return (sockfd);
}
