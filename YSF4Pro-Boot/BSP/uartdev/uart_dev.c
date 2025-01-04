#include <stdio.h>
#include <stdarg.h>
#include "uart_dev.h"
#include "comm_types.h"

extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

device_handle g_console_uart_handle;
device_handle g_mcu_lte1_uart_handle;
device_handle g_mcu_lte2_uart_handle;
device_handle g_rs485_uart_handle;
device_handle g_RS232_uart_handle;

static void uart_init(u_int8_t uart_index, int baud);

///@brief  串口信息初始化，串口号及波特率.
uart_device_info_t st_uart_devices[]={
  {.init_status = FALSE, .uart_index = 0,},
  {.init_status = FALSE, .uart_index = 1,},
  {.init_status = FALSE, .uart_index = 2,},
  {.init_status = FALSE, .uart_index = 3,},
  {.init_status = FALSE, .uart_index = 4,},
  {.init_status = FALSE, .uart_index = 5,},
  {.init_status = FALSE, .uart_index = 6,},
};

///@brief  初始化串口设备.默认使用初始化波特率
device_handle uart_dev_init(u_int8_t uart_index, u_int32_t uart_baudrate)
{
  int i = 0;
  for(; i < ELEMENT_OF(st_uart_devices); i++){
    if(st_uart_devices[i].uart_index == uart_index){
      if(!st_uart_devices[i].init_status){ 
        InitRingQueue(&st_uart_devices[i].uart_ring_queue,st_uart_devices[i].inbuff, sizeof(st_uart_devices[i].inbuff)); 
        
        if(uart_baudrate == NULL){
           uart_init(uart_index, st_uart_devices[i].uart_baudrate);  
        }else{
           uart_init(uart_index, uart_baudrate);  
        }
        st_uart_devices[i].port_mutex    = rt_sem_create("port mutex",   1u, RT_IPC_FLAG_FIFO);
        st_uart_devices[i].ser_mutex     = rt_sem_create("serial mutex",  1u, RT_IPC_FLAG_FIFO);
        st_uart_devices[i].sem_recv_data = rt_sem_create("sem recv data", 0u, RT_IPC_FLAG_FIFO);
        st_uart_devices[i].init_status = TRUE;
      }
      
      st_uart_devices[i].uartdev_handle =  (device_handle)(&st_uart_devices[i]);
      uart_enable_irq(uart_index);
      return st_uart_devices[i].uartdev_handle;
    }
  }
  return 0;
}

///@brief  串口硬件初始化.
static void uart_init(u_int8_t uart_index, int baud)
{   
  switch(uart_index){
  //case 0:MX_LPUART1_UART_Init();break;
  case 1:MX_USART1_UART_Init();break;
  case 2:MX_USART2_UART_Init();break; 
  //case 3:MX_USART3_UART_Init();break; 
  //case 4:MX_UART4_Init();break;
  //case 5:MX_UART5_Init();break; 
  //case 6:MX_USART6_UART_Init();break;
  default:break;
  }
}

///@brief  根据uart_index返回对应串口
static UART_HandleTypeDef get_uartHandTypeDef(u_int8_t uart_index)
{
      UART_HandleTypeDef uartnum;
      
      switch(uart_index){
        //case 0: uartnum = hlpuart1;break;
        case 1:uartnum = huart1;break;
        case 2:uartnum = huart2;break;
        //case 3:uartnum = huart3;break;
        //case 4:uartnum = huart4;break;
        //case 5:uartnum = huart5;break;
        //case 6:uartnum = huart6;break;
        default:break;
      }
     return uartnum;
}

///@brief  根据uart_index返回对应串口中断号
static IRQn_Type get_uartIRQ(u_int8_t uart_index)
{
      IRQn_Type uart_irq_num;
      
      switch(uart_index){
        //case 0:uart_irq_num = LPUART1_IRQn;break;
        case 1:uart_irq_num = USART1_IRQn;break;
        case 2:uart_irq_num = USART2_IRQn;break;
        case 3:uart_irq_num = USART3_IRQn;break;
        case 4:uart_irq_num = UART4_IRQn;break;
        //case 5:uart_irq_num = UART5_IRQn;break;
        //case 6:uartnum = huart6;break;
        default:break;
      }
     return uart_irq_num;
}


///@brief  关闭串口
void uart_close(u_int8_t uart_index)
{     
  UART_HandleTypeDef uartnum;
  uartnum = get_uartHandTypeDef(uart_index);
  HAL_UART_MspDeInit(&uartnum);
}

///@brief  使能串口中断
void uart_enable_irq(u_int8_t uart_index)
{     
  IRQn_Type uart_irq_num;
  uart_irq_num = get_uartIRQ(uart_index);
  HAL_NVIC_EnableIRQ(uart_irq_num);
}

///@brief  失能串口中断
void uart_disable_irq(u_int8_t uart_index)
{
  IRQn_Type uart_irq_num;
  uart_irq_num = get_uartIRQ(uart_index);
  HAL_NVIC_DisableIRQ(uart_irq_num);
}

///@brief  发送一个字节.
static u_int8_t uart_putchar(device_handle device, char ch)
{  
   UART_HandleTypeDef uartnum;
   int ret=HAL_ERROR;
   
   uart_device_info_t *device_info = (uart_device_info_t *)device; 
   if((!device) || (!device_info->init_status))
      return 0;
    
    uartnum = get_uartHandTypeDef(device_info->uart_index);
    ret = HAL_UART_Transmit(&uartnum,(uint8_t*)&ch,1,10);
    if(ret == HAL_OK){
       return 1;
    }else{
       return 0;
    }
}
///@brief  发送字符串.
#define  ASCII_CHAR_BACKSPACE                           0x08    /* '\b' */
#define  ASCII_CHAR_CHARACTER_TABULATION                0x09    /* '\t' */
#define  ASCII_CHAR_LINE_FEED                           0x0A    /* '\n' */
#define  ASCII_CHAR_LINE_TABULATION                     0x0B    /* '\v' */
#define  ASCII_CHAR_FORM_FEED                           0x0C    /* '\f' */
#define  ASCII_CHAR_CARRIAGE_RETURN                     0x0D    /* '\r' */ 
void  uart_sendstr(device_handle device,char *str)
{    
    while ((*str) != (char )0u) {
        if (*str == ASCII_CHAR_LINE_FEED){
            uart_putchar(device, (ASCII_CHAR_CARRIAGE_RETURN));
            uart_putchar(device, ASCII_CHAR_LINE_FEED);
            str++;
        }else{
            uart_putchar(device, *str++);
        }
    }
}

///@brief  串口多字节发送.
void uart_dev_write(device_handle device, void *data, int len)
{  
    uart_device_info_t *dev = (uart_device_info_t *)device;
    rt_sem_take(dev->ser_mutex, RT_WAITING_FOREVER);  
    for (int i = 0; i < len; i++){
        uart_putchar(device, ((u_int8_t *)data)[i]);
    }
    rt_sem_release(dev->ser_mutex);
}

///@brief  判断串口设备循环buff是否有数据.
int uart_dev_char_present(device_handle device)
{
  uart_device_info_t *device_info = (uart_device_info_t *)device;

  if((!device) || (!device_info->init_status))
    return 0;
  
  return !RingQueueEmpty(&device_info->uart_ring_queue);
}

///@brief  从串口设备循环buff读取一个数据.
char uart_dev_in_char(device_handle device)
{
  uart_device_info_t *device_info = (uart_device_info_t *)device;
  char c = 0;

  if (uart_dev_char_present(device))
      OutRingQueue(&device_info->uart_ring_queue, (u_int8_t*)&c);
  return c;
}

///@brief  判断循环buff是否有数据.
int ring_queue_dev_char_present(RingQueue *ring_queue)
{
    return !RingQueueEmpty(ring_queue);
}

///@brief  从循环buff读取一个数据.
char ring_queue_dev_in_char(RingQueue *ring_queue)
{
  char c = 0;
  if (ring_queue_dev_char_present(ring_queue)) 
      OutRingQueue(ring_queue, (u_int8_t*)&c);
  return c;
}

///@brief  清循环buff数据
int clean_data_buff(device_handle device)
{
    uart_device_info_t *uart_dev = (uart_device_info_t *)device;
    AssertReturnNoPrint(uart_dev_char_present(device), return 0);
    int offset;
    char c;
    for (offset = 0; offset < 1024; offset++) {
        if (!RingQueueEmpty(&uart_dev->uart_ring_queue)){
           OutRingQueue(&uart_dev->uart_ring_queue,(uint8_t*)&c); 
        }
    }
    return 0;
}

///@brief rtt console调试串口输出
void rt_hw_console_output(const char *str)
{   
    uart_device_info_t *device = (uart_device_info_t *)g_console_uart_handle;
    if((!device) || (!device->init_status))
          return ;
    rt_sem_take(device->ser_mutex, RT_WAITING_FOREVER);   
    uart_sendstr(g_console_uart_handle, (char *)str);
    rt_sem_release(device->ser_mutex);     
}

///@brief rtt console调试串口输入
char rt_hw_console_getchar()
{   
     char c = -1;
     uart_device_info_t *uart_dev = (uart_device_info_t *)g_console_uart_handle;
     rt_sem_take(uart_dev->sem_recv_data, RT_WAITING_FOREVER);   
     c = uart_dev_in_char(g_console_uart_handle);
    
     return c;       
}

///@brief  调试串口初始化
void init_console_uart()
{
    g_console_uart_handle = uart_dev_init(CONSOLE_UART_INDEX, CONSOLE_UART_BAUDRATE);
}

///@brief  LTE1串口初始化 
void init_mcu_lte1_uart()
{    
    g_mcu_lte1_uart_handle = uart_dev_init(MCU_LTE1_UART_INDEX, MCU_LTE1_UART_BAUDRATE);   
}

///@brief  LTE2串口初始化
void init_mcu_lte2_uart()
{
    g_mcu_lte2_uart_handle = uart_dev_init(MCU_LTE2_UART_INDEX, MCU_LTE2_UART_BAUDRATE);
}

///@brief  RS485串口初始化
void init_RS485_uart()
{
    g_rs485_uart_handle = uart_dev_init(RS485_UART_INDEX, RS485_UART_BAUDRATE);
}

///@brief  RS232串口初始化
void init_RS232_uart()
{
    g_RS232_uart_handle = uart_dev_init(RT232_UART_INDEX, RS232_UART_BAUDRATE);
}

