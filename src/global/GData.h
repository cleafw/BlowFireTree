//
// Created by 99081 on 2026/2/6.
//

#ifndef WISHINGTREE_GDATA_H
#define WISHINGTREE_GDATA_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// ==================== 引脚定义 ====================
#define StripLight_PIN_0    D0 // 灯带 0 数据引脚
#define StripLight_PIN_1    D1 // 灯带 1 数据引脚
#define StripLight_PIN_2    D2 // 灯带 2 数据引脚
#define StripLight_PIN_3    D7 // 灯带 3 数据引脚
#define Relay_PIN           D9 // 继电器数字引脚
#define TempHumi_SDA        D4 // 温湿度传感器 SDA
#define TempHumi_SCL        D5 // 温湿度传感器 SCL
#define SOUND_SENSOR_PIN    A8 // 声音传感器引脚


// ==================== 灯带配置 ====================
#define STRIP_COUNT         4       // 灯带数量
#define STRIP_NUM_LEDS      200     // 每条灯带 LED 数量

// 正常状态（常亮）
#define NORMAL_BRIGHTNESS   250     // 正常亮度 (0-255)
#define NORMAL_COLOR_R      255     // 正常颜色 R
#define NORMAL_COLOR_G      255     // 正常颜色 G
#define NORMAL_COLOR_B      51      // 正常颜色 B

// 交互状态（初始很暗）
#define INTERACTIVE_MIN_BRIGHTNESS  250  // 交互模式最低亮度
#define INTERACTIVE_MAX_BRIGHTNESS  250 // 交互模式最高亮度

#define INTERACTIVE_MIN_COLOR_R     255  // 交互模式初始颜色 R（最浅）
#define INTERACTIVE_MIN_COLOR_G     255  // 交互模式初始颜色 G（最浅）
#define INTERACTIVE_MIN_COLOR_B     51   // 交互模式初始颜色 B（最浅）

#define INTERACTIVE_MAX_COLOR_R     0   // 交互模式目标颜色 R（最深）
#define INTERACTIVE_MAX_COLOR_G     255 // 交互模式目标颜色 G（最深）
#define INTERACTIVE_MAX_COLOR_B     0   // 交互模式目标颜色 B（最深）

// ==================== 吹气检测配置 ====================
#define HUMIDITY_THRESHOLD      0.3     // 湿度突然增加阈值（百分比）
#define MAX_BREATH_TRIGGERS     6      // 完全点亮所需的吹气次数
#define TIMEOUT_MS              10000   // 超时时间（5秒）
#define TRANSITION_TIME_MS      500    // 渐变动画时间（毫秒）

// ==================== 温湿度传感器配置 ====================
#define TEMP_HUMI_READ_INTERVAL 300     // 温湿度读取间隔（毫秒）

// ==================== 系统状态 ====================
enum SystemState {
    STATE_NORMAL,       // 正常状态：常亮
    STATE_INTERACTIVE   // 交互状态：根据吹气次数渐变
};

#endif //WISHINGTREE_GDATA_H