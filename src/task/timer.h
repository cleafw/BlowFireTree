//
// 定时器模块头文件
//

#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

// 注意：定时器间隔由 GData.h 中的 TIMEOUT_MS 定义

// 定义回调函数类型
typedef void (*TimerCallback)(void);

// 定时器初始化函数
// 参数: timeout_ms - 超时时间（毫秒）
//       callback - 定时器触发时的回调函数
void timer_task_init(uint32_t timeout_ms, TimerCallback callback);

// 停止定时器
void timer_stop();

// 启动定时器
void timer_start();

// 重启定时器（停止后重新启动）
void timer_restart();

#endif // TIMER_H