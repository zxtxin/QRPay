#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <json.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include "ghttp/ghttp.h"
#include "MyCom.h"
#define MAX_LINE 1024
#define REQ_REGISTER "register"
#define REQ_PAY "pay"
#define REQ_RESULT "result"
#define REQ_TYPE "req_type"
#define RESP_TYPE "resp_type"
#define MACHINE_ID "m_id"
#define ORDER_ID "o_id"
#define INFO_VERSION "info_version"
#define AMOUNT "amount"
extern HWND hwnd_pic,hwnd_txt;
extern BITMAP pic;

void add_set(fd_set * sockset,int fd1,int fd2){
	FD_ZERO(sockset);
	FD_SET(fd1,sockset);
	FD_SET(fd2,sockset);
}

void *MainService(void)
{
	
	int fdcom,sockfd;
	int status_com;
	char *buf=(char *)malloc(MAX_LINE);
	char *p;
	int size_readfdcom;
	unsigned int machine_id;
	unsigned int order_id;
	unsigned int info_version;
	double amount;
	pid_t pid;
	fd_set sockset;
	struct json_object *my_object=NULL;	
	struct ghttp_request *request = NULL;
	ghttp_status http_status;
	FILE *order;

	request = ghttp_request_new();
	my_object = json_object_from_file("config"); 
	machine_id = json_object_get_int(json_object_object_get(my_object,MACHINE_ID));
	info_version = json_object_get_int(json_object_object_get(my_object,INFO_VERSION));

	ghttp_set_uri(request, json_object_get_string(json_object_object_get(my_object,"host"));
	ghttp_prepare(request);
	ghttp_set_sync(request, ghttp_sync);
	ghttp_set_type(request, ghttp_type_post);
	ghttp_set_header(request, http_hdr_Connection, "keep-alive");
	
	json_object_object_add(my_object, REQ_TYPE, json_object_new_string(REQ_REGISTER));
	
	p=json_object_to_json_string(my_object);
	ghttp_set_body(request, p, strlen(p));
	http_status=ghttp_process(request);

	json_object_put(my_object);

	my_object=json_tokener_parse(ghttp_get_body(request));
	if(!strcmp(json_object_get_string(json_object_object_get(my_object,RESP_TYPE)),REQ_REGISTER)){
		json_object_object_del(my_object, RESP_TYPE);
		json_object_to_file("config",my_object);
	}
	else
		exit(1);

/*	portinfo_t portinfo ={
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
*/
	


	
	while(1)
	{
		json_object_put(my_object);	
		ghttp_clean(request);	

		LoadBitmap (HDC_SCREEN, &pic, "default.png");
		SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
		SetWindowText (hwnd_txt, "Welcome \nto QRPay!");
		
		size_readfdcom=read(fdcom,buf,MAX_LINE);
		SetWindowText (hwnd_txt, "Dealing...");
		
		ghttp_set_sync(request, ghttp_sync);
		ghttp_set_type(request, ghttp_type_post);
		ghttp_set_header(request, http_hdr_Connection, "keep-alive");
		//set body
		my_object = json_object_new_object(); 
		json_object_object_add(my_object, REQ_TYPE, json_object_new_string(REQ_PAY));
		json_object_object_add(my_object, MACHINE_ID, json_object_new_int(machine_id));
		order = fopen("order","r+");
		fscanf(order,"%d",&order_id);
		fseek(order,0,SEEK_SET);
		fprintf(order,"%d",order_id+1);
		fclose(order);
		json_object_object_add(my_object, ORDER_ID, json_object_new_int(order_id));
		json_object_object_add(my_object, AMOUNT, json_object_new_double(amount));
		q=json_object_to_json_string(my_object);
		ghttp_set_body(request, q, strlen(q));

		http_status=ghttp_process(request);
		json_object_put(my_object);
		//get_body
		//if(body==ok)then
/*		my_object=json_tokener_parse(ghttp_get_body(request));
		if(strcmp(json_object_get_string(json_object_object_get(my_object,RESP_TYPE)),REQ_PAY))
			continue;
		json_object_put(my_object);
*/
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
		my_object = json_object_new_object(); 
		json_object_object_add(my_object, REQ_TYPE, json_object_new_string(REQ_RESULT));
		json_object_object_add(my_object, MACHINE_ID, json_object_new_int(machine_id));
		json_object_object_add(my_object, ORDER_ID, json_object_new_int(order_id));
		
		q=json_object_to_json_string(my_object);
		ghttp_set_body(request, q, strlen(q));
		http_status=ghttp_process(request);

		sockfd=ghttp_get_socket(request);
		add_set(&sockset,sockfd,fdcom);
		select( (sockfd >fdcom) ? (sockfd+1) : (fdcom+1),&sockset,NULL,NULL,NULL);
		if(FD_ISSET(fdcom,&sockset)){
						
			continue;
		}
		if(FD_ISSET(sockfd,&sockset)){
			http_status=ghttp_process(request);
			
			//get_body
			//if(payment succeed)then
			my_object=json_tokener_parse(ghttp_get_body(request));
			if(strcmp(json_object_get_string(json_object_object_get(my_object,RESP_TYPE)),REQ_RESULT)||(json_object_get_int(json_object_object_get(my_object,ORDER_ID))!=order_id))
				continue;
			write(fdcom,buf,MAX_LINE);
			SetWindowText (hwnd_txt, "Payment Succeed.");
			
		}

	}
	close(fdcom);
	close(sockfd);
	return 0;
}

