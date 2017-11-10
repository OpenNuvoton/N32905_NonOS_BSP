#define FALSE				0
#define TRUE				1

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

#define	_DWORD_(a)	*((volatile DWORD *)(a))
#define	_WORD_(a)	*((volatile WORD *)(a))
#define	_BYTE_(a)	*((volatile BYTE *)(a))
