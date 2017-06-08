#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>  

#include "resource.h"
#include "winform.h"
#include "position.h"
//#include "link\toolsCV.h"

#pragma comment(lib,"comctl32.lib")  
#pragma comment(lib, "WINMM.LIB")
#pragma comment( lib, "msimg32.lib" )

const LPCSTR cszAbout = "橘趣象棋（图形连线器） Ver 0530\n\n潇湘棋士";// 版本号
const int WINDOW_STYLES = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;// 窗口和绘图属性

extern Position pos;		// 局面实例
WINFORMSTRUCT wforms;

void change_piece_size(char *path, char* dst,int size);//批量变换兵河棋子图像大小
//LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;


// 窗体事件捕捉过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

	static struct CButton
	{
		int     iStyle ;
		TCHAR * szText ;
	}button[] = {
		BS_PUSHBUTTON,      TEXT ("连接测试"),
		BS_PUSHBUTTON,      TEXT ("取消连接"),
		BS_DEFPUSHBUTTON,   TEXT ("换棋盘"),
		BS_DEFPUSHBUTTON,   TEXT ("换棋子"),
		BS_AUTOCHECKBOX,    TEXT ("开局库"),
		BS_AUTOCHECKBOX,    TEXT ("云库"),
		//BS_AUTO3STATE,      TEXT ("引擎"),
		BS_AUTORADIOBUTTON, TEXT ("红先"),
		BS_AUTORADIOBUTTON, TEXT ("黑先"),
		//BS_CHECKBOX,        TEXT ("CHECKBOX"), 
		//BS_RADIOBUTTON,     TEXT ("RADIOBUTTON"),
		//BS_3STATE,          TEXT ("3STATE"),
		//BS_GROUPBOX,        TEXT ("GROUPBOX"),
		//BS_OWNERDRAW,       TEXT ("OWNERDRAW")
	} ;
	static int NUMOFBUTTON =8;   //此数目应与上面实际相对，否则出错
	static HWND hwndButton[10]; 

	static int cxChar, cyChar;
	static int  cxClient, cyClient ;
	static HPEN hpen, holbpen;

	static HFONT hFont;			//逻辑字体
	static HWND hLabUsername;	//静态文本框--用户名
	static HWND hLabPassword;	 //静态文本框--密码
	static HWND hEditUsername;  //单行文本输入框
	static HWND hEditPassword;  //密码输入框
	static HWND hBtnLogin;		//登录按钮
	static char buffer[255]="this is string!";
	static RECT rect;

    TCHAR   ButtonName[]=TEXT("按钮");  
    static  HWND  hWndStatus,hWndTool;  
    TCHAR   szBuf[MAX_PATH];  
    int nCount = 2;  
    int array[2]={150,-1};  
    static TBBUTTON tbb[3];  
    static TBADDBITMAP tbab;  
    //LPNMHDR lpnmhdr;  
    //LPTOOLTIPTEXT lpttext;  

	//定义缓冲区
//	TCHAR szUsername[100];
//	TCHAR szPassword[100];
//	TCHAR szUserInfo[200];
	HDC hdc;
	PAINTSTRUCT ps;
	int x, y;

	switch (uMsg) {
		// 新建窗口
	case WM_CREATE:
		//statusbar
		hWndStatus=CreateWindowEx(0,STATUSCLASSNAME,"",WS_CHILD|WS_BORDER|WS_VISIBLE,-100,-100,10,10,hWnd,(HMENU)100,wforms.hInst,NULL);  
        if(!hWndStatus)  
            MessageBox(hWnd,TEXT("can't create statusbar!"),TEXT("error_notify"),MB_OK);  
        SendMessage(hWndStatus,SB_SETPARTS,(WPARAM)nCount,(LPARAM)array);  
        SendMessage(hWndStatus,SB_SETTEXT,(LPARAM)1,(WPARAM)TEXT("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1"));  
        SendMessage(hWndStatus,SB_SETTEXT,(LPARAM)2,(WPARAM)TEXT("info message bestmove"));  
		
		//toolbar
        hWndTool=CreateWindowEx(0,TOOLBARCLASSNAME,NULL,WS_CHILD|WS_VISIBLE|TBSTYLE_TOOLTIPS,0,0,0,0,hWnd,(HMENU)1111, wforms.hInst,NULL);  
        SendMessage(hWndTool,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);  

		tbab.hInst=HINST_COMMCTRL;  
        tbab.nID=IDB_STD_SMALL_COLOR;  
        if(SendMessage(hWndTool,TB_ADDBITMAP,1,(LPARAM)&tbab)==-1)  
            MessageBox(hWnd,TEXT("失败"),TEXT("创建"),0);  
        ZeroMemory(tbb,sizeof(tbb));  
        tbb[0].iBitmap=STD_FILENEW;  
        tbb[0].fsState=TBSTATE_ENABLED;  
        tbb[0].fsStyle=TBSTYLE_BUTTON;  
        tbb[0].idCommand=ID_FILE_NEW;  
        tbb[0].iString=(INT_PTR)"new";
        tbb[1].iBitmap=STD_FILEOPEN;  
        tbb[1].fsState=TBSTATE_ENABLED;  
        tbb[1].fsStyle=TBSTYLE_BUTTON;  
        tbb[1].idCommand=ID_FILE_OPEN;  
        tbb[1].iString=(INT_PTR)"open";  
		tbb[2].iBitmap=STD_FILESAVE;  
        tbb[2].fsState=TBSTATE_ENABLED;  
        tbb[2].fsStyle=TBSTYLE_BUTTON;  
        tbb[2].idCommand=ID_FILE_SAVEAS;  
        tbb[2].iString=(INT_PTR)"save";  
        SendMessage(hWndTool,TB_ADDBUTTONS,sizeof(tbb)/sizeof(TBBUTTON),(LPARAM)&tbb);  
        SendMessage(hWndTool,TB_SETBUTTONSIZE,0,(LPARAM)MAKELONG(35,35));  
        SendMessage(hWndTool,TB_AUTOSIZE,0,0);  

		cxChar = LOWORD (GetDialogBaseUnits ()) ;
		cyChar = HIWORD (GetDialogBaseUnits ()) ;

    	//生成按钮等子控件
		for (int i = 0 ; i < NUMOFBUTTON ; i++)
			hwndButton[i] = CreateWindow ( TEXT("button"), button[i].szText, WS_CHILD | WS_VISIBLE | button[i].iStyle, 
				cxChar + BOARD_WIDTH + BOARD_EDGE, cyChar * (1 + 2 * i) + LBrook_TOP, 15 * cxChar, 7*cyChar/4, 
				hWnd, (HMENU) i, ((LPCREATESTRUCT) lParam)->hInstance, NULL) ;

		//创建静态文本框控件
		hLabUsername = CreateWindow(TEXT("static"), TEXT("窗口标题名："), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ |SS_RIGHT /*水平居右*/, cxChar+BOARD_WIDTH+ BOARD_EDGE /*x坐标*/, cyChar * (1 + 2 * 11) /*y坐标*/, 100/*宽度*/, 26 /*高度*/, hWnd /*父窗口句柄*/, (HMENU)1 /*控件ID*/, wforms.hInst /*当前程序实例句柄*/, NULL	);

		//创建单行文本框控件
		hEditUsername = CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER /*边框*/ | ES_AUTOHSCROLL /*水平滚动*/, cxChar+BOARD_WIDTH+ BOARD_EDGE /*x坐标*/, cyChar * (1 + 2 * 12) /*y坐标*/, 100, 26,	hWnd, (HMENU)3,  wforms.hInst, NULL	);


		// 调整窗口位置和尺寸
		GetWindowRect(hWnd, &rect);
		x = rect.left;
		y = rect.top;
		rect.right = rect.left + BOARD_WIDTH + BOARD_EDGE + 135;
		rect.bottom = rect.top + TOOLBAR_SIZE + BOARD_HEIGHT + BOARD_EDGE*5;
		AdjustWindowRect(&rect, WINDOW_STYLES, TRUE);
		MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		break;

	case WM_SIZE:
        {  
            HWND hTool;  
			hTool=GetDlgItem(wforms.hWnd,IDC_MAIN_TOOL);  
            SendMessage(hTool,TB_AUTOSIZE,0,0);  
            MoveWindow(hWndStatus,0,0,0,0,TRUE);  
        }  

		//cxChar = LOWORD(lParam);
		//cyChar = HIWORD(lParam);
		cxClient = LOWORD (lParam) ;
		cyClient = HIWORD (lParam) ;
		return 0 ;

		// 退出
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// 菜单命令
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case ID_FILE_NEW:  
			MessageBox(wforms.hWnd,TEXT("notify"),TEXT("file new"),0);  
			break;  
		case ID_FILE_OPEN:  
			MessageBox(wforms.hWnd,TEXT("notify"),TEXT("file open"),0);  
			break;  
		case ID_FILE_SAVEAS:  
			MessageBox(wforms.hWnd,TEXT("notify"),TEXT("file save as"),0);  
			break;  

		//case 5:
		//	//获取输入框的数据
		//	GetWindowText(hEditUsername, szUsername, 100);
		//	GetWindowText(hEditPassword, szPassword, 100);
		//	wsprintf(szUserInfo, TEXT("您的用户账号：%s\r\n您的用户密码：%s"), szUsername, szPassword);
		//	MessageBox(hWnd, szUserInfo, TEXT("信息提示"), MB_ICONINFORMATION);
		//	break;
		case BS_PUSHBUTTON:
			strcpy_s(buffer, "BS_PUSHBUTTON");
			rect.left   = 0 ;
			rect.top    = 0 ;
			rect.right  = BOARD_WIDTH ;
			rect.bottom = 100;
			InvalidateRect (wforms.hWnd, &rect, TRUE) ;
			break;
		case BS_AUTO3STATE:
			strcpy_s(buffer, "BS_AUTO3STATE");
			rect.left   = 0 ;
			rect.top    = 0 ;
			rect.right  = BOARD_WIDTH ;
			rect.bottom = 100;
			InvalidateRect (wforms.hWnd, &rect, TRUE) ;
			break;
		case BS_RADIOBUTTON:
			strcpy_s(buffer, "BS_RADIOBUTTON");
			rect.left   = 0 ;
			rect.top    = 0 ;
			rect.right  = BOARD_WIDTH ;
			rect.bottom = 100;
			InvalidateRect (wforms.hWnd, &rect, TRUE) ;
			break;
		case IDM_FILE_RED:
		case IDM_FILE_BLACK:
			wforms.bFlipped = (LOWORD(wParam) == IDM_FILE_BLACK);
			Startup();
			hdc = GetDC(wforms.hWnd);
			DrawBoard(hdc);
			if (wforms.bFlipped) {
				wforms.hdc = hdc;
				wforms.hdcTmp = CreateCompatibleDC(wforms.hdc);
				ResponseMove();
				DeleteDC(wforms.hdcTmp);
			}
			ReleaseDC(wforms.hWnd, hdc);
			break;
		case IDM_FILE_EXIT:
			DestroyWindow(wforms.hWnd);
			break;
		case IDM_HELP_ABOUT:

			ShellAbout(hWnd,"橘中新趣","橘中新趣",wforms.hIcon);    

			/*	MessageBeep(MB_ICONINFORMATION);
			   mbp.cbSize = sizeof(MSGBOXPARAMS);
			   mbp.hwndOwner = hWnd;
			   mbp.hInstance = wforms.hInst;
			   mbp.lpszText = cszAbout;
			   mbp.lpszCaption = "橘中新趣";
			   mbp.dwStyle = MB_USERICON;
			   mbp.lpszIcon = MAKEINTRESOURCE(IDI_APPICON);
			   mbp.dwContextHelpId = 0;
			   mbp.lpfnMsgBoxCallback = NULL;
			   mbp.dwLanguageId = 0;
			   MessageBoxIndirect(&mbp);*/
			break;
		}
		break;
		// 绘图
	case WM_PAINT:
		hdc = BeginPaint(wforms.hWnd, &ps);
		
		DrawBoard(hdc);
		//DrawText (hdc, TEXT ("Hello, Windows 98!"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER) ;

		//以下新加/////////////////////////////////////
		//hbitmap = LoadImage(NULL,"..\\res\\rp.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);  
		//Hdcmem = CreateCompatibleDC(hdc);  
		//hbmp= LoadResBmp(IDB_RP);
		//DrawTransBmp(wforms.hdc, hdcmem, 33, 33,hbmp);
		//DrawTransBmp(wforms.hdc, hdcmem, 0, 0, (HBITMAP)hbitmap);

		EndPaint(wforms.hWnd, &ps);
		break;
		// 鼠标点击
	case WM_LBUTTONDOWN:
		x = FILE_LEFT + (LOWORD(lParam) - (BOARD_EDGE + LBrook_LEFT - SQUARE_SIZE/2 )) / SQUARE_SIZE;
		y = RANK_TOP + (HIWORD(lParam) - (BOARD_EDGE  + LBrook_TOP + TOOLBAR_SIZE - SQUARE_SIZE/2)) / SQUARE_SIZE;
		if (x >= FILE_LEFT && x <= FILE_RIGHT && y >= RANK_TOP && y <= RANK_BOTTOM) 
		{
			ClickSquare(COORD_XY(x, y));
			SetCapture(wforms.hWnd);
			SetCursor(wforms.hCursor);
		}
		break;

	case    WM_MOUSEMOVE:  
        wsprintf(szBuf,"Mouse %d,%d",LOWORD(lParam),HIWORD(lParam));  
        SendMessage(hWndStatus,SB_SETTEXT,0,(LPARAM)(LPSTR)szBuf);  
        break;  
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
		// 其他事件
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
#ifdef _DEBUG
	//change_piece_size("E:\\private\\兵河五四3.6\\兵河棋盘棋子\\棋子大全-素还真提供\\3D精致棋子(金边)", "C:\\deleted\\link_vc2010\\RES\\",PIECE_SIZE );
	//change_piece_size("E:\\兵河五四360\\兵河棋盘棋子\\棋子大全-素还真提供\\3D精致棋子(金边)", "G:\\link_vc2010_office\\RES\\",PIECE_SIZE);		//home
#endif
	int i;
	MSG msg;
	WNDCLASSEX wce;

	// 初始化全局变量
	srand((DWORD) time(NULL));
	//InitZobrist();
	wforms.hInst = hInstance;
	LoadBook(hInstance);
	wforms.bFlipped = FALSE;
	Startup();

	// 装入图片
	wforms.bmpBoard = LoadResBmp(IDB_BOARD);
	wforms.bmpSelected = LoadResBmp(IDB_SELECTED);
	for (i = PIECE_KING; i <= PIECE_PAWN; i ++) {
		wforms.bmpPieces[SIDE_TAG(0) + i] = LoadResBmp(IDB_RK + i);
		wforms.bmpPieces[SIDE_TAG(1) + i] = LoadResBmp(IDB_BK + i);
	}

	// 设置窗口
	wce.cbSize = sizeof(WNDCLASSEX);
	wce.style = 0;
	wce.lpfnWndProc = (WNDPROC) WndProc;
	wce.cbClsExtra = wce.cbWndExtra = 0;
	wce.hInstance = hInstance;
	wce.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 32, 32, LR_SHARED);
	wce.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	wce.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wce.lpszMenuName = MAKEINTRESOURCE(IDM_MAINMENU);
	wce.lpszClassName = "JQXQ";
	wce.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);
	RegisterClassEx(&wce);

	// 打开窗口
	wforms.hIcon = wce.hIcon;
	//wforms.hCursor=  SetCursor(LoadCursor(wforms.hInst, MAKEINTRESOURCE(IDC_CURSOR_HAND)));
	//wforms.hCursor = LoadCursorFromFile("hand.cur");
	wforms.hCursor = LoadResCur(IDC_CURSOR_HAND);

	wforms.hWnd = CreateWindow("JQXQ", "橘趣象棋", WINDOW_STYLES,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (wforms.hWnd == NULL) {
		return 0;
	}
	ShowWindow(wforms.hWnd, nCmdShow);
	UpdateWindow(wforms.hWnd);

	// 接收消息
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}
	return msg.wParam;
}
