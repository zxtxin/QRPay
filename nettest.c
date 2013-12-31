#include "inet.h"
int main()
{
	int sockfd,i;
	
	int status_read,status_write;
	char buf[10]={};
	
	if(!(sockfd=init_cli())) {
		printf("Error: socket initialization error.\n");
		exit(1);
	}
	else	{
		printf("Socket success.\n");
	}

/*	scanf("%s",buf);
	buf[9]='\n';
	status_write=write(sockfd,buf,1000);
	printf("%d,%s\n",status_write,buf);
*/	
	

	status_read=read(sockfd,buf,10);
	printf("%d\n",status_read);
//	for(i=0;i==9;i++)
		printf("%s\n",buf);
	
	status_read=read(sockfd,buf,10);
	printf("%d\n",status_read);
	printf("%s\n",buf);

}

