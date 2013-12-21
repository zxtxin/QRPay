#include "mainservice.h"


void add_set(fd_set * sockset,int fd1,int fd2){
	FD_ZERO(sockset);
	FD_SET(fd1,sockset);
	FD_SET(fd2,sockset);
}

void * MainService(void)
{
	int sockfd;
	int fdcom;
	int status_com,status_sock;
	char buf[MAX_LINE]={};
	char strOnDisplay[100]={};
	pid_t pid;
	fd_set sockset;
	if(!(sockfd=init_cli())) {
		printf("Error: socket initialization error.\n");
		exit(1);
	}

	portinfo_t portinfo ={
		'0',                          	// print prompt after receiving
 		115200,                      	// baudrate: 9600
 		'8',                          	// databit: 8
 		'0',                          	// debug: off
 		'0',                          	// echo: off
 		'2',                          	// flow control: software
 		'0',                          	// default tty: COM1
 		'0',                          	// parity: none
 		'1',                          	// stopbit: 1
 		 0    	                  	// reserved
	};

	fdcom = PortOpen(&portinfo);
	
	if(fdcom<0){
		printf("Error: open serial port error.\n");
		exit(1);
	}

	PortSet(fdcom, &portinfo);
	add_set(&sockset,sockfd,fdcom);

	LoadBitmap (HDC_SCREEN, &pic, "default.png");
	while(1)
	{
		select( (sockfd >fdcom) ? (sockfd+1) : (fdcom+1),&sockset,NULL,NULL,NULL);
		if(FD_ISSET(fdcom,&sockset)){
			status_com=read(fdcom,buf,MAX_LINE);
			UnloadBitmap (&pic);
			pid=vfork();
			if(!pid) execl("/qrencode","qrencode","-s 5","-m 2","-o qr_pic.png",buf,NULL);
			else wait(NULL);
			LoadBitmap (HDC_SCREEN, &pic, "qr_pic.png");
			SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
			SetWindowText (hwnd_txt, strOnDisplay);
			write(sockfd,buf,MAX_LINE);

		}
		if(FD_ISSET(sockfd,&sockset)){
			status_sock=read(sockfd,buf,MAX_LINE);
			write(fdcom,buf,MAX_LINE);
			LoadBitmap (HDC_SCREEN, &pic, "default.png");
			SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
			
			SetWindowText (hwnd_txt, strOnDisplay);
		}
	}
	close(fdcom);
	close(sockfd);
	return 0;
}
