#include <stdio.h>

int main(){
	FILE * fd;
	int SERV_TCP_PORT;
	char SERV_HOST_ADDR[1000];	
	fd=fopen("config","r");
	fgets(SERV_HOST_ADDR,1000,fd);
	fscanf(fd,"%d",&SERV_TCP_PORT);
	printf("%s\n",SERV_HOST_ADDR);
	printf("%d\n",SERV_TCP_PORT);
	fclose(fd);
}
