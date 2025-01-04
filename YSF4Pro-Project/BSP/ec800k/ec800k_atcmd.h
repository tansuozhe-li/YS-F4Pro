#ifndef _EC800K_H
#define _EC800K_H
#include "uart_dev.h"
#include "ring_buff.h"
#include "comm_types.h"

typedef u_int32_t lte_device_handle;
 
extern lte_device_handle g_main_lte_handle;
extern lte_device_handle g_sub_lte_handle;

/* 通道拨号状态 */
typedef enum {
    LTE_NET_CONNECTED_FAILED,       
    LTE_NET_CONNECTED_IDEL,     
    LTE_NET_CONNECTED_SUCCESS,      
}net_status_e;

#pragma  pack(push,1)
// 通信通道结构
typedef struct _chn_table{
    u_int8_t  serv_ip[20];
    u_int16_t port;
    u_int8_t  net_type[4]; 
    RingQueue data_in_buff;     
    u_int8_t  connected_stats;
    rt_sem_t  sem_dist_data;
    uint32_t  recv_data_len;
}chn_table_t;

// LTE通信模块结构
#define MAX_CHN_NUM          2
#define MAX_LTE_MODEM_NUM    2
typedef struct _lte_table{
    chn_table_t stchn_table[MAX_CHN_NUM]; 
    u_int8_t  accesspoint[20];
    u_int8_t  username[20]; 
    u_int8_t  password[20]; 
    u_int8_t  signal_intensity;
    u_int8_t  ccid[20];
    u_int8_t  dial_status;
}lte_table_t;
extern lte_table_t g_stlte_table[MAX_LTE_MODEM_NUM];

// LTE通信设备驱动结构体 
typedef struct _lte_device_info{
    u_int8_t      init;              
    u_int8_t      index;             
    device_handle lte_uart_handle;   
    RingQueue     cmd_in_buff;       
    rt_sem_t      sem_dist_cmd;      
    rt_sem_t      sem_mutex_send_data;
    rt_sem_t      sem_mutex_read_at_lock;
    lte_table_t   stlte_chn_table;
}lte_device_info_t;
#pragma pack(pop)

BOOL ec800k_send_data(lte_device_handle lte_dev,u_int8_t chnn,u_int8_t *pdata,int data_len);
BOOL ec800k_check_init_status(lte_device_handle lte_dev);
BOOL ec800k_activate_the_scene(lte_device_handle lte_dev); 
BOOL ec800k_get_local_ip(lte_device_handle lte_dev);
BOOL ec800k_query_network_status(lte_device_handle lte_dev); 
BOOL ec800k_open_echo_fault_code(lte_device_handle lte_dev);
BOOL ec800k_Turn_off_airplane_mode(lte_device_handle lte_dev);
BOOL ec800k_Turn_on_airplane_mode(lte_device_handle lte_dev);
BOOL ec800k_test_at_echo(lte_device_handle lte_dev);
BOOL ec800k_set_baud(lte_device_handle lte_dev);
BOOL ec800k_close_at_echo(lte_device_handle lte_dev);
BOOL ec800k_sleep_on(lte_device_handle lte_dev);
BOOL ec800k_query_csq(lte_device_handle lte_dev);
BOOL ec800k_check_sim_card(lte_device_handle lte_dev);
BOOL ec800k_query_sim_ccid(lte_device_handle lte_dev);
BOOL ec800k_set_access_point(lte_device_handle lte_dev);
BOOL ec800k_create_ip_channel(lte_device_handle lte_dev,u_int8_t chnn);
BOOL ec800k_power_down(lte_device_handle lte_dev);
BOOL ec800k_tcpclose(lte_device_handle lte_dev,u_int8_t chnn);
lte_device_handle lte_dev_init(u_int8_t index, u_int8_t uart_index, u_int32_t baud,
                                         u_int8_t *cmd_in_buff,      int cmd_in_buff_size,
                                         u_int8_t *ch0_data_in_buff, int ch0_data_in_buff_size,
                                         u_int8_t *ch1_data_in_buff, int ch1_data_in_buff_size); 

int ec800k_process_readln(lte_device_handle lte_device, void const *data, int data_len);
int ec800k_uart_readln(lte_device_handle lte_device, u_int8_t *buff, int buff_size,u_int32_t timeout_ms);
#endif 










