#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

#include "MyCom.h"
#include "inet.h"
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

/*	fdcom = PortOpen(&portinfo);
	
	if(fdcom<0){
		printf("Error: open serial port error.\n");
		exit(1);
	}

	PortSet(fdcom, &portinfo);
*/
	fdcom=0;


	LoadBitmap (HDC_SCREEN, &pic, "default.png");
	SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
	while(1)
	{
		add_set(&sockset,sockfd,fdcom);
		select( (sockfd >fdcom) ? (sockfd+1) : (fdcom+1),&sockset,NULL,NULL,NULL);
		if(FD_ISSET(fdcom,&sockset)){
			status_com=read(fdcom,buf,MAX_LINE);

			pid=vfork();
			if(!pid)
				execlp("qrencode","qrencode","-s 5","-m 2","-oqrcode.png",buf,NULL);
			else
				wait(NULL);

			LoadBitmap (HDC_SCREEN, &pic, "qrcode.png");
			SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);


			SetWindowText (hwnd_txt, buf);

			write(sockfd,buf,MAX_LINE);

		}
		if(FD_ISSET(sockfd,&sockset)){
			status_sock=read(sockfd,buf,MAX_LINE);
			buf[status_sock]=0;
			write(fdcom,buf,MAX_LINE);
			printf("\n");
			LoadBitmap (HDC_SCREEN, &pic, "default.png");
			SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);

			SetWindowText (hwnd_txt, buf);

		}

	}
	close(fdcom);
	close(sockfd);
	return 0;
}
