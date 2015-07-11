#include <stdio.h>

#define		INS			32768		/* 输入缓冲大小（Input Buffer Size） */
#define		OBS			4096		/* 输出缓冲大小（Output Buffer Size） */
#define		NARG		32			/* 一个宏的最大参数个数 */
#define		NINCLUDE	32			/* 包含目录的最大个数(即命令行上-I选项的最大个数) */
#define		NIF			32			/* #if条件编译的最大陷入(nesting)深度 */

#ifndef		EOF
#define		EOF		(-1)
#endif

#ifndef		NULL
#define		NULL	((void *)0)
#endif

#ifndef __alpha
typedef unsigned char uchar;
#endif

/**
 * 一共60个token类型
 */
enum toktype {
	END,
	UNCLASS,	/* 未归类，unclass，token */
	NAME,	/* 标识符名，identified name，token */
	NUMBER,	/* 数字，number，token */
	STRING,	/* 字符串，string，token */
	CCON,
	NL,
	WS,
	DSHARP,	/* 粘贴字符（ ## ），double sharp，token */
	EQ,		/* 相等运算符（ == ），equal，token */
	NEQ,	/* 不相等运算符（ != ），not equal，token */
	LEQ,	/* 小于等于运算符（ <= ），less equal，token */
	GEQ,	/* 大于等于运算符（ >= ），greater than，token */
	LSH,	/* 左移运算符（ << ），left shift，token */
	RSH,	/* 右移运算符（ >> ），right shift，token */
	LAND,	/* 逻辑与运算符（ && ），logic and，token */
	LOR,	/* 逻辑或运算符（ || ），logic or，token */
	PPLUS,	/* 自加运算符（ ++ ），plus plus，token */
	MMINUS,	/* 自减符( -- )，minus minus，token */
	ARROW,	/* 指向结构体成员运算符，arrow，token */
	SBRA,	/* 左方括号（ [ ），left bracket，token */
	SKET,	/* 右方括号（ ] ），right bracket，token */
	LP,		/* 左圆括号（ ( ），left parenthesis，token */
	RP,		/* 右圆括号（ ) ），right parenthesis，token */
	DOT,	/* 句点（ . ），dot，token */
	AND,	/* 按位与运算（ & ），bit and，token */
	STAR,	/* 星号字符（ * ），star，token */
	PLUS,	/* 加号（ + ），plus，token */
	MINUS,	/* 减号（ - ），minus，token */
	TILDE,	/* 波浪号（ ~ ），tilde，token */
	NOT,	/* 逻辑非（ ! ），logic not，token */
	SLASH,	/* 除法运算符（ / ），slash，token */
	PCT,	/* 求模运算符（ % ），percentage，token */
	LT,	/* 小于（ < ），less than，token */
	GT,	/* 大于（ > ），greater than，token */
	CIRC, /* 异或运算符，也称"闭环加法"或"本位加法"运算符（ ^ ），circ，token */
	OR,	/* 按位或运算符（ | ），bit or，token */
	QUEST,	/* 问号（ ? ），question mark，token */
	COLON,	/* 冒号（ : ），colon，token */
	ASGN,	/* 赋值（ = ），assign，token */
	COMMA,	/* 逗号（ , ），comma，token */
	SHARP,	/* 井字符（ '#' ），sharp，token */
	SEMIC,	/* 分号（ ; ），semicolon，token */
	CBRA,	/* 左花括号（ { ），left curly bracket，token */
	CKET,	/* 右花括号（ } ），right curly bracket，token */
	ASPLUS,	/* 加赋值运算符（ += ），assign plus，token */
	ASMINUS,/* 减赋值运算符（ -= ），assign minus，token */
	ASSTAR,	/* 乘赋值运算符（ *= ），assign star，token */
	ASSLASH, /* 除赋值运算符（ /= ），assign slash，token */
	ASPCT,	/* 求模赋值运算符（ %= ），assign percentage，token */
	ASCIRC, /* 按位异或赋值运算符（ ^= ），assign circ，token */
	ASLSH,	/* 左移赋值运算符（ <<= ），assign left shift，token */
	ASRSH,	/* 右移赋值运算符（ >>= ），assign right shift，token */
	ASOR,	/* 按位或赋值运算符（ |= ），assign bit or，token */
	ASAND,	/* 按位与赋值运算符（ &= ），assign bit and，token */
	ELLIPS,	/* 可变参数，ellipses argument，token */
	DSHARP1,
	NAME1,
	DEFINED,
	UMINUS
};

/**
 * 一共19个关键字
 */
enum kwtype {
	KIF,		/* #if */
	KIFDEF,		/* #ifdef */
	KIFNDEF,	/* #ifndef */
	KELIF,		/* #elif */
	KELSE,		/* #else */
	KENDIF,		/* #endif */
	KINCLUDE,	/* #include */
	KDEFINE,	/* #define */
	KUNDEF,		/* #undef */
	KLINE,		/* #line关键字(影响__LINE__的值)*/
	KERROR,		/* #error */
	KPRAGMA,	/* #pragma */
	KDEFINED,	/* defined 用法: #if defined FOO */
	KLINENO,	/* __LINE__ */
	KFILE,		/* __FILE__ */
	KDATE,		/* __DATE__ */
	KTIME,		/* __TIME__ */
	KSTDC,		/* __STDC__ */
	KEVAL		/* #eval */
};

#define		ISDEFINED	01	/* has #defined value */
#define		ISKW		02	/* is PP(preprocessor) keyword */
#define		ISUNCHANGE	04	/* can't be #defined in PP */
#define		ISMAC		010	/* builtin macro, e.g. __LINE__ */

#define		EOB		0xFE		/* sentinel(哨兵) for end of input buffer */
#define		EOFC	0xFD		/* sentinel(哨兵) for end of input file(End Of File Character) */
#define		XPWS	1			/* token flag: white space to assure token sep. */

/**
 * 描述一个token
 */
typedef struct token {
	unsigned char	type;		/* token的类型,在枚举常量toktype里取值 */
	unsigned char 	flag;		/* token的flag */
	unsigned short	hideset;	/* TODO: 隐藏集合? */
	unsigned int	wslen;		/* TODO: white space length? */
	unsigned int	len;		/* token长度 */
	uchar	*t;					/* token字串 */
} Token;

/**
 * tokenrow - 描述一串token
 */
typedef struct tokenrow {
	Token	*tp;		/* 当前正被扫描的token(current one to scan) */
	Token	*bp;		/* Token数组的基地址(base of allocated value) */
	Token	*lp;		/* 指向上上一个Token(last+1 token used) */
	int	max;			/* 数组中Token的个数(number allocated) */
} Tokenrow;

/**
 * 描述一个输入文件（栈节点）
 */
typedef struct source {
	char	*filename;		/* 源文件的文件名 */
	int		line;			/* 当前的行号 */
	int		lineinc;		/* TODO: adjustment for \\n lines */
	uchar	*inb;			/* 输入缓冲区的基地址(input base) */
	uchar	*inp;			/* 指向输入缓冲区中当前正在处理的字符(input position) */
	uchar	*inl;			/* 指向当前输入缓冲区中最后一个有效字符的下一个字符（其后可能是EOB或EOF标记）(input length) */
	FILE*	fd;				/* 输入源文件的FILE指针(input source) */
	int		ifdepth;		/* conditional nesting in include */
	struct	source *next;	/* stack for #include */
} Source;

/**
 * nlist - token的hash表
 */
typedef struct nlist {
	struct nlist *next;		/* 下一节点 */
	uchar	*name;			/* 名字 */
	int	len;				/* 长度 */
	Tokenrow *vp;			/* 宏展开的值(value as macro) */
	Tokenrow *ap;			/* 参数名的列表,如果有的话(list of argument names, if any) */
	char	val;			/* enum toktype类型(value as preprocessor name) */
	char	flag;			/* is defined, is pp name */
} Nlist;

/**
 * includelist - 文件包含的list
 */
typedef	struct	includelist {
	char	deleted;	/* 设置为0表明，从头文件包含目录列表中删除 */
	char	always;		/* TODO:总是干嘛？ */
	char	*file;		/* -I指令中的包含目录 */
} Includelist;

#define		new(t)			(t *)domalloc(sizeof(t))

#define		quicklook(a,b)	(namebit[(a)&077] & (1<<((b)&037)))
#define		quickset(a,b)	namebit[(a)&077] |= (1<<((b)&037))

extern	unsigned long namebit[077+1];

enum errtype { WARNING, ERROR, FATAL }; /* 错误级别 */

void	expandlex(void);
void	fixlex(void);
void	setup(int, char **);
int	gettokens(Tokenrow *, int);
int	comparetokens(Tokenrow *, Tokenrow *);
Source	*setsource(char *, FILE *, char *);
void	unsetsource(void);
void	puttokens(Tokenrow *);
void	process(Tokenrow *);
void	*domalloc(int);
void	dofree(void *);
void	error(enum errtype, char *, ...);
void	flushout(void);
int	fillbuf(Source *);
int	trigraph(Source *);
int	foldline(Source *);
Nlist	*lookup(Token *, int);
void	control(Tokenrow *);
void	dodefine(Tokenrow *);
void	doadefine(Tokenrow *, int);
void	doinclude(Tokenrow *);
void	doif(Tokenrow *, enum kwtype);
void	expand(Tokenrow *, Nlist *);
void	builtin(Tokenrow *, int);
int	gatherargs(Tokenrow *, Tokenrow **, int *);
void	substargs(Nlist *, Tokenrow *, Tokenrow **);
void	expandrow(Tokenrow *, char *);
void	maketokenrow(int, Tokenrow *);
Tokenrow *copytokenrow(Tokenrow *, Tokenrow *);
Token	*growtokenrow(Tokenrow *);
Tokenrow *normtokenrow(Tokenrow *);
void	adjustrow(Tokenrow *, int);
void	movetokenrow(Tokenrow *, Tokenrow *);
void	insertrow(Tokenrow *, int, Tokenrow *);
void	peektokens(Tokenrow *, char *);
void	doconcat(Tokenrow *);
Tokenrow *stringify(Tokenrow *);
int	lookuparg(Nlist *, Token *);
long	eval(Tokenrow *, int);
void	genline(void);
void	setempty(Tokenrow *);
void	makespace(Tokenrow *);
char	*outnum(char *, int);
int	digit(int);
uchar	*newstring(uchar *, int, int);
int	checkhideset(int, Nlist *);
void	prhideset(int);
int	newhideset(int, Nlist *);
int	unionhideset(int, int);
void	iniths(void);
void	setobjname(char *);
#define	rowlen(tokrow)	((tokrow)->lp - (tokrow)->bp)

extern	char *outp;
extern	Token	nltoken;
extern	Source *cursource;
extern	char *curtime;
extern	int incdepth;
extern	int ifdepth;
extern	int ifsatisfied[NIF];
extern	int Mflag;
extern	int skipping;
extern	int verbose;
extern	int Cplusplus;
extern	Nlist *kwdefined;
extern	Includelist includelist[NINCLUDE];
extern	char wd[];
