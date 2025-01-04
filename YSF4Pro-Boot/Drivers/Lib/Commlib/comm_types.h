#ifndef _COMM_TYPES_
#define _COMM_TYPES_
//#define RAM_FUNC __ramfunc 

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned char u_int8_t;
typedef unsigned short int u_int16_t;
typedef unsigned int u_int32_t;
typedef unsigned  long  long  u_int64_t;
typedef unsigned  char BOOL;

// This type MUST be 8 bit 
typedef unsigned char	BYTE;

// These types MUST be 16 bit 
typedef short		    SHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

// These types MUST be 16 bit or 32 bit 
typedef int			INT;
typedef unsigned int	UINT;

// These types MUST be 32 bit 
typedef long			LONG;
typedef unsigned long	DWORD;

typedef unsigned long time_t;

#ifdef	FALSE
#undef	FALSE
#endif
#define FALSE	(0)

#ifdef	TRUE
#undef	TRUE
#endif
#define	TRUE	(1)

//#define SUCCESS (0)

#define ELEMENT_OF(x) (sizeof(x) / sizeof((x)[0]))

#define delay(n)  rt_thread_mdelay(n*1000) /* 秒延时 */
#define mdelay(n) rt_thread_mdelay(n)      /* 毫秒延时 */


#endif /* _COMM_TYPES_ */



