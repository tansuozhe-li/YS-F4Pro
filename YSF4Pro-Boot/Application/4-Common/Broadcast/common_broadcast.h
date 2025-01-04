/************************************************************************
** File name: common_broadcast.h
* Description: 实现通过消息队列发送广播包信息，包含相关接口函数
* Others： 
* Version： 
* Author： 
* Author: ID: 
* Modification:
**********************************************************************/
#ifndef _COMMON_BROADCAST_H_
#define _COMMON_BROADCAST_H_

#include "comm_types.h"
#include "broadcast.h"
  
extern struct rt_broadcast_server g_mq_broadcast_server;
#pragma  pack(push,1)

/* 发送消息广播包数据结构 */
#define MAX_MSG_BODY      120
#define MSG_TYPE_LEN      25
typedef struct _mq_brodcast_msg{
  u_int8_t   msg_type[MSG_TYPE_LEN];//消息类型,字符串,消息名不要大于等于25字节
  u_int32_t  msg_save_addr;//消息存放地址
  u_int16_t  msg_len;      //消息长度
  u_int8_t   msg_body[MAX_MSG_BODY]; //预留字节，用于直接传递数据
}mq_brodcast_msg;

/* 消息处理函数 */
typedef BOOL (*MsgProcFunc)(mq_brodcast_msg *);

/* 广播消息处理表 */
typedef struct
{
    u_int8_t     msgType[20];   /* 消息ID */
    char         msgName[64];   /* 消息名 */
    MsgProcFunc  pMsgProc;      /* 消息处理函数 */
} BroadcastMsgProcTable_t;

//发送广播包
BOOL send_mq_broadcast_msg(      u_int8_t* msg_type,
                                   u_int32_t msg_save_addr,
                                   u_int16_t msg_len,
                                   u_int8_t *msg_body);


int InitBroadcastServiceModule(void);
#pragma pack(pop)

#endif
