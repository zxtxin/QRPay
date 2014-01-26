#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "MyCom.h"
//#include "inet.h"
//#include "mainservice.h"
/*
#ifdef _LANG_ZHCN
#include "static_res_cn.h"
#elif defined _LANG_ZHTW
#include "static_res_tw.h"
#else
#include "static_res_en.h"
#endif
*/
extern void * MainService(void);
HWND hwnd_pic,hwnd_txt;
BITMAP pic;

static int QRPayWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    switch (message) {
        case MSG_CREATE:
            hwnd_pic = CreateWindow (CTRL_STATIC,"",WS_CHILD | SS_BITMAP | WS_VISIBLE ,IDC_STATIC, 240, 16, 240, 240, hWnd, 0);
            hwnd_txt = CreateWindow (CTRL_STATIC,"Welcome \nto QRPay!",WS_CHILD | SS_NOTIFY | SS_CENTER | SS_WHITERECT | WS_VISIBLE,IDC_STATIC+1,0, 85, 240, 240, hWnd, 0);
	        SetWindowFont(hwnd_txt,CreateLogFont (NULL, "Arial", "ISO8859-1",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,25, 0));
            break;

        case MSG_DESTROY:
            DestroyAllControls (hWnd);
            break;

        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            break;
	
	default:
	    return DefaultMainWinProc(hWnd, message, wParam, lParam);
    }
	return 0;
    
}
/*
void *MainService(void)
{
	int i,j;
	scanf("%d",&i);
	if(i==10){
	printf("i==10,true\n");
	LoadBitmap (HDC_SCREEN, &pic, "9.gif");
	SendMessage(hwnd_pic,STM_SETIMAGE,(WPARAM)&pic,(LPARAM)0);
	SetWindowText (hwnd_txt, "Total:\n");
	}
	else printf("WRONG");
	
}
*/
int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
   
    pthread_t new_thread;
    int ret;
    ret=CreateThreadForMainWindow(&new_thread, NULL, MainService, 0);
  
/*
#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "static" , 0 , 0);
#endif
*/
    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Welcome to QRPay!";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = QRPayWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 480;
    CreateInfo.by = 272;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

