/*
 *	数据类型声明符的再定义.
 * 	NOTE	these typedef come from file Gbl_MacroDef.h
 */
 
#ifndef _AK_TYPES_H_ 
#define _AK_TYPES_H_

/* preliminary type definition for global area */
typedef	unsigned char			T_U8;		/* unsigned 8 bit integer */
typedef	unsigned char			T_U08;		/* unsigned 8 bit integer */
typedef	unsigned short			T_U16;		/* unsigned 16 bit integer */
typedef	unsigned long			T_U32;		/* unsigned 32 bit integer */
typedef	unsigned long long		T_U64;		/* unsigned 64 bit integer */
typedef	signed char			    T_S8;		/* signed 8 bit integer */
typedef	signed short			T_S16;		/* signed 16 bit integer */
typedef	signed long 			T_S32;		/* signed 32 bit integer */
typedef	signed long long		T_S64;		/* signed 64 bit integer */
typedef void					T_VOID;		/* void */

/* basal type definition for global area */
typedef T_S8					T_CHR;		/* char */
typedef T_U8					T_BOOL;		/* BOOL type */

typedef T_VOID *				T_pVOID;	/* pointer of void data */
typedef const T_VOID *			T_pCVOID;	/* const pointer of void data */

typedef T_S8 *					T_pSTR;		/* pointer of string */
typedef const T_S8 *			T_pCSTR;	/* const pointer of string */

typedef T_U8 *					T_pDATA;	/* pointer of data */
typedef const T_U8 *			T_pCDATA;	/* const pointer of data */

typedef T_S16					T_LEN;		/* length type: unsigned short */
typedef T_S16					T_POS;		/* position type: short */
typedef T_U32					T_COLOR;
typedef T_U16					T_TIMER;

//typedef HANDLE					T_HANDLE;			/* a handle */
//typedef LPTHREAD_START_ROUTINE	T_LPTHREAD_START;	/* handle for thread */
typedef T_U32					T_HANDLE;			/* a handle */
typedef	T_pVOID					T_LPTHREAD_START;	/* handle for thread */



typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;


#define	AK_FALSE				0
#define	AK_TRUE					1
#define AK_NULL					((T_VOID*)0)
#define NULL					0



// raw read/write
#define HAL_READ_UINT8( _register_, _value_ )        ((_value_) = *((volatile char *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ )       (*((volatile  char *)(_register_)) = (_value_))
       
#define HAL_READ_UINT16( _register_, _value_ )      ((_value_) = *((volatile unsigned short *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ )     (*((volatile  unsigned short *)(_register_)) = (_value_))
       
#define HAL_READ_UINT32( _register_, _value_ )      ((_value_) = *((volatile unsigned long *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ )     (*((volatile  unsigned long *)(_register_)) = (_value_))

#define REG32(_register_)                               (*(volatile T_U32 *)(_register_))
#define REG16(_register_)                               (*(volatile T_U16 *)(_register_))
#define REG8(_register_)                                (*(volatile T_U8 *)(_register_))


//read and write register
#define outb(v,p)		(*(volatile unsigned char  *)(p) = (v))
#define outw(v,p)		(*(volatile unsigned short *)(p) = (v))
#define outl(v,p)		(*(volatile unsigned long  *)(p) = (v))

#define inb(p)			(*(volatile unsigned char  *)(p))
#define inw(p)			(*(volatile unsigned short *)(p))
#define inl(p)			(*(volatile unsigned long  *)(p))

//read and write L2 buffer
#define WriteBuf(v,p)	(*(volatile unsigned long  *)(p) = (v))
#define ReadBuf(p)		(*(volatile unsigned long *)(p))

//read and write ram
#define WriteRamb(v,p)	(*(volatile unsigned char *)(p) = (v))
#define WriteRamw(v,p)	(*(volatile unsigned short *)(p) = (v))
#define WriteRaml(v,p)	(*(volatile unsigned long  *)(p) = (v))

#define ReadRamb(p)		(*(volatile unsigned char *)(p))
#define ReadRamw(p)		(*(volatile unsigned short *)(p))
#define ReadRaml(p)		(*(volatile unsigned long *)(p))




#endif	//  _AK_TYPES_H_
