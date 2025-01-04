#ifndef _I1_GW2015_PROTOCOL_H
#define _I1_GW2015_PROTOCOL_H

#include "stdint.h"
/* 国网输电线路状态监测装置（2015.3.20 最后修改）应用层数据传输规约 */
#define I1GW2015PROTOCOL_VERSION            0x01000101
/* 帧类型表 */
typedef enum {
  GW2015_FRAME_TYPE_01H_MONITOR_DATA_REPORT = 0x01,  /* 监测数据报（监测装置->上位机）     */
  GW2015_FRAME_TYPE_02H_MONITOR_DATA_RESP   = 0x02,  /* 数据响应报（上位机->监测装置）     */
  GW2015_FRAME_TYPE_03H_CONTROL_DATA_REPORT = 0x03,  /* 控制数据报（上位机->监测装置）     */
  GW2015_FRAME_TYPE_04H_CONTROL_DATA_RESP   = 0x04,  /* 控制响应报（监测装置->上位机）     */
  GW2015_FRAME_TYPE_05H_PIC_DATA_REPORT     = 0x05,  /* 图像数据报（监测装置->上位机）     */
  GW2015_FRAME_TYPE_06H_PIC_DATA_RESP       = 0x06,  /* 图像数据响应报（上位机->监测装置） */
  GW2015_FRAME_TYPE_07H_PIC_CONTROL_REPORT  = 0x07,  /* 图像控制报（上位机->监测装置       */
  GW2015_FRAME_TYPE_08H_PIC_CONTROL_RESP    = 0x08,  /* 图像控制响应报（监测装置->上位机） */
  GW2015_FRAME_TYPE_09H_WORK_STATUS_REPORT  = 0x09,  /* 工作状态报（监测装置->上位机）     */
  GW2015_FRAME_TYPE_0AH_WORK_STATUS_RESP    = 0x0A,  /* 工作状态响应报（上位机->监测装置） */
}Gw2015FrameType_e;

/* 报文类型表 */
typedef enum {
  /* 监测数据报（0x01~0x0F） */
  GW2015_PACK_TYPE_01H_WEATHER_DATA    = 0x01,  /* 气象环境类数据报 */
  GW2015_PACK_TYPE_02H_TOWER_INIC_DATA = 0x02,  /* 杆塔倾斜数据报   */
  GW2015_PACK_TYPE_03H_BREEZE_VIB_DATA = 0x03,  /* 导地线微风振动特征量数据报 */
  GW2015_PACK_TYPE_04H_BREEZE_VIB_WAVE = 0x04,  /* 导地线微风振动波形信号数据报 */
  GW2015_PACK_TYPE_05H_WIRE_SAG_DATA   = 0x05,  /* 导线弧垂数据报 */
  GW2015_PACK_TYPE_06H_WIRE_TEMP_DATA  = 0x06,  /* 导线温度数据报 */
  GW2015_PACK_TYPE_07H_ICING_DATA      = 0x07,  /* 覆冰及不均衡张力差数据报 */
  GW2015_PACK_TYPE_08H_WIRE_WIND_YAW   = 0x08,  /* 导线风偏数据报 */
  GW2015_PACK_TYPE_09H_WIRE_WAVE_DATA  = 0x09,  /* 导线舞动特征量数据报 */
  GW2015_PACK_TYPE_0AH_WIRE_TRACK_DATA = 0x0A,  /* 导线舞动轨迹数据报 */
  GW2015_PACK_TYPE_0BH_DIRTY_DATA      = 0x0B,  /* 现场污秽度数据报 */
                                                /* 0x0C ~ 0x0F新型数据报预留字段*/
  /* 控制数据报（0xA1~0xAF） */
  GW2015_PACK_TYPE_A1H_NIC_QUERY_SET   = 0xA1,  /* 状态监测装置网络适配器查询/设置 */
  GW2015_PACK_TYPE_A2H_HISTORY_DATA    = 0xA2,  /* 上级设备请求状态监测装置历史数据 */
  GW2015_PACK_TYPE_A3H_SAMPLE_PERIOD   = 0xA3,  /* 状态监测装置采样周期查询/设置 */
  GW2015_PACK_TYPE_A4H_UPPER_QUERY_SET = 0xA4,  /* 状态监测装置指向上位机的信息查询/设置 */
  GW2015_PACK_TYPE_A5H_ID_QUERY_SET    = 0xA5,  /* 状态监测装置ID查询/设置 */
  GW2015_PACK_TYPE_A6H_DEV_RESET       = 0xA6,  /* 状态监测装置复位 */
  GW2015_PACK_TYPE_A7H_CFG_QUERY_SET   = 0xA7,  /* 状态监测装置模型参数配置信息查询/设置 */
                                                /* 0xA8~0xAF 控制报预留字段 */
  /* 远程图像数据报（0xB1~0xBF) */
  GW2015_PACK_TYPE_B1H_PIC_SAMPLE_SET  = 0xB1,  /* 图像采集参数设置 */
  GW2015_PACK_TYPE_B2H_PIC_TIME_SET    = 0xB2,  /* 拍照时间表设置 */
  GW2015_PACK_TYPE_B3H_MANUAL_REQ_PIC  = 0xB3,  /* 手动请求拍摄照片 */
  GW2015_PACK_TYPE_B4H_REQ_SEND_PIC    = 0xB4,  /* 采集装置请求上送照片 */
  GW2015_PACK_TYPE_B5H_LONG_PIC_DATA   = 0xB5,  /* 远程图像数据报 */
  GW2015_PACK_TYPE_B6H_LONG_PIC_FINISH = 0xB6,  /* 远程图像数据上送结束标记 */
  GW2015_PACK_TYPE_B7H_LONG_PIC_PATCH  = 0xB7,  /* 远程图像补包数据下发 */
  GW2015_PACK_TYPE_B8H_VID_LONG_ADJUST = 0xB8,  /* 摄像机远程调节 */
                                                /* 0xB9~0xBF远程图像数据报预留字段 */
  /* 工作状态数据报（0xC1~0xCF）*/
  GW2015_PACK_TYPE_C1H_HEARTBEAT_DATA  = 0xC1,  /* 心跳数据报 */
  GW2015_PACK_TYPE_C2H_FAULT_MESSAGE   = 0xC2,  /* 故障信息报 */
}Gw2015PackType_e;

#define GW2015_RECV_FRAME_BEGIN_FLAG 0X5AA5
#define GW2015_FRAME_BEGIN_FLAG  0XA55A
#define GW2015_FRAME_END_FLAG    0x96

#pragma pack(push,1)
typedef struct _GW2015FrameHeader{
  uint16_t Sync;
  uint16_t Packet_Length;
  uint8_t  CMD_ID[17];
  uint8_t  Frame_Type;
  uint8_t  Packet_Type;
  uint8_t  Frame_No;
  uint8_t  data[2];
}GW2015FrameHeader_t;
#define GW2015_PACK_HEAD_SIZE(x)  (((char*)(x)->data) - (char*)(x))
#define GW2015_PACK_SIZE(x)       ((char*)&((x)->data[(x)->Packet_Length]) - (char*)(x) + sizeof(uint16_t) + sizeof(uint8_t))
#define GW2015_PACK_CRC16(x)      (*(uint16_t *)(&((x)->data[(x)->Packet_Length])))

/* 气象数据报内容 */
typedef struct _GW2015WeatherDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Average_WindSpeed_10min;      /* 10 分钟平均风速（浮点数，精确到小数点后 1 位，单位：m/s） */
  uint16_t Average_WindDirection_10min;  /* 10 分钟平均风向（无符号整数，单位：°） */
  float    Max_WindSpeed;                /* 最大风速（浮点数，精确到小数点后 1 位，单位：m/s） */
  float    Extreme_WindSpeed;            /* 极大风速（浮点数，精确到小数点后 1 位，单位：m/s） */
  float    Standard_WindSpeed;           /* 标准风速（利用对数风廓线转换到标准状态的风速，浮点数，精确到小数点后 1 位，单位：m/s） */
  float    Air_Temperature;              /* 气温（浮点数，精确到小数点后 1 位，单位：℃） */
  uint16_t Humidity;                     /* 湿度（无符号整数，单位：%RH） */
  float    Air_Pressure;                 /* 气压（浮点数，精确到小数点后 1 位，单位：hPa） */
  float    Precipitation;                /* 降雨量（浮点数，精确到小数点后 1 位，单位：mm） */
  float    Precipitation_Intensity;      /* 降水强度（浮点数，精确到小数点后 1 位，单位：mm/min）*/
  uint16_t Radiation_Intensity;          /* 光辐射强度（无符号整数，单位：W/m2） */
}GW2015WeatherDataPack_t;

/* 杆塔倾斜数据报内容 */
typedef struct _GW2015TowerIncliDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Inclination;                  /* 倾斜度（浮点数，精确到小数点后 1 位，单位：mm/m）*/
  float    Inclination_X;                /* 顺线倾斜度（浮点数，精确到小数点后 1 位，单位：mm/m）*/
  float    Inclination_Y;                /* 横向倾斜度（浮点数，精确到小数点后 1 位，单位：mm/m）*/
  float    Angle_X;                      /* 顺线倾斜角（浮点数，精确到小数点后 2 位，单位：°）*/
  float    Angle_Y;                      /* 横向倾斜角（浮点数，精确到小数点后 2 位，单位：°）*/
}GW2015TowerIncliDataPack_t;

/* 微风振动特征量数据报内容 */
typedef struct _GW2015BreezeVibDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint8_t  Unit_Sum;                     /* 采集单元总数（无符号整数，取值范围：大于0）*/
  uint8_t  Unit_No;                      /* 采集单元序号（无符号整数，取值范围：大于等于 0）*/
  uint32_t Time_Stamp;                   /* 采集时间 */
  uint16_t Strain_Amplitude;             /* 动弯应变幅值（无符号整数，单位：µε）*/
  float    Bending_Amplitude;            /* 弯曲振幅（浮点数，精确到小数点后 3 位，单位：mm）*/
  float    Vibration_Frequency;          /* 微风振动频率（浮点数，精确到小数点后 2 位，单位：Hz）*/
}GW2015BreezeVibDataPack_t;

/* 微风振动波形信号数据报内容 */
typedef struct _GW2015BreezeVibWavePack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint8_t  Unit_Sum;                     /* 采集单元总数（无符号整数，取值范围：大于0）*/
  uint8_t  Unit_No;                      /* 采集单元序号（无符号整数，取值范围：大于等于 0）*/
  uint32_t Time_Stamp;                   /* 采集时间 */
  uint8_t  SamplePack_Sum;               /* 数据拆包总数（无符号整数，取值范围：大于 0）*/
  uint8_t  SamplePack_No;                /* 数据报包序（无符号整数，取值范围：大于 0）*/
  uint8_t  Strain_Data[0];               /* 微风振动信号（无符号整数，单位：µε）*/
}GW2015BreezeVibWavePack_t;

/* 导线弧垂数据报内容 */
typedef struct _GW2015WireSagDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Conductor_Sag;                /* 导线弧垂（浮点数，精确到小数点后 3 位，单位：m）*/
  float    Toground_Distance;            /* 导线对地距离（浮点数，精确到小数点后 3 位，单位：m）*/
  float    Angle;                        /* 线夹出口处导线切线与水平线夹角（浮点数，精确到小数点后 2 位，单位：°）*/
  uint8_t  Measure_Flag;                 /* 测量法标识：①0x00 直接法；  ②0x01 间接法 */
}GW2015WireSagDataPack_t;

/* 导线温度数据报内容 */
typedef struct _GW2015LineTempDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint8_t  Unit_Sum;                     /* 采集单元总数（无符号整数，取值范围：大于0）*/
  uint8_t  Unit_No;                      /* 采集单元序号（无符号整数，取值范围：大于等于 0）*/
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Line_Temperature;             /* 线温（浮点数，精确到小数点后 1 位，单位：℃）*/
}GW2015LineTempDataPack_t;

/* 覆冰及不均衡张力差数据报内容 */
typedef struct _GW2015IceTensionDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Equal_IceThickness;           /* 等值覆冰厚度（浮点数，精确到小数点后 1 位，单位：mm）*/
  float    Tension;                      /* 综合悬挂载荷（浮点数，精确到小数点后 1 位，单位：N）*/
  float    Tension_Difference;           /* 不均衡张力差（浮点数，精确到小数点后 1 位，单位：N）*/
  uint8_t  T_Sensor_Num;                 /* 安装的（拉力）传感器个数 */
  float    Original_Tension1;            /* 第 1 个传感器的原始拉力值（浮点数，精确到小数点后 1 位，单位：N）*/
  float    Windage_Yaw_Angle1;           /* 第 1 个传感器的绝缘子串风偏角（浮点数，精确到小数点后 2 位，单位：°）*/ 
  float    Deflection_Angle1;            /* 第 1 个传感器的绝缘子串偏斜角（浮点数，精确到小数点后 2 位，单位：°）*/
  float    Original_Tension2;            /* 第 2 个传感器的原始拉力值（浮点数，精确到小数点后 1 位，单位：N）*/
  float    Windage_Yaw_Angle2;           /* 第 2 个传感器的绝缘子串风偏角（浮点数，精确到小数点后 2 位，单位：°）*/ 
  float    Deflection_Angle2;            /* 第 2 个传感器的绝缘子串偏斜角（浮点数，精确到小数点后 2 位，单位：°）*/
}GW2015IceTensionDataPack_t;

/* 风偏数据报内容 */
typedef struct _GW2015WindageYawDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Windage_Yaw_Angle;            /* 风偏角（浮点数，精确到小数点后 2 位，单位：°）*/
  float    Deflection_Angle;             /* 偏斜角（浮点数，精确到小数点后 2 位，单位：°）*/
  float    Least_Clearance;              /* 最小电气间隙（浮点数，精确到小数点后 3 位，单位：m）*/
}GW2015WindageYawDataPack_t;

/* 导线舞动数据报 */
typedef struct _GW2015AmplitudeDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint8_t  Unit_Sum;                     /* 采集单元总数（无符号整数，取值范围：大于0）*/
  uint8_t  Unit_No;                      /* 采集单元序号（无符号整数，取值范围：大于等于 0）*/
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    U_Gallop_Amplitude;           /* 舞动幅值（浮点数，精确到小数点后 3 位，单位：m）*/
  float    U_Vertical_Amplitude;         /* 垂直舞动幅值（浮点数，精确到小数点后 3 位，单位：m）*/
  float    U_Horizontal_Amplitude;       /* 水平舞动幅值（浮点数，精确到小数点后 3 位，单位：m）*/
  float    U_AngleToVertical;            /* 舞动椭圆倾斜角（浮点数，精确到小数点后 2位，单位：°）*/
  float    U_Gallop_Frequency;           /* 舞动频率（浮点数，精确到小数点后 2 位，单位：Hz）*/
}GW2015AmplitudeDataPack_t;

/* 导线舞动轨迹数据报内容 */
typedef struct _GW2015AmpTrackDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint8_t  Unit_Sum;                     /* 采集单元总数（无符号整数，取值范围：大于0）*/
  uint8_t  Unit_No;                      /* 采集单元序号（无符号整数，取值范围：大于等于 0）*/
  uint32_t Time_Stamp;                   /* 采集时间 */
  uint8_t  SamplePack_Sum;               /* 数据拆包总数（无符号整数，取值范围：大于0）*/
  uint8_t  SamplePack_No;                /* 数据报包序（无符号整数，取值范围：大于 0）*/
  float    Displacement[0];              /* 字段采用以下结构：X 方向相对位移坐标（4Byte）+Y 方向相对位移坐标（4Byte）+Z 方向相对位移坐标（4Byte）（位移坐标为浮点数，精确到小数点后 3 位，单位：m）*/
}GW2015AmpTrackDataPack_t;

/* 现场污秽度数据报 */
typedef struct _GW2015DirtyDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    ESDD;                         /* 等值附盐密度，即盐密（浮点数，精确到小数点后 3位，单位：mg/cm2）*/
  float    NSDD;                         /* 不溶物密度，即灰密（浮点数，精确到小数点后 3 位，单位：mg/cm2）*/
  float    Daily_Max_Temperature;        /* 日最高温度（浮点数，精确到小数点后 1 位，单位：℃）*/
  float    Daily_Min_Temperature;        /* 日最低温度（浮点数，精确到小数点后 1 位，单位：℃）*/
  float    Daily_Max_Humidity;           /* 日最大湿度（无符号整数，单位：％RH）*/
  float    Daily_Min_Humidity;           /* 日最小湿度（无符号整数，单位：％RH）*/
}GW2015DirtyDataPack_t;

/* 状态监测装置采样周期查询/设置 */
typedef struct _GW2015ConllectInterval {
  uint8_t  Request_Set_Flag;              /* 参数配置类型标志， 0x00-查询配置信息，0x01-设置配置信息 */ 
  uint8_t  Request_Flag;                  /* 标识位，增加参数选择 */
  uint8_t  Request_Type;                  /* 采样的数据类型 */
  uint16_t Main_Time;                     /* 采集时间周期 */
  uint8_t  Heartbeat_Time;                /* 心跳上送周期 */
}GW2015ConllectInterval_t;

typedef struct _GW2015ConllectIntervalResp {
  uint8_t  Request_Set_Flag;              /* 参数配置类型标志， 0x00-查询配置信息，0x01-设置配置信息 */ 
  uint8_t  Command_Status;
  uint8_t  Request_Flag;                  /* 标识位，增加参数选择 */
  uint8_t  Request_Type;                  /* 采样的数据类型 */
  uint16_t Main_Time;                     /* 采集时间周期 */
  uint8_t  Heartbeat_Time;                /* 心跳上送周期 */
}GW2015ConllectIntervalResp_t;

/* 心跳数据报 */
typedef struct _GW2015HeartbeatDataPack {
  uint32_t Time_Stamp;                   /* 当前时间 */
  float    Battery_Voltage;              /* 电源电压（浮点数，精确到小数点后 1 位，单位：V） */
  float    Operation_Temperature;        /* 工作温度（浮点数，精确到小数点后 1 位，单位：℃） */
  float    Battery_Capacity;             /* 电池剩余电量（浮点数，精确到小数点后 1 位，单位：Ah）*/
  uint8_t  FloatingCharge;               /* 浮充状态：①0x00 充电  ②0x01 放电 */
  uint32_t Total_Working_Time;           /* 工作总时间（无符号整数，单位：小时） */
  uint32_t Working_Time;                 /* 本次连续工作时间（无符号整数，单位：小时） */
  uint8_t  Connection_State;             /* 连接状态：①0x00 与所有传感器连接正常 ②0x01 与一个或者多个传感器连接异常 */
  uint32_t Send_Flow;                    /* 当月发送流量（无符号整数，单位：字节） */
  uint32_t Receive_Flow;                 /* 当月接收流量（无符号整数，单位：字节）*/
  uint32_t Protocol_Version;             /* 通信协议版本号（带小数位）四个部分，每个字节代表一段；举例：版本号 1.2.4.10 的 4 个字节表示为：01 02 04 0A*/
}GW2015HeartbeatDataPack_t;
//心跳数据响应报（上位机->监测装置）
typedef struct _GW2015HeartbeatResp {
  uint8_t  Command_Status;               /* 数据发送状态 ①0xFF 成功,②0x00 失败 */
  uint8_t  Mode;                         /* 运行模式 ①0x00 切换到正常模式 ②0x01 切换到调试模式 */
  uint32_t Clocktime_Stamp;              /* 上位机当前时间（世纪秒，当值为 0 时，表示装置时间与上位机时间一致；当值非 0 时，装置需要根据该时间进行校时）*/
}GW2015HeartbeatResp_t;

/* 装置故障信息报 */
typedef struct _GW2015DevFaultDesc {
  uint32_t Time_Stamp;                   /* 当前时间 */
  char     fault_desc[30];               /* 故障信息 */
}GW2015DevFaultDesc_t;

/* 根据输电I1接口规约修改的自定义协议  */
/* 帧类型表 */
typedef enum {
  DS_FRAME_TYPE_20H_CMD_DATA_REPORT      = 0x20,   /* 数据指令报（后台→边缘终端）*/
  DS_FRAME_TYPE_21H_RESPONSE_DATA_REPORT = 0x21,   /* 数据响应报（边缘终端→后台）*/
}DSFrameType_e;

/* 报文类型表 */
typedef enum {
  DS_PACK_TYPE_D1H_OTA_UPGRADE_INFORM   = 0xD1,   /* OTA升级通知 */
  DS_PACK_TYPE_D2H_OTA_UPGRADE_DATA     = 0xD2,   /* OTA升级数据请求及回复 */
  DS_PACK_TYPE_D3H_OTA_UPGRADE_FINISH   = 0xD3,   /* OTA升级结束 */
  DS_PACK_TYPE_D4H_INQUIRE_DEV_VERSION  = 0xD4,   /* 查询设备的软件&硬件版本信息 */
  DS_PACK_TYPE_D5H_DEV_WORK_STATUS_DATA = 0xD5,   /* 设备状态数据报 */
  DS_PACK_TYPE_D6H_SIM_CCID_DATA        = 0xD6,   /* 查询SIM卡CCID */

  DS_PACK_TYPE_0CH_DEVICE_RTK_SAG_DATA  = 0x0C,   /* rtk弧垂数据 */
}DSPackType_e;

/* TLV数据格式 */
typedef struct _TLVDataStruct {
  uint8_t  Type;
  uint16_t lenth;
  uint8_t  data[2];
}TLVData_t;

/* OTA升级通知 */
typedef struct _DSOTAUpgradeInform {
  uint32_t Flag;                         /* 升级通知标志"DXFF" */
  uint8_t  File_Type;                    /* 升级文件类型（1：App，2：Bootloader，3:配置文件，4:密钥，5:加密证书）*/
  char     Software_Version[9];          /* 软件版本 举例：版本号 Sv0102010  表示为：表示的版本号为：Sv1.2.10 */
  char     Hardware_Version[9];          /* 硬件版本 举例：版本号 Hv0102010  表示为：表示的版本号为：Hv1.2.10 */
  uint32_t File_Length;                  /* 升级文件总长度 */
  uint16_t File_Frame_Len;               /* 升级文件每包长度(默认512Byte，可配)*/
  uint16_t File_Frame_Num;               /* 升级文件分包个数 */ 
  uint16_t File_CRC;                     /* 升级文件的校验 */
}DSOTAUpgradeInform_t;

/* OTA升级数据报文 */
typedef struct _DSOTAUpgradeData {
  uint16_t File_Frame_Index;             /* 升级文件帧的序号 */
  uint16_t File_Frame_Length;            /* 升级文件帧的数据长度 */
  uint8_t File_Frame_Data[2];            /* 升级文件的数据内容 */
}DSOTAUpgradeData_t;

/* OTA升级结束报文 */
typedef struct _DSOTAUpgradeFinish {
  uint8_t  Flag;                         /* 数据内容：0xFF：设备升级成功, 0x00：设备升级失败*/
  char     Software_Version[9];          /* 软件版本 */
  char     Hardware_Version[9];          /* 硬件版本 */
}DSOTAUpgradeFinish_t;

/* 获取SIM CCID */
typedef struct _DSDeviceSIMCCIDPack {
  char     SIM1_CCID[20];                /* SIM1_CCID */
  char     SIM2_CCID[20];                /* SIM2_CCID */
}DSDeviceSIMCCIDPack_t;

/* 设备状态数据 */
typedef struct _DSDeviceBatteryPack {
  uint32_t Time_Stamp;                   /* 当前时间 */
  float    Battery_Voltage;              /* 电源电压（浮点数，精确到小数点后 1 位，单位：V） */
  uint8_t  FloatingCharge;               /* 浮充状态：①0x00 充电  ②0x01 放电 */
  uint8_t  Charge_Type;                  /* 充电类型：充电类型：①0x00太阳能 ②0x01 电场取能 */
  float    Open_voltage;                 /* 开路电压（浮点数，精确到小数点后 2 位，单位：V）*/
  float    Charge_Current;               /* 充电电流（浮点数，精确到小数点后 2 位，单位：A）*/
  float    Operation_Temperature;        /* 电池温度（浮点数，精确到小数点后 1 位，单位：℃） */
  float    Battery_Capacity;             /* 电池剩余电量（浮点数，精确到小数点后 1 位，单位：Ah）*/
}DSDeviceBatteryPack_t;

/* 导线弧垂数据报内容 */
typedef struct _DSRTKWireSagDataPack {
  uint8_t  Componet_ID[17];              /* 被监测设备 ID（17 位编码） */
  uint32_t Time_Stamp;                   /* 采集时间 */
  float    Longitude;                    /* 经度（浮点数，精确到小数点后7位）*/
  float    Latitude;                     /* 维度（浮点数，精确到小数点后7位） */
  float    Altitude;                     /* 高程（浮点数，精确到小数点后2位，单位：米）*/
}DSRTKWireSagDataPack_t;

#pragma pack(pop)
#endif