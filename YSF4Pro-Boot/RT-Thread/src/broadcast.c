/************************************************************************
** File name: broadcast.c
* Description: 实现通过消息队列发送广播包信息，包含相关接口函数
* Others： 
* Version： 
* Author： 
* Author: ID: 
* Modification:
***********************************************************************/
#include "common_broadcast.h"
#include "rthw.h"

/**************************************************************************
* Func Name：   rt_broadcast_server_init
* Description ：初始化广播服务器
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
***************************************************************************/
rt_err_t rt_broadcast_server_init(rt_broadcast_server_t server)
{
    RT_ASSERT(server !=RT_NULL);
 
	//初始化客户端链表
    rt_list_init(&(server->client_list));
 
    return RT_EOK;
}
//RTM_EXPORT(rt_broadcast_server_init);

/**************************************************************************
* Func Name：   rt_broadcast_client_init
* Description ：初始化广播客户端
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*************************************************************************/
rt_err_t rt_broadcast_client_init(rt_broadcast_client_t client,
                                    const char *name,
                                    void *msgpool,
                                    rt_size_t   msg_size,
                                    rt_size_t   pool_size,
                                    rt_uint8_t  flag)
{
    rt_err_t err;
 
    RT_ASSERT(client !=RT_NULL);
   	//使用传入的消息池初始化接收消息队列
    err =rt_mq_init(&client->receive_mq,name,msgpool,msg_size,pool_size,flag);
    if(err !=RT_EOK)
    {
        return err;
    }
 
	//初始化客户端节点
    rt_list_init(&client->node);
    return RT_EOK;    
}
//RTM_EXPORT(rt_broadcast_client_init);

/**************************************************************************
* Func Name：   rt_broadcast_client_regist
* Description ：给广播服务器注册客户端
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
***********************************************************************/
rt_err_t rt_broadcast_client_regist(rt_broadcast_server_t server,rt_broadcast_client_t client)
{
    register rt_base_t temp;
 
    RT_ASSERT(server !=RT_NULL);
    RT_ASSERT(client !=RT_NULL);
   	//开关断
    temp = rt_hw_interrupt_disable();
	//将客户端节点注册到到服务器
    rt_list_insert_after(&server->client_list,&client->node);
 
    //开中断
    rt_hw_interrupt_enable(temp);
    return RT_EOK;
}
//RTM_EXPORT(rt_broadcast_client_regist)

/**************************************************************************
* Func Name：   rt_broadcast_client_unregist
* Description ：卸载一个客户端
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
***********************************************************************/
rt_err_t rt_broadcast_client_unregist(rt_broadcast_client_t client)
{
    register rt_base_t temp;
 
    RT_ASSERT(client !=RT_NULL);
    //关中断
    temp = rt_hw_interrupt_disable();
	//将客户端节点从服务器上移除
    rt_list_remove(&client->node);
 
    //开中断
    rt_hw_interrupt_enable(temp);
    return RT_EOK;
}
//RTM_EXPORT(rt_broadcast_client_unregist);

#if 0
/**************************************************************************
* Func Name：   rt_broadcast_client_detach
* Description ：脱离广播客户端
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*********************************************************************/
rt_err_t rt_broadcast_client_detach(rt_broadcast_client_t client)
{
	rt_err_t err;
 
    RT_ASSERT(client !=RT_NULL);
 
	//扯载此客户端
	err =rt_broadcast_client_unregist(client);
	if(err !=RT_EOK)
	{
		return err;
	}
	//脱离此客户端的消息队列
	return rt_mq_detach(&client->receive_mq);
}
//RTM_EXPORT(rt_broadcast_client_detach);

/**************************************************************************
* Func Name：   rt_broadcast_server_detach
* Description ：脱离广播服务器
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*********************************************************************/
rt_err_t rt_broadcast_server_detach(rt_broadcast_server_t server)
{
	struct rt_list_node *n;
    rt_broadcast_client_t client;
    rt_err_t err;
    rt_err_t result =RT_EOK;
 
    RT_ASSERT(server != RT_NULL);
 
 
	//遍历服务服务器上已注册的客户端节点
	if (!rt_list_isempty(&server->client_list))
    {
        n = server->client_list.next;
        while (n != &server->client_list)
        {
             client =rt_list_entry(n,struct rt_broadcast_client,node);
 
			 //脱离此客户端
             err =rt_broadcast_client_detach(client);
             if(err !=RT_EOK)
             {
                result =err;
             } 
        }
    }
 
    return result;
 
}
//RTM_EXPORT(rt_broadcast_server_detach); 
#endif

/**************************************************************************
* Func Name：   rt_broadcast_recv
* Description ：接收广播
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*******************************************************************/
rt_err_t rt_broadcast_recv(rt_broadcast_client_t client,void *buffer,rt_size_t size,rt_int32_t timeout)
{
    RT_ASSERT(client != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);
 
    return rt_mq_recv(&client->receive_mq,buffer,size,timeout);
}
//RTM_EXPORT(rt_broadcast_recv); 

/**************************************************************************
* Func Name：   rt_broadcast_send
* Description ：发送广播
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*******************************************************************/
rt_err_t rt_broadcast_send(rt_broadcast_server_t server, void *buffer, rt_size_t size)
{
    struct rt_list_node *n;
    rt_broadcast_client_t client;
    rt_err_t err;
    rt_err_t result =RT_EOK;
 
    RT_ASSERT(server != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);
    
    //进入临界区,关调度器
    rt_enter_critical();
	//遍历服务器上所有注册的客户端节点，给每个客户端发送消息
    if (!rt_list_isempty(&server->client_list))
    {
        n = server->client_list.next;
		while (n != &server->client_list)
        {
             client =rt_list_entry(n,struct rt_broadcast_client,node);
             err =rt_mq_send(&client->receive_mq,buffer,size);
             if(err !=RT_EOK)
             {
                result =err;
             }
			 n = n->next; 
        }
    }
 
    //出临界区,再次使用调度器
    rt_exit_critical();
 
    return result;
}
//RTM_EXPORT(rt_broadcast_send);

#if 0
/**************************************************************************
* Func Name：   rt_broadcast_ugent
* Description ：紧急发送广播
* Return Value： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     
*******************************************************************/
rt_err_t rt_broadcast_ugent(rt_broadcast_server_t server, void *buffer, rt_size_t size)
{
    struct rt_list_node *n;
    rt_broadcast_client_t client;
    rt_err_t err;
    rt_err_t result =RT_EOK;
 
    RT_ASSERT(server != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);
    
    //进入临界区,关调度器
    rt_enter_critical();
 
	//遍历服务器上所有注册的客户端节点，给每个客户端紧急发送消息
    if (!rt_list_isempty(&server->client_list))
    {
        n = server->client_list.next;
        while (n != &server->client_list)
        {
             client =rt_list_entry(n,struct rt_broadcast_client,node);
             err =rt_mq_urgent(&client->receive_mq,buffer,size);
             if(err !=RT_EOK)
             {
                result =err;
             }
			 n = n->next; 
        }
    }
 
    //出临界区,再次使用调度器
    rt_exit_critical();
 
    return result;
}
#endif