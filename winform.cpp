#include<Windows.h>

#include "position.h"
#include "winform.h"
#include "resource.h"

const int MASK_COLOR = RGB(0, 0, 0);
const int BOARD_EDGE = 8;		//
const int TOOLBAR_SIZE = 41;		

const int SQUARE_SIZE = 56;		//棋格大小，兵河56
const int PIECE_SIZE  = 52;		//棋子大小，兵河52

const int LBrook_LEFT = 55;		//棋盘左上角黑车位置X（相对棋盘图像），兵河55
const int LBrook_TOP  = 70;		//棋盘左上角黑车位置Y（相对棋盘图像），兵河70

const int BOARD_WIDTH  = 560;
const int BOARD_HEIGHT = 645;
const int ScreenOffX =   BOARD_EDGE + LBrook_LEFT ;	//左边空白
const int ScreenOffY =   BOARD_EDGE + LBrook_TOP + TOOLBAR_SIZE;	//上边空白

// "DrawSquare"参数
const bool DRAW_SELECTED = true;

static void DrawLine(HDC hdc,int x1,int y1,int x2,int y2);//画线
static void DrawBoard1(HDC hdc);//画棋盘

// 绘制格子
void DrawSquare(int sq, BOOL bSelected)
{
	int sqFlipped, xx, yy, pc;
	sqFlipped = wforms.bFlipped ? SQUARE_FLIP(sq) : sq;
	xx = LBrook_LEFT - SQUARE_SIZE/2+ (FILE_X(sqFlipped) - FILE_LEFT) * SQUARE_SIZE ;		//此时的X是相对是棋盘位图左上角的
	yy = LBrook_TOP  - SQUARE_SIZE/2+ (RANK_Y(sqFlipped) - RANK_TOP) * SQUARE_SIZE  ;
	SelectObject(wforms.hdcTmp, wforms.bmpBoard);   //位图选进兼容的设备内容时，左上角坐标是（0，0）注意转换！
	//BitBlt(hdc, BOARD_EDGE, BOARD_EDGE + TOOLBAR_SIZE, BOARD_WIDTH, BOARD_HEIGHT, hdcTmp, 0, 0, SRCCOPY);   //绘制棋盘时的数据
	BitBlt(wforms.hdc, BOARD_EDGE+xx, BOARD_EDGE+TOOLBAR_SIZE+yy, SQUARE_SIZE, SQUARE_SIZE, wforms.hdcTmp, xx, yy, SRCCOPY);			//恢复原棋盘该处图像
	pc = pos.ucpcSquares[sq];

	xx += BOARD_EDGE ;
	yy += BOARD_EDGE + TOOLBAR_SIZE ;
	if (pc != 0) 
	{
		DrawTransBmp(wforms.hdc, wforms.hdcTmp,xx, yy, wforms.bmpPieces[pc]);
	}
	if (bSelected)
	{
		DrawTransBmp(wforms.hdc, wforms.hdcTmp,  xx, yy,  wforms.bmpSelected);
	}
}

// 点击格子事件处理
void ClickSquare(int sq) {
	int pc, mv, vlRep;
	wforms.hdc = GetDC(wforms.hWnd);
	wforms.hdcTmp = CreateCompatibleDC(wforms.hdc);
	sq = wforms.bFlipped ? SQUARE_FLIP(sq) : sq;
	pc = pos.ucpcSquares[sq];

	if ((pc & SIDE_TAG(pos.sdPlayer)) != 0)
	{
		// 如果点击自己的子，那么直接选中该子
		if (wforms.sqSelected != 0) 
		{
			DrawSquare(wforms.sqSelected);
		}
		wforms.sqSelected = sq;
		DrawSquare(sq, DRAW_SELECTED);
		if (wforms.mvLast != 0)
		{
			DrawSquare(SRC(wforms.mvLast));
			DrawSquare(DST(wforms.mvLast));
		}
		PlayResWav(IDR_CLICK); // 播放点击的声音
	} 
	else if (wforms.sqSelected != 0 && !wforms.bGameOver)
	{
		// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
		mv = MOVE(wforms.sqSelected, sq);
		if (pos.LegalMove(mv)) 
		{
			if (pos.MakeMove(mv)) 
			{
				wforms.mvLast = mv;
				DrawSquare(wforms.sqSelected, DRAW_SELECTED);
				DrawSquare(sq, DRAW_SELECTED);
				wforms.sqSelected = 0;
				// 检查重复局面
				vlRep = pos.RepStatus(3);
				if (pos.IsMate()) {
					// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
					PlayResWav(IDR_WIN);
					MessageBoxMute("祝贺你取得胜利！");
					wforms.bGameOver = TRUE;
				} 
				else if (vlRep > 0) 
				{
					vlRep = pos.RepValue(vlRep);
					// 注意："vlRep"是对电脑来说的分值
					PlayResWav(vlRep > WIN_VALUE ? IDR_LOSS : vlRep < -WIN_VALUE ? IDR_WIN : IDR_DRAW);
					MessageBoxMute(vlRep > WIN_VALUE ? "长打作负，请不要气馁！" :
						vlRep < -WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
					wforms.bGameOver = TRUE;
				} 
				else if (pos.nMoveNum > 100) 
				{
					PlayResWav(IDR_DRAW);
					MessageBoxMute("超过自然限着作和，辛苦了！");
					wforms.bGameOver = TRUE;
				} 
				else 
				{
					// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
					PlayResWav(pos.InCheck() ? IDR_CHECK : pos.Captured() ? IDR_CAPTURE : IDR_MOVE);
					if (pos.Captured()) 
					{
						pos.SetIrrev();
					}
					ResponseMove(); // 轮到电脑走棋
				}
			}
			else 
			{
				PlayResWav(IDR_ILLEGAL); // 播放被将军的声音
			}
		}
		// 如果根本就不符合走法(例如马不走日字)，那么程序不予理会
	}
	DeleteDC(wforms.hdcTmp);
	ReleaseDC(wforms.hWnd, wforms.hdc);
}


// 绘制透明图片
//TransparentBlt的倒数第2，3个参数必须小于图片实际大小，这个行为和BitBlt不同，需要格外注意。#pragma comment( lib, "msimg32.lib" )。TransparentBlt函数需要加载这个类库。
void DrawTransBmp(HDC hdc, HDC hdcTmp, int xx, int yy, HBITMAP bmp) {
	SelectObject(hdcTmp, bmp);
	TransparentBlt(hdc, xx, yy, PIECE_SIZE, PIECE_SIZE, hdcTmp, 0, 0, PIECE_SIZE, PIECE_SIZE, MASK_COLOR);
}

void DrawLine(HDC hdc,int x1,int y1,int x2,int y2)//画线
{
	MoveToEx (hdc, x1, y1, NULL) ;
	LineTo (hdc,x2, y2) ;
}

// 绘制棋盘
void DrawBoard(HDC hdc) {
	int x, y, xx, yy, sq, pc;
	HDC hdcTmp;

	// 画棋盘
	hdcTmp = CreateCompatibleDC(hdc);
	SelectObject(hdcTmp, wforms.bmpBoard);
	BitBlt(hdc, BOARD_EDGE, BOARD_EDGE + TOOLBAR_SIZE, BOARD_WIDTH, BOARD_HEIGHT, hdcTmp, 0, 0, SRCCOPY);
	DrawBoard1(hdc);

	// 画棋子
	for (x = FILE_LEFT; x <= FILE_RIGHT; x ++) {
		for (y = RANK_TOP; y <= RANK_BOTTOM; y ++) {
			if (wforms.bFlipped) 
			{
				xx = BOARD_EDGE + LBrook_LEFT -	SQUARE_SIZE/2 +	(FILE_FLIP(x) - FILE_LEFT) * SQUARE_SIZE;
				yy = BOARD_EDGE + LBrook_TOP + TOOLBAR_SIZE - SQUARE_SIZE/2 + (RANK_FLIP(y) - RANK_TOP) * SQUARE_SIZE;
			} 
			else 
			{
				xx = BOARD_EDGE + LBrook_LEFT- SQUARE_SIZE/2 +  (x - FILE_LEFT) * SQUARE_SIZE;
				yy = BOARD_EDGE  + LBrook_TOP + TOOLBAR_SIZE - SQUARE_SIZE/2 + (y - RANK_TOP) * SQUARE_SIZE;
			}

			sq = COORD_XY(x, y);
			pc = pos.ucpcSquares[sq];

			if (pc != 0) 
			{
				DrawTransBmp(hdc, hdcTmp, xx, yy, wforms.bmpPieces[pc]);
			}
			if (sq == wforms.sqSelected || sq == SRC(wforms.mvLast) || sq == DST(wforms.mvLast)) 
			{
				DrawTransBmp(hdc, hdcTmp, xx, yy, wforms.bmpSelected);
			}
		}
	}
	DeleteDC(hdcTmp);
}

void DrawBoard1(HDC hdc)//画棋盘
{
	char buffer[]="楚       河              汉      界";
	char numberB[9][3]={"1","2","3","4","5","6","7","8","9"};
	char numberR[9][3]={"一","二","三","四","五","六","七","八","九"};
	int i;
	
	//画横线
	for( i=0;i<=9;i++)
	{
		DrawLine(hdc,ScreenOffX, ScreenOffY+SQUARE_SIZE*i, ScreenOffX+SQUARE_SIZE*8, ScreenOffY+SQUARE_SIZE*i);
	}

	TextOut(hdc, ScreenOffX*3, int(ScreenOffY+SQUARE_SIZE*4.3), buffer, sizeof(buffer)-1) ;//楚河汉界

	//画竖线
	for( i=0;i<=8;i++)
	{
		TextOut(hdc, ScreenOffX+SQUARE_SIZE*i, ScreenOffY+SQUARE_SIZE*0-SQUARE_SIZE, numberB[i], sizeof(numberB[i])-2) ;//黑方标记
		TextOut(hdc, ScreenOffX+SQUARE_SIZE*i, int(ScreenOffY+SQUARE_SIZE*9+SQUARE_SIZE*0.7), numberR[i], sizeof(numberR[i])-1) ;//红方标记

		if(i==0||i==8)
		{
			DrawLine(hdc,ScreenOffX+SQUARE_SIZE*i,ScreenOffY,ScreenOffX+SQUARE_SIZE*i,ScreenOffY+SQUARE_SIZE*9);
		}
		else
		{
			DrawLine(hdc,ScreenOffX+SQUARE_SIZE*i,ScreenOffY,ScreenOffX+SQUARE_SIZE*i,ScreenOffY+SQUARE_SIZE*4);
			DrawLine(hdc,ScreenOffX+SQUARE_SIZE*i,ScreenOffY+SQUARE_SIZE*5,ScreenOffX+SQUARE_SIZE*i,ScreenOffY+SQUARE_SIZE*9);
		}
	}

	////画九宫格
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*2);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*7,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*9);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*2,ScreenOffX+SQUARE_SIZE*5,ScreenOffY);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*9,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*7);

	////画炮位
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*2);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*7,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*9);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*2,ScreenOffX+SQUARE_SIZE*5,ScreenOffY);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*9,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*7);
	//
	////画兵位
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*2);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*7,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*9);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*2,ScreenOffX+SQUARE_SIZE*5,ScreenOffY);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*9,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*7);

	////画外边粗的方框
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*2);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*7,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*9);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*2,ScreenOffX+SQUARE_SIZE*5,ScreenOffY);
	//DrawLine(hdc,ScreenOffX+SQUARE_SIZE*3,ScreenOffY+SQUARE_SIZE*9,ScreenOffX+SQUARE_SIZE*5,ScreenOffY+SQUARE_SIZE*7);
}


// 弹出不带声音的提示框
void MessageBoxMute(LPCSTR lpszText) {
	MSGBOXPARAMS mbp;
	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = wforms.hWnd;
	mbp.hInstance = NULL;
	mbp.lpszText = lpszText;
	mbp.lpszCaption = "橘中新趣";
	mbp.dwStyle = MB_USERICON;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_INFORMATION);
	mbp.dwContextHelpId = 0;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = 0;
	if (MessageBoxIndirect(&mbp) == 0) {
		// 系统图标在 Windows 98 下会失败，所以要使用应用程序图标
		mbp.hInstance = wforms.hInst;
		mbp.lpszIcon = MAKEINTRESOURCE(IDI_APPICON);
		MessageBoxIndirect(&mbp);
	}
}

// 电脑回应一步棋
void ResponseMove(void) {
	int vlRep;
	// 电脑走一步棋
	SetCursor((HCURSOR) LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	int mv= SearchMain();
	SetCursor((HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	pos.MakeMove(mv);
	// 清除上一步棋的选择标记
	DrawSquare(SRC(wforms.mvLast));
	DrawSquare(DST(wforms.mvLast));
	// 把电脑走的棋标记出来
	wforms.mvLast = mv;//Search.mvResult;
	DrawSquare(SRC(wforms.mvLast), DRAW_SELECTED);
	DrawSquare(DST(wforms.mvLast), DRAW_SELECTED);
	// 检查重复局面
	vlRep = pos.RepStatus(3);
	if (pos.IsMate())
	{
		// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
		PlayResWav(IDR_LOSS);
		MessageBoxMute("请再接再厉！");
		wforms.bGameOver = TRUE;
	} 
	else if (vlRep > 0) 
	{
		vlRep = pos.RepValue(vlRep);
		// 注意："vlRep"是对玩家来说的分值
		PlayResWav(vlRep < -WIN_VALUE ? IDR_LOSS : vlRep > WIN_VALUE ? IDR_WIN : IDR_DRAW);
		MessageBoxMute(vlRep < -WIN_VALUE ? "长打作负，请不要气馁！" :
			vlRep > WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
		wforms.bGameOver = TRUE;
	} 
	else if (pos.nMoveNum > 100) 
	{
		PlayResWav(IDR_DRAW);
		MessageBoxMute("超过自然限着作和，辛苦了！");
		wforms.bGameOver = TRUE;
	} 
	else 
	{
		// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
		PlayResWav(pos.InCheck() ? IDR_CHECK2 : pos.Captured() ? IDR_CAPTURE2 : IDR_MOVE2);
		if (pos.Captured()) 
		{
			pos.SetIrrev();
		}
	}
}


// 初始化棋局
void Startup(void) 
{
	pos.Startup();
	wforms.sqSelected = wforms.mvLast = 0;
	wforms.bGameOver = FALSE;
}
