/************************************************************************
** File name: broadcas.h
* Description: 实现通过消息队列发送广播包信息，包含相关接口函数
* Others： 
* Version： 
* Author： 
* Author: ID: 
* Modification:
**********************************************************************/
#ifndef _BROADCAST_H_
#define _BROADCAST_H_

#include <rtthread.h>
#include "rtdef.h"

//广播服务器的控制块
struct rt_broadcast_server
{
	//客户端节点链表
    rt_list_t client_list;
};
typedef struct rt_broadcast_server *rt_broadcast_server_t;
 
//广播接客户端的控制块
struct rt_broadcast_client
{    //接收消息队列
     struct rt_messagequeue receive_mq;
	   //客户端节点
     rt_list_t node;
     //此广播客户端对应的服务器
     //rt_broadcast_server_t server;
};
typedef struct rt_broadcast_client *rt_broadcast_client_t;

/* 初始化广播服务器 */
rt_err_t rt_broadcast_server_init(rt_broadcast_server_t server);

/* 初始化广播客户端 */
rt_err_t rt_broadcast_client_init(rt_broadcast_client_t client,
		                                    const char *name,
		                                    void *msgpool,
		                                    rt_size_t   msg_size,
		                                    rt_size_t   pool_size,
		                                    rt_uint8_t  flag);

/* 给广播服务器注册客户端 */
rt_err_t rt_broadcast_client_regist(rt_broadcast_server_t server,rt_broadcast_client_t client);

/* 卸载一个客户端 */
rt_err_t rt_broadcast_client_unregist(rt_broadcast_client_t client);

/* 接收广播 */
rt_err_t rt_broadcast_recv(rt_broadcast_client_t client,void *buffer,rt_size_t size,rt_int32_t timeout);

/* 发送广播包 */
rt_err_t rt_broadcast_send(rt_broadcast_server_t server, void *buffer, rt_size_t size);
#endif
