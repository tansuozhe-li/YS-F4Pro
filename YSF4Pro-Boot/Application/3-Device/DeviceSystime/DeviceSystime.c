/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceSystime.c
*  内容摘要：  微气象模块主要功能实现
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年11月21日
*  
*  修改记录 1：
*        修改日期：2023年11月21日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "DeviceSystime.h"
#include "commlib.h"
/* 定时器的控制块 */
static rt_timer_t systime;
/* 指向信号量的指针 */
static rt_sem_t systime_sem = RT_NULL;
/* 指向互斥量的指针 */
static rt_mutex_t systime_mutex = RT_NULL;
//默认初始时间23年8月8日8时8分8秒
sys_time_t system_time = {8,8,8,8,8,2023};

sys_work_time_t work_time;

/* 平年月份 */
static const uint32_t g_NonLeapYearMonths[12] =
{
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
};

/* 闰年月份 */
static const uint32_t g_LeapYearMonth[12] =
{
    31,
    31 + 29,
    31 + 29 + 31,
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
};

///@brief   判断某年是平年还是闰年
///@return  闰年 - 1；平年 - 0
static uint8_t TimeCovertIsLeapYear(uint32_t year)
{
    /* 能被4整除,不能被百整除,能被400整除 */
    if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
    {
        /* 闰年 */
        return 1;
    }
    else
    {
        /* 平年 */
        return 0;
    }
}

///@brief   将UTC时间转换为本地时间
sys_time_t TimeCovertUtcToLocalTime(uint32_t UtcTime)
{
    uint32_t i = 0;
    sys_time_t LocalTime;
    uint32_t Hour,Days,Year;

    /* 得到秒余数 */
    LocalTime.sec = UtcTime%60;
    /* 得到整数分钟数 */
    LocalTime.min = (UtcTime/60)%60;

    /* 得到整数小时数 */
    Hour = (UtcTime/60)/60;
    /* 得到小时余数+时区 */
    LocalTime.hour = Hour%24+LOCAL_TIME_ZONE;
    if(LocalTime.hour>23)
    {
        LocalTime.hour-=24;
        Days=Hour/24+1;
    }
    else
    {
        Days=Hour/24;
    }
    /* 计算星期,0-表示星期天        注：1970-1-1 是星期4 */
    //LocalTime.week=(Days+4)%7;

    /* 注：400年=146097天,100年=36524天,4年=1461天 */
    Year = 1970; /* utc时间从1970开始 */
    Year += (Days/146097)*400;

    Days %= 146097; /* 计算400年内的剩余天数 */
    Year += (Days/36525)*100;

    Days %= 36525; 
    Year += (Days/1461)*4;

    Days %= 1461; /* 计算4年内剩余天数,1970平1972闰年 */
    while( Days > 365)
    {
        if(TimeCovertIsLeapYear(Year))
        {
            Days--;
        }
        Days -= 365;
        Year++;
    }
    if (!TimeCovertIsLeapYear(Year) && (Days == 365))
    {
        Year++;
        LocalTime.day =1;
        LocalTime.month =1;
        LocalTime.year =Year;
        return LocalTime;
    }
    LocalTime.year =Year;
    LocalTime.month=0;
    LocalTime.day=0;
    if (TimeCovertIsLeapYear(Year))
    {
        /* 本年是闰年 */
        for (i = 0; i < 12; i++)
        {
            if (Days < g_LeapYearMonth[i])
            {
                LocalTime.month = i + 1;
                if (i == 0)
                {
                    LocalTime.day = Days;
                }
                else
                {
                    LocalTime.day = Days - g_LeapYearMonth[i - 1];
                }
                LocalTime.day++;
                return LocalTime;
            }
        }
    }
    else
    {
        /* 本年是平年 */
        for (i = 0; i < 12; i++)
        {
            if (Days < g_NonLeapYearMonths[i])
            {
                LocalTime.month  = i + 1;
                if (i == 0)
                {
                    LocalTime.day = Days;
                }
                else
                {
                    LocalTime.day = Days - g_NonLeapYearMonths[i - 1];
                }
                LocalTime.day++;
                return LocalTime;
            }
        }
    }
    return LocalTime;
}

///@brief   将本地时间转换为UTC时间
uint32_t TIME_CONVERT_LocalTime2Utc(sys_time_t LocalTime)
{
    /* 看一下有几个400年,几个100年,几个4年 */
    uint32_t y = LocalTime.year -1970;
    uint32_t dy = (y / 400);

    /* 400年的天数 */
    uint32_t days = dy * (400 * 365 + 97);

    /* 100年的天数 */
    dy = (y % 400) / 100;
    days += dy * (100 * 365 + 25);

    /* 4年的天数 */
    dy = (y % 100) / 4;
    days += dy * (4 * 365 + 1);

    /* 注意:这里1972是闰年,与1970只差2年 */
    dy = y % 4;
    days += dy * 365;

    /* 这个4年里,有没有闰年就差1天 */
    if(dy == 3)
    {
        /* 只有这个是要手动加天数的,因为1973年计算时前面的天数按365天算,1972少算了一天 */
        days++;
    }

    if (LocalTime.month != 1)
    {
        if(TimeCovertIsLeapYear(LocalTime.year))
        {
            /* 看看今年是闰年还是平年 */
            days += g_LeapYearMonth[(LocalTime.month - 1) - 1];
        }
        else
        {
             /* 如果给定的月份数为x则,只有x-1个整数月 */
            days += g_NonLeapYearMonths[(LocalTime.month  - 1) - 1];
        }
    }
    days += LocalTime.day - 1;

    return (days * 24 * 3600 + ((uint32_t)LocalTime.hour - LOCAL_TIME_ZONE)* 3600 + (uint32_t)LocalTime.min * 60 + (uint32_t)LocalTime.sec);
}


///@brief 获取当前系统时钟的世纪秒
uint32_t get_current_centurysec()
{
    rt_mutex_take(systime_mutex, RT_WAITING_FOREVER);
    work_time.Clocktime_Stamp = TIME_CONVERT_LocalTime2Utc(system_time);
    rt_mutex_release(systime_mutex);
    return work_time.Clocktime_Stamp;
}

///@brief 获取当前的系统时间
void get_systime(sys_time_t *time)
{
    rt_mutex_take(systime_mutex, RT_WAITING_FOREVER);
    time->year = system_time.year;
    time->month = system_time.month;
    time->day = system_time.day;
    time->hour = system_time.hour;
    time->min = system_time.min;
    time->sec = system_time.sec;
    rt_mutex_release(systime_mutex);
}

///@brief 更新系统时间
void update_systime(sys_time_t systime)
{
    rt_mutex_take(systime_mutex,RT_WAITING_FOREVER);
    system_time.year = systime.year;
    system_time.month = systime.month;
    system_time.day = systime.day;
    system_time.hour = systime.hour;
    system_time.min = systime.min;
    system_time.sec = systime.sec ;    
    rt_mutex_release(systime_mutex);
}

BOOL Systime_BroadcastMsgProc_TimingSystime(mq_brodcast_msg *pMsg)
{ 
    sys_time_t sys_time_stamp;
    GW2015HeartbeatResp_t *pResp = (GW2015HeartbeatResp_t *)pMsg->msg_body;
    sys_time_stamp = TimeCovertUtcToLocalTime(pResp->Clocktime_Stamp);
    update_systime(sys_time_stamp);
    LOG_I("当前系统时间：%d/%d/%d %d:%d:%d", system_time.year, system_time.month, system_time.day,
                                  system_time.hour, system_time.min, system_time.sec);
    LOG_I("工作总时间：%d分钟, 本次连续工作时间: %d分钟", work_time.Total_Working_Time, work_time.Working_Time);
    return TRUE;
}

///@brief 系统时钟维护线程
void Systime_run()
{
    static u_int8_t dayInMonthTbl[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    static u_int8_t dayInMonth;
    // 等待系统启动
    while (1)
    {
        rt_sem_take(systime_sem, RT_WAITING_FOREVER);
        if (system_time.month != 2){
            dayInMonth = dayInMonthTbl[system_time.month - 1];
        }else{
            if ((system_time.year%400==0) || (system_time.year%4==0 && system_time.year%100!=0)) {
                dayInMonth = 29;
            } else {
                dayInMonth = 28; 
            }
        }
        system_time.sec++;
        if (system_time.sec >= 60) {
            system_time.sec = 0;
            system_time.min++;
            if (system_time.min >= 60) {
                system_time.min = 0;
                system_time.hour++;
                if (system_time.hour >= 24) {
                    system_time.hour = 0;
                    system_time.day++;
                    if (system_time.day > dayInMonth) {
                        system_time.day = 1;
                        system_time.month++;
                        if (system_time.month > 12){
                            system_time.month = 1;
                            system_time.year++;
                        }
                    } 
                }  
            }
        } 
        work_time.Total_Working_Time++;
        work_time.Working_Time++;
        if ((work_time.Working_Time) > 24 * 3600) {
            EITL_REBOOT();
        } 
        work_time.Clocktime_Stamp = TIME_CONVERT_LocalTime2Utc(system_time);
        DS_SysTime = TIME_CONVERT_LocalTime2Utc(system_time);
        mdelay(5);
    }
}

///@brief   定时器回调函数，周期:1000个os_tick(1s)
static void SysTimeOut(void *parameter)
{
    rt_sem_release(systime_sem);
}

///@brief 初始化模块定时器、信号量
BOOL InitModuleOtherFunc()
{
    DS_SysTime = TIME_CONVERT_LocalTime2Utc(system_time);
    //创建定时器
    systime = rt_timer_create("systime", SysTimeOut, RT_NULL, 1000, RT_TIMER_FLAG_PERIODIC);
    AssertError(systime != RT_NULL, return FALSE, "create thread:%s failed", "systime");
    rt_timer_start(systime);
    
    // 创建一个动态信号量，初始值是 0
    systime_sem = rt_sem_create("systime_sem", 0, RT_IPC_FLAG_FIFO);
    AssertError(systime_sem != RT_NULL, return FALSE, "create systime_sem:%s failed", "systime_sem");
    
    //创建一个动态互斥量
    systime_mutex = rt_mutex_create("systime_mutex", RT_IPC_FLAG_FIFO);
    AssertError(systime_mutex != RT_NULL, return FALSE, "create systime_mutex:%s failed", "systime_mutex");
    
    return 0;
}
