#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <json.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <libusb.h>
#include <ev.h>
#include "ghttp/ghttp.h"


#define REQ_REGISTER 1
#define REQ_SUBMIT 2
#define REQ_QUERY 3
#define REQ_CANCEL 4
#define REQ_TYPE "req_type"
#define BODY "body"
#define RESP_TYPE "resp_type"
#define RESP_RESULT "resp_result"
#define RESULT_SUCCESS 0
#define RESULT_FAILURE -1
#define RESULT_TIMEOUT -2
#define MACHINE_ID "m_id"
#define ORDER_ID "o_id"
#define INFO_VERSION "info_version"
#define ENCODE_FORMAT "encode_format"
#define HOST "host"
#define AMOUNT "amount"
#define USB_BUF_SIZE 64
#define USB_VENDORID 0x10c4
#define USB_PRODUCTID 0x0000

extern HWND hwnd_pic,hwnd_txt;
extern BITMAP pic;
enum STATUS
{
	INITIALIZE,
	READY,
	SUBMIT,
	WAIT,
	CANCEL
}status;
typedef struct {
	int type;
	int result;
}RESP_RET;
static RESP_RET resp_ret;
static struct ghttp_request *request;
static struct json_object *my_object;
static unsigned int machine_id;
static unsigned int *p_order_id;
static unsigned int info_version;
static char *amount;
static char usb_in_buf[USB_BUF_SIZE];
static char usb_out_buf[USB_BUF_SIZE];
static ev_signal usb_watcher;
static ev_io net_watcher;
static int fd_net,net_io;
static int transferred;
static pthread_t main_thread;
static libusb_device_handle *handle;
static char qrcode_buf[80];
static ghttp_status http_status;
static char* format_qrcode;
void update_display(const char* img,int type)//undone
{
	char txt[50];
	if(type==1)
	{
		sprintf(txt,"The amount is : %s\n",amount);
	}
	else
	{
		sprintf(txt,"Wait for order.\n");
	}
	LoadBitmap (HDC_SCREEN, &pic, img);
	SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
	SetWindowText (hwnd_txt, txt);
}
static void write_to_usb(int resp_type,int resp_result)
{
	int err;
	usb_out_buf[0]=resp_type;
	usb_out_buf[1]=resp_result;
	err=libusb_interrupt_transfer(handle,0x02,usb_out_buf,USB_BUF_SIZE,&transferred,0);
	if(err)
		puts(libusb_error_name(err));
	
}
static void *read_from_usb()
{
	int err;
	while(1)
	{
		if(err=libusb_interrupt_transfer(handle,0x81,usb_in_buf,USB_BUF_SIZE,&transferred,0))
        {
        	puts(libusb_error_name(err));
        }
		if(err=pthread_kill(main_thread,SIGUSR1))
		{
			perror("Pthread_kill ERROR");
		}
	}

}
static void http_request(int req_type)
{	
	char *str;
	struct json_object *json,*body; 
	json = json_object_new_object();
	json_object_object_add(json, REQ_TYPE, json_object_new_int(req_type));
	if(req_type==REQ_REGISTER)
	{
		body=my_object;
	}
	else
	{
		body=json_object_new_object();
		if(req_type==REQ_SUBMIT)
		{
			int ms_async;
			(*p_order_id)++;
			ms_async=msync((void*)p_order_id,(size_t)sizeof(unsigned int),MS_ASYNC);
			if(ms_async==-1)
				perror("MSYNC_ERROR");
			json_object_object_add(body, AMOUNT, json_object_new_string(amount));
		}
		json_object_object_add(body, MACHINE_ID, json_object_new_int(machine_id));
		json_object_object_add(body, ORDER_ID, json_object_new_int(*p_order_id));
	}

	json_object_object_add(json, BODY, body);
	
	str=json_object_to_json_string(json);
	ghttp_set_body(request, str, strlen(str));
	http_status=ghttp_process(request);
	fd_net=ghttp_get_socket(request);
//	json_object_put(body);
	json_object_put(json);
	
}


static int http_response_process(void * arg)
{
	struct json_object *resp; 
	if(ghttp_process(request)==ghttp_done)
	{
		resp=json_tokener_parse(ghttp_get_body(request));
		resp_ret.type=json_object_get_int(json_object_object_get(resp,RESP_TYPE));
		resp_ret.result=json_object_get_int(json_object_object_get(resp,RESP_RESULT));
		if(resp_ret.type==REQ_REGISTER)
		{
			json_object_object_del(resp, RESP_TYPE);
			json_object_to_file((char *)arg,resp);
		}
		else
		{
			if((resp_ret.type==REQ_SUBMIT)&&(!resp_ret.result))
			{
				sprintf(qrcode_buf,format_qrcode,machine_id,*p_order_id,info_version);
				pid_t pid=vfork();
				if(!pid)
					execlp("qrencode","qrencode","-s 6","-m 2","-lH","-oqrcode.png",qrcode_buf,0);
				else
					wait(NULL);
			}
		}
		json_object_put(resp);
		return 0;
	}
	else
		return -1;

}


static void usb_cb (EV_P_ ev_io *w, int revents)
{
	switch(usb_in_buf[0])
	{
		case 'N': 
		{
			if(status==READY)
			{
				http_request(REQ_SUBMIT);
				net_io=1;
				status=SUBMIT;
				update_display("qrcode.png",1);
			}
			break;
		}
		case 'C': 
		{
			if(status==WAIT)
			{
				http_request(REQ_CANCEL);
				net_io=1;
				status=CANCEL;
			}
			else
			{
				status=READY;
				update_display("default.png",0);
			}
			break;
		}
	}
	memset(usb_in_buf,0,USB_BUF_SIZE);
	ev_break (EV_A_ EVBREAK_ALL);
}
static void net_cb (EV_P_ ev_io *w, int revents)
{
	if(http_response_process(NULL))
	{
		net_io=0;
		if((resp_ret.type==REQ_SUBMIT)&&(!resp_ret.result))
		{
			http_request(REQ_QUERY);
			net_io=1;
			status=WAIT;
		}
		else
		{
			status=READY;
			update_display("default.png",0);
		}
		write_to_usb(resp_ret.type,resp_ret.type);
	}

	ghttp_flush_response_buffer(request);
	ev_break (EV_A_ EVBREAK_ALL);
}
static int init(const char *config_file,const char *orderid_file)
{	
	
	int fd_order;
	request = ghttp_request_new();
	my_object = json_object_from_file(config_file); 
	machine_id = json_object_get_int(json_object_object_get(my_object,MACHINE_ID));
	info_version = json_object_get_int(json_object_object_get(my_object,INFO_VERSION));
	format_qrcode = json_object_get_string(json_object_object_get(my_object,ENCODE_FORMAT));
	fd_order=open(orderid_file,O_RDWR);
	if(fd_order==-1)
	{
		int order_id=0;
		fd_order=open(orderid_file,O_RDWR|O_CREAT,0666);
		write(fd_order,(void*)&order_id,sizeof(unsigned int));
		lseek(fd_order,0,SEEK_SET);
	}
	
	p_order_id=(unsigned int*)mmap(NULL,sizeof(unsigned int),PROT_READ|PROT_WRITE,MAP_SHARED,fd_order,0);
	if(p_order_id==-1)
		return -3;
	
	ghttp_set_uri(request, json_object_get_string(json_object_object_get(my_object,HOST)));
	ghttp_prepare(request);
	ghttp_set_sync(request, ghttp_async);
	ghttp_set_type(request, ghttp_type_post);
	ghttp_set_header(request, http_hdr_Connection, "keep-alive");

	http_request(REQ_REGISTER);
	while(http_response_process((void*)config_file)==-1);
	ghttp_flush_response_buffer(request);
	return 0;
}
void *MainService(void * arg)
{
	
	status=INITIALIZE;
	if(init("config","orderid")){
		puts("Initialization error.");
		exit(1);
	}
	else
		status=READY;
	update_display("default.png",0);

	
	libusb_init(NULL);
	handle=libusb_open_device_with_vid_pid(NULL,USB_VENDORID,USB_PRODUCTID);
	if(!handle)
	{	
		printf("OPEN DEVICE ERROR.\n");
		getchar();
		exit(1);
	}
	libusb_claim_interface(handle,0);
	pthread_t readusb_thread;
	main_thread=pthread_self();
	struct ev_loop *loop = EV_DEFAULT;
	ev_init (&net_watcher, net_cb);
	ev_signal_init(&usb_watcher,usb_cb,SIGUSR1);
	ev_signal_start(loop,&usb_watcher);
	if(pthread_create(&readusb_thread,NULL,read_from_usb,NULL))
	{
		printf("CREATE THREAD ERROR.\n");
		getchar();
		exit(1);
	}
	
	while(1)
	{
		if(!ev_is_active(EV_A_ &net_watcher))
		{
			ev_io_set (&net_watcher, fd_net, EV_READ);
			ev_io_start(loop,&net_watcher);
		}
		ev_run (loop, 0);
		if(net_io)
			ev_io_stop (EV_A_ &net_watcher);
	}
	munmap((void *)p_order_id,sizeof(unsigned int));
	ghttp_close(request);
	ghttp_request_destroy(request);
	return 0;
}
