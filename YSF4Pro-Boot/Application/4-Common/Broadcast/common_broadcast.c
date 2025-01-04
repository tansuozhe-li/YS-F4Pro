/************************************************************************
** File name:
* Description:包含消息队列广播包接口函数
* Others： 
* Version： 
* Author： 
* Author: ID: 
* Modification:
************************************************************************/
// 包含的头文件
#include <string.h>
#include "common_broadcast.h"
#include <rtdef.h>
#include "rtdbg.h"

rt_sem_t    sem_mutex_mq_broadcast_msg;
//广播服务器
struct rt_broadcast_server g_mq_broadcast_server;
 

///@brief 发送广播包
BOOL send_mq_broadcast_msg(  u_int8_t* msg_type,
                             u_int32_t msg_save_addr,
                             u_int16_t msg_len,
                             u_int8_t *msg_body)
{       
        rt_sem_take(sem_mutex_mq_broadcast_msg, RT_WAITING_FOREVER);  

        rt_err_t err;
        static u_int8_t buff[MAX_MSG_BODY+50]={0x00};
        
        if(msg_len > MAX_MSG_BODY){
          LOG_D("[MSGQ]: mq_brodcast_ms->.msg_body over len\r\n");
          rt_sem_release(sem_mutex_mq_broadcast_msg);  
          return FALSE;
        }
        mq_brodcast_msg *msg_pack = (mq_brodcast_msg*)buff;
        strcpy((char *)msg_pack->msg_type, (char *)msg_type);
        msg_pack->msg_save_addr = msg_save_addr;
        msg_pack->msg_len = msg_len;
        memcpy(msg_pack->msg_body, msg_body, msg_len);
        
        err =rt_broadcast_send(&g_mq_broadcast_server, msg_pack, msg_len+ sizeof(msg_pack->msg_type)+sizeof(msg_pack->msg_save_addr)+ sizeof(msg_pack->msg_len)); 
        if(RT_EOK == err){
            LOG_D("[MSGQ]: ->send msg:%s ok\r\n",msg_pack->msg_type);
        }else{
            LOG_E("[MSGQ]: ->send msg:%s failed,err:%d\r\n",msg_pack->msg_type,err);
        }
        rt_thread_delay(50);

        rt_sem_release(sem_mutex_mq_broadcast_msg);  
        return TRUE;
}


/**************************************************************************
* Func Name： 
* Description： 
* Others： 
* Date  Version  Author  修改内容
* -----------------------------------------------
* V1.0  XXXX     XXXX
***************************************************************************/

int InitBroadcastServiceModule(void)
{
    rt_broadcast_server_init(&g_mq_broadcast_server);//初始化广播服务器,上行下行用同一个

    /* 创建互斥信号量 */
    sem_mutex_mq_broadcast_msg = rt_sem_create( "sem_mutex_mq_broadcast_msg", 1u, RT_IPC_FLAG_FIFO);
    
    return 0;
}



