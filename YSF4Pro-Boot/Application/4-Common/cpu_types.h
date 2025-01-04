#ifndef _CPU_TYPES_H_
#define _CPU_TYPES_H_

#define RAM_FUNC __ramfunc

typedef            void        CPU_VOID;
typedef            char        CPU_CHAR;                        /*  8-bit character                                     */
typedef  unsigned  char        CPU_BOOLEAN;                     /*  8-bit boolean or logical                            */
typedef  unsigned  char        CPU_INT08U;                      /*  8-bit unsigned integer                              */
typedef    signed  char        CPU_INT08S;                      /*  8-bit   signed integer                              */
typedef  unsigned  short       CPU_INT16U;                      /* 16-bit unsigned integer                              */
typedef    signed  short       CPU_INT16S;                      /* 16-bit   signed integer                              */
typedef  unsigned  int         CPU_INT32U;                      /* 32-bit unsigned integer                              */
typedef    signed  int         CPU_INT32S;                      /* 32-bit   signed integer                              */
typedef  unsigned  long  long  CPU_INT64U;                      /* 64-bit unsigned integer                              */
typedef    signed  long  long  CPU_INT64S;                      /* 64-bit   signed integer                              */

typedef            float       CPU_FP32;                        /* 32-bit floating point                                */
typedef            double      CPU_FP64;                        /* 64-bit floating point                                */


typedef  volatile  CPU_INT08U  CPU_REG08;                       /*  8-bit register                                      */
typedef  volatile  CPU_INT16U  CPU_REG16;                       /* 16-bit register                                      */
typedef  volatile  CPU_INT32U  CPU_REG32;                       /* 32-bit register                                      */
typedef  volatile  CPU_INT64U  CPU_REG64;                       /* 64-bit register                                      */

typedef  unsigned  char        U8;                      /*  8-bit unsigned integer                              */
typedef  unsigned  short       U16;                      /* 16-bit unsigned integer                              */
typedef  unsigned  int         U32;                      /* 32-bit unsigned integer                              */
typedef  unsigned  long  long  U64;                      /* 64-bit unsigned integer                              */

#define  DEF_BIT_NONE                                   0x00u

#define  DEF_BIT_00                                     0x01u
#define  DEF_BIT_01                                     0x02u
#define  DEF_BIT_02                                     0x04u
#define  DEF_BIT_03                                     0x08u
#define  DEF_BIT_04                                     0x10u
#define  DEF_BIT_05                                     0x20u
#define  DEF_BIT_06                                     0x40u
#define  DEF_BIT_07                                     0x80u

#define  DEF_BIT_08                                   0x0100u
#define  DEF_BIT_09                                   0x0200u
#define  DEF_BIT_10                                   0x0400u
#define  DEF_BIT_11                                   0x0800u
#define  DEF_BIT_12                                   0x1000u
#define  DEF_BIT_13                                   0x2000u
#define  DEF_BIT_14                                   0x4000u
#define  DEF_BIT_15                                   0x8000u

#define  DEF_BIT_16                               0x00010000u
#define  DEF_BIT_17                               0x00020000u
#define  DEF_BIT_18                               0x00040000u
#define  DEF_BIT_19                               0x00080000u
#define  DEF_BIT_20                               0x00100000u
#define  DEF_BIT_21                               0x00200000u
#define  DEF_BIT_22                               0x00400000u
#define  DEF_BIT_23                               0x00800000u

#define  DEF_BIT_24                               0x01000000u
#define  DEF_BIT_25                               0x02000000u
#define  DEF_BIT_26                               0x04000000u
#define  DEF_BIT_27                               0x08000000u
#define  DEF_BIT_28                               0x10000000u
#define  DEF_BIT_29                               0x20000000u
#define  DEF_BIT_30                               0x40000000u
#define  DEF_BIT_31                               0x80000000u
/*$PAGE*/
#define  DEF_BIT_32                       0x0000000100000000u
#define  DEF_BIT_33                       0x0000000200000000u
#define  DEF_BIT_34                       0x0000000400000000u
#define  DEF_BIT_35                       0x0000000800000000u
#define  DEF_BIT_36                       0x0000001000000000u
#define  DEF_BIT_37                       0x0000002000000000u
#define  DEF_BIT_38                       0x0000004000000000u
#define  DEF_BIT_39                       0x0000008000000000u

#define  DEF_BIT_40                       0x0000010000000000u
#define  DEF_BIT_41                       0x0000020000000000u
#define  DEF_BIT_42                       0x0000040000000000u
#define  DEF_BIT_43                       0x0000080000000000u
#define  DEF_BIT_44                       0x0000100000000000u
#define  DEF_BIT_45                       0x0000200000000000u
#define  DEF_BIT_46                       0x0000400000000000u
#define  DEF_BIT_47                       0x0000800000000000u

#define  DEF_BIT_48                       0x0001000000000000u
#define  DEF_BIT_49                       0x0002000000000000u
#define  DEF_BIT_50                       0x0004000000000000u
#define  DEF_BIT_51                       0x0008000000000000u
#define  DEF_BIT_52                       0x0010000000000000u
#define  DEF_BIT_53                       0x0020000000000000u
#define  DEF_BIT_54                       0x0040000000000000u
#define  DEF_BIT_55                       0x0080000000000000u

#define  DEF_BIT_56                       0x0100000000000000u
#define  DEF_BIT_57                       0x0200000000000000u
#define  DEF_BIT_58                       0x0400000000000000u
#define  DEF_BIT_59                       0x0800000000000000u
#define  DEF_BIT_60                       0x1000000000000000u
#define  DEF_BIT_61                       0x2000000000000000u
#define  DEF_BIT_62                       0x4000000000000000u
#define  DEF_BIT_63                       0x8000000000000000u

#endif