#ifndef TOOLSYS_H_INCLUDE
#define TOOLSYS_H_INCLUDE

extern const int MASK_COLOR;
extern const int SQUARE_SIZE;
extern const int PIECE_SIZE;
extern const int LBrook_LEFT;		//棋盘左上角黑车位置X（相对棋盘图像），兵河55
extern const int LBrook_TOP;		//棋盘左上角黑车位置Y（相对棋盘图像），兵河70

extern const int BOARD_EDGE;
extern const int BOARD_WIDTH;
extern const int BOARD_HEIGHT;
extern const bool DRAW_SELECTED;
extern const int ScreenOffX;
extern const int ScreenOffY;
		
// 与图形界面有关的全局变量
struct WINFORMSTRUCT{
	HINSTANCE hInst;                              // 应用程序句柄实例
	HWND hWnd;                                    // 主窗口句柄
	HDC hdc, hdcTmp;                              // 设备句柄，只在"ClickSquare"过程中有效
	HBITMAP bmpBoard, bmpSelected, bmpPieces[24]; // 资源图片句柄
	int sqSelected, mvLast;                       // 选中的格子，上一步棋
	BOOL bFlipped, bGameOver;                     // 是否翻转棋盘，是否游戏结束(不让继续玩下去)
	HICON hIcon;
	HCURSOR hCursor;
};			
extern WINFORMSTRUCT wforms;

void MessageBoxMute(LPCSTR lpszText);

void DrawBoard(HDC hdc);
void DrawTransBmp(HDC hdc, HDC hdcTmp, int xx, int yy, HBITMAP bmp);
void DrawSquare(int sq, BOOL bSelected = FALSE);//绘制格子
void ResponseMove(void);
void ClickSquare(int sq);
void Startup(void);

// 播放资源声音
inline void PlayResWav(int nResId) {
	PlaySound(MAKEINTRESOURCE(nResId), wforms.hInst, SND_ASYNC | SND_NOWAIT | SND_RESOURCE);
}

// 装入资源图片
inline HBITMAP LoadResBmp(int nResId) {
	return (HBITMAP) LoadImage(wforms.hInst, MAKEINTRESOURCE(nResId), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

inline HCURSOR LoadResCur(int nResId) {
	return (HCURSOR) LoadImage(wforms.hInst, MAKEINTRESOURCE(nResId), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

#endif