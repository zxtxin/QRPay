#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include "ghttp/ghttp.h"
#include "MyCom.h"
#define MAX_LINE 1024

extern HWND hwnd_pic,hwnd_txt;
extern BITMAP pic;

void add_set(fd_set * sockset,int fd1,int fd2){
	FD_ZERO(sockset);
	FD_SET(fd1,sockset);
	FD_SET(fd2,sockset);
}

void * MainService(void)
{
	
	int fdcom,sockfd;
	int status_com;
	char *buf=(char *)malloc(MAX_LINE);
	int size_readfdcom;
	pid_t pid;
	fd_set sockset;
	char *uri;
	ghttp_request *request = ghttp_request_new();
	ghttp_status http_status;

	ghttp_set_uri(request, uri);
	ghttp_prepare(request);

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
		perror("Error: open serial port error.\n");
		exit(1);
	}

	PortSet(fdcom, &portinfo);

	


	
	while(1)
	{
		LoadBitmap (HDC_SCREEN, &pic, "default.png");
		SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);

		size_readfdcom=read(fdcom,buf,MAX_LINE);
		SetWindowText (hwnd_txt, "Dealing...");
		
		ghttp_set_sync(request, ghttp_sync);
		ghttp_set_type(request, ghttp_type_post);
		ghttp_set_header(request, http_hdr_Connection, "keep-alive");
		//set body
		
		
		http_status=ghttp_process(request);
		//get_body
		//if(body==ok)then
		pid=vfork();
		if(!pid)
			execlp("qrencode","qrencode","-s 6","-m 2","-lH","-oqrcode.png",buf,NULL);
		else
			wait(NULL);
		LoadBitmap (HDC_SCREEN, &pic, "qrcode.png");
		SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
		SetWindowText (hwnd_txt, "Amount of money:\n");
		
		ghttp_clean(request);
		ghttp_set_sync(request, ghttp_async);
		ghttp_set_type(request, ghttp_type_post);
		ghttp_set_header(request, http_hdr_Connection, "keep-alive");
		//set body
		
		
		http_status=ghttp_process(request);
		sockfd=ghttp_get_socket(request);

		add_set(&sockset,sockfd,fdcom);
		select( (sockfd >fdcom) ? (sockfd+1) : (fdcom+1),&sockset,NULL,NULL,NULL);
		if(FD_ISSET(fdcom,&sockset)){
			ghttp_clean(request);
			continue;
		}
		if(FD_ISSET(sockfd,&sockset)){
			http_status=ghttp_process(request);
			//get_body
			//if(payment succeed)then
			write(fdcom,buf,MAX_LINE);
			SetWindowText (hwnd_txt, "Payment Succeed.");
			ghttp_clean(request);
		}

	}
	close(fdcom);
	close(sockfd);
	return 0;
}

