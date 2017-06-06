#ifndef POSITIONSTRUCT_H_INCLUDED
#define POSITIONSTRUCT_H_INCLUDED

// 棋盘范围
const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3;
const int FILE_RIGHT = 11;

// 棋子编号
const int PIECE_KING = 0;
const int PIECE_ADVISOR = 1;
const int PIECE_BISHOP = 2;
const int PIECE_KNIGHT = 3;
const int PIECE_ROOK = 4;
const int PIECE_CANNON = 5;
const int PIECE_PAWN = 6;

// "GenerateMoves"参数
const BOOL GEN_CAPTURE = TRUE;

// 其他常数
const int MAX_GEN_MOVES = 128; // 最大的生成走法数
const int MAX_MOVES = 256;     // 最大的历史走法数
const int LIMIT_DEPTH = 64;    // 最大的搜索深度
const int MATE_VALUE = 10000;  // 最高分值，即将死的分值
const int BAN_VALUE = MATE_VALUE - 100; // 长将判负的分值，低于该值将不写入置换表
const int WIN_VALUE = MATE_VALUE - 200; // 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
const int DRAW_VALUE = 20;     // 和棋时返回的分数(取负值)
const int ADVANCED_VALUE = 3;  // 先行权分值
const int RANDOM_MASK = 7;     // 随机性分值
const int NULL_MARGIN = 400;   // 空步裁剪的子力边界
const int NULL_DEPTH = 2;      // 空步裁剪的裁剪深度
const int HASH_SIZE = 1 << 20; // 置换表大小
const int HASH_ALPHA = 1;      // ALPHA节点的置换表项
const int HASH_BETA = 2;       // BETA节点的置换表项
const int HASH_PV = 3;         // PV节点的置换表项
const int BOOK_SIZE = 16384;   // 开局库大小

extern const char ccInBoard[256];
extern const char ccInFort[256];
extern const char ccLegalSpan[512];
extern const char ccKnightPin[512];
extern const char ccKingDelta[4];
extern const char ccAdvisorDelta[4];
extern const char ccKnightDelta[4][2];
extern const char ccKnightCheckDelta[4][2];
extern const BYTE cucpcStartup[256];
extern const BYTE cucvlPiecePos[7][256];

/////////////////////////////////////////////////////////////////////////
// 判断棋子是否在棋盘中
inline BOOL IN_BOARD(int sq) {
	return ccInBoard[sq] != 0;
}

// 判断棋子是否在九宫中
inline BOOL IN_FORT(int sq) {
	return ccInFort[sq] != 0;
}

// 获得格子的横坐标
inline int RANK_Y(int sq) {
	return sq >> 4;
}

// 获得格子的纵坐标
inline int FILE_X(int sq) {
	return sq & 15;
}

// 根据纵坐标和横坐标获得格子
inline int COORD_XY(int x, int y) {
	return x + (y << 4);
}

// 翻转格子
inline int SQUARE_FLIP(int sq) {
	return 254 - sq;
}

// 纵坐标水平镜像
inline int FILE_FLIP(int x) {
	return 14 - x;
}

// 横坐标垂直镜像
inline int RANK_FLIP(int y) {
	return 15 - y;
}

// 格子水平镜像
inline int MIRROR_SQUARE(int sq) {
	return COORD_XY(FILE_FLIP(FILE_X(sq)), RANK_Y(sq));
}

// 格子水平镜像
inline int SQUARE_FORWARD(int sq, int sd) {
	return sq - 16 + (sd << 5);
}

// 走法是否符合帅(将)的步长
inline BOOL KING_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 1;
}

// 走法是否符合仕(士)的步长
inline BOOL ADVISOR_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 2;
}

// 走法是否符合相(象)的步长
inline BOOL BISHOP_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 3;
}

// 相(象)眼的位置
inline int BISHOP_PIN(int sqSrc, int sqDst) {
	return (sqSrc + sqDst) >> 1;
}

// 马腿的位置
inline int KNIGHT_PIN(int sqSrc, int sqDst) {
	return sqSrc + ccKnightPin[sqDst - sqSrc + 256];
}

// 是否未过河
inline BOOL HOME_HALF(int sq, int sd) {
	return (sq & 0x80) != (sd << 7);
}

// 是否已过河
inline BOOL AWAY_HALF(int sq, int sd) {
	return (sq & 0x80) == (sd << 7);
}

// 是否在河的同一边
inline BOOL SAME_HALF(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x80) == 0;
}

// 是否在同一行
inline BOOL SAME_RANK(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0xf0) == 0;
}

// 是否在同一列
inline BOOL SAME_FILE(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x0f) == 0;
}

// 获得红黑标记(红子是8，黑子是16)
inline int SIDE_TAG(int sd) {
	return 8 + (sd << 3);
}

// 获得对方红黑标记
inline int OPP_SIDE_TAG(int sd) {
	return 16 - (sd << 3);
}

// 获得走法的起点
inline int SRC(int mv) {
	return mv & 255;
}

// 获得走法的终点
inline int DST(int mv) {
	return mv >> 8;
}

// 根据起点和终点获得走法
inline int MOVE(int sqSrc, int sqDst) {
	return sqSrc + sqDst * 256;
}

// 走法水平镜像
inline int MIRROR_MOVE(int mv) {
	return MOVE(MIRROR_SQUARE(SRC(mv)), MIRROR_SQUARE(DST(mv)));
}

// RC4密码流生成器
struct RC4Struct {
	BYTE s[256];
	int x, y;

	void InitZero(void);   // 用空密钥初始化密码流生成器
	BYTE NextByte(void) {  // 生成密码流的下一个字节
		BYTE uc;
		x = (x + 1) & 255;
		y = (y + s[x]) & 255;
		uc = s[x];
		s[x] = s[y];
		s[y] = uc;
		return s[(s[x] + s[y]) & 255];
	}
	DWORD NextLong(void) { // 生成密码流的下四个字节
		BYTE uc0, uc1, uc2, uc3;
		uc0 = NextByte();
		uc1 = NextByte();
		uc2 = NextByte();
		uc3 = NextByte();
		return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
	}
};


// 置换表项结构
struct HashItem {
	BYTE ucDepth, ucFlag;
	short svl;
	WORD wmv, wReserved;
	DWORD dwLock0, dwLock1;
};

// 开局库项结构
struct BookItem {
	DWORD dwLock;
	WORD wmv, wvl;
};

struct SEARCH{
	int mvResult;                  // 电脑走的棋
	int nHistoryTable[65536];      // 历史表
	int mvKillers[LIMIT_DEPTH][2]; // 杀手走法表
	HashItem HashTable[HASH_SIZE]; // 置换表
	int nBookSize;                 // 开局库大小
	BookItem BookTable[BOOK_SIZE]; // 开局库
} ;

// Zobrist结构
struct ZobristStruct {
	DWORD dwKey, dwLock0, dwLock1;

	void InitZero(void) {                 // 用零填充Zobrist
		dwKey = dwLock0 = dwLock1 = 0;
	}
	void InitRC4(RC4Struct &rc4) {        // 用密码流填充Zobrist
		dwKey = rc4.NextLong();
		dwLock0 = rc4.NextLong();
		dwLock1 = rc4.NextLong();
	}
	void Xor(const ZobristStruct &zobr) { // 执行XOR操作
		dwKey ^= zobr.dwKey;
		dwLock0 ^= zobr.dwLock0;
		dwLock1 ^= zobr.dwLock1;
	}
	void Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2) {
		dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
		dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
		dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
	}
};

// Zobrist表
struct ZOBRIST{
	ZobristStruct Player;
	ZobristStruct Table[14][256];
};

extern ZOBRIST Zobrist;

// 历史走法信息(占4字节)
struct MoveStruct {
	WORD wmv;
	BYTE ucpcCaptured, ucbCheck;
	DWORD dwKey;

	void Set(int mv, int pcCaptured, BOOL bCheck, DWORD dwKey_) {
		wmv = mv;
		ucpcCaptured = pcCaptured;
		ucbCheck = bCheck;
		dwKey = dwKey_;
	}
}; // mvs

// 局面结构
struct Position {
	int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
	BYTE ucpcSquares[256];          // 棋盘上的棋子
	int vlWhite, vlBlack;           // 红、黑双方的子力价值
	int nDistance, nMoveNum;        // 距离根节点的步数，历史走法数
	MoveStruct mvsList[MAX_MOVES];  // 历史走法信息列表
	ZobristStruct zobr;             // Zobrist

	void ClearBoard(void) {         // 清空棋盘
		sdPlayer = vlWhite = vlBlack = nDistance = 0;
		memset(ucpcSquares, 0, 256);
		zobr.InitZero();
	}
	void SetIrrev(void) {           // 清空(初始化)历史走法信息
		mvsList[0].Set(0, 0, Checked(), zobr.dwKey);
		nMoveNum = 1;
	}
	void Startup(void);             // 初始化棋盘
	void ChangeSide(void) {         // 交换走子方
		sdPlayer = 1 - sdPlayer;
		zobr.Xor(Zobrist.Player);
	}
	void AddPiece(int sq, int pc) { // 在棋盘上放一枚棋子
		ucpcSquares[sq] = pc;
		// 红方加分，黑方(注意"cucvlPiecePos"取值要颠倒)减分
		if (pc < 16) {
			vlWhite += cucvlPiecePos[pc - 8][sq];
			zobr.Xor(Zobrist.Table[pc - 8][sq]);
		} else {
			vlBlack += cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pc - 9][sq]);
		}
	}
	void DelPiece(int sq, int pc) { // 从棋盘上拿走一枚棋子
		ucpcSquares[sq] = 0;
		// 红方减分，黑方(注意"cucvlPiecePos"取值要颠倒)加分
		if (pc < 16) {
			vlWhite -= cucvlPiecePos[pc - 8][sq];
			zobr.Xor(Zobrist.Table[pc - 8][sq]);
		} else {
			vlBlack -= cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pc - 9][sq]);
		}
	}
	int Evaluate(void) const {      // 局面评价函数
		return (sdPlayer == 0 ? vlWhite - vlBlack : vlBlack - vlWhite) + ADVANCED_VALUE;
	}
	BOOL InCheck(void) const {      // 是否被将军
		return mvsList[nMoveNum - 1].ucbCheck;
	}
	BOOL Captured(void) const {     // 上一步是否吃子
		return mvsList[nMoveNum - 1].ucpcCaptured != 0;
	}
	int MovePiece(int mv);                      // 搬一步棋的棋子
	void UndoMovePiece(int mv, int pcCaptured); // 撤消搬一步棋的棋子
	BOOL MakeMove(int mv);                      // 走一步棋
	void UndoMakeMove(void) {                   // 撤消走一步棋
		nDistance --;
		nMoveNum --;
		ChangeSide();
		UndoMovePiece(mvsList[nMoveNum].wmv, mvsList[nMoveNum].ucpcCaptured);
	}
	void NullMove(void) {                       // 走一步空步
		DWORD dwKey;
		dwKey = zobr.dwKey;
		ChangeSide();
		mvsList[nMoveNum].Set(0, 0, FALSE, dwKey);
		nMoveNum ++;
		nDistance ++;
	}
	void UndoNullMove(void) {                   // 撤消走一步空步
		nDistance --;
		nMoveNum --;
		ChangeSide();
	}
	// 生成所有走法，如果"bCapture"为"TRUE"则只生成吃子走法
	int GenerateMoves(int *mvs, BOOL bCapture = FALSE) const;
	BOOL LegalMove(int mv) const;               // 判断走法是否合理
	BOOL Checked(void) const;                   // 判断是否被将军
	BOOL IsMate(void);                          // 判断是否被杀
	int DrawValue(void) const {                 // 和棋分值
		return (nDistance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
	}
	int RepStatus(int nRecur = 1) const;        // 检测重复局面
	int RepValue(int nRepStatus) const {        // 重复局面分值
		int vlReturn;
		vlReturn = ((nRepStatus & 2) == 0 ? 0 : nDistance - BAN_VALUE) +
			((nRepStatus & 4) == 0 ? 0 : BAN_VALUE - nDistance);
		return vlReturn == 0 ? DrawValue() : vlReturn;
	}
	BOOL NullOkay(void) const {                 // 判断是否允许空步裁剪
		return (sdPlayer == 0 ? vlWhite : vlBlack) > NULL_MARGIN;
	}
	void Mirror(Position &posMirror) const; // 对局面镜像
};

int 	SearchMain(void) ;
void	LoadBook(HINSTANCE hInst);

extern Position pos;		// 局面实例

#endif