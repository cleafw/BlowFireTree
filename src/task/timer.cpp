//
// 定时器模块实现 - ESP32-S3/C3 兼容版
//

#include "timer.h"

// 定时器句柄
hw_timer_t *timer = NULL;

// 回调函数指针
static TimerCallback timerCallback = NULL;

// 定时器周期（微秒）
static uint32_t timerPeriodUs = 1000000;

// 定时器中断服务函数
void IRAM_ATTR onTimer() {
    // 如果注册了回调函数，则调用
    if (timerCallback != NULL) {
        timerCallback();
    }
}

// 定时器任务初始化
void timer_task_init(uint32_t timeout_ms, TimerCallback callback) {
    // 保存回调函数
    timerCallback = callback;

    // 计算定时器周期（毫秒转微秒）
    timerPeriodUs = timeout_ms * 1000;

    // 初始化定时器
    // 参数说明:
    //   - 定时器编号: 0 (ESP32-C3 有 0-1, ESP32-S3 有 0-3)
    //   - 分频器: 80 (APB时钟 80MHz / 80 = 1MHz)
    //   - 计数方向: true (向上计数)
    //
    // 结果: 定时器频率 = 1MHz, 每个计数 = 1微秒

    timer = timerBegin(0, 80, true);

    // 检查初始化是否成功
    if (timer == NULL) {
        Serial.println("错误: 定时器初始化失败!");
        return;
    }

    // 绑定中断函数
    timerAttachInterrupt(timer, &onTimer, true);

    // 设置定时器周期（微秒）
    timerAlarmWrite(timer, timerPeriodUs, true);

    Serial.printf("定时器已初始化: %d us (%d ms)\n", timerPeriodUs, timeout_ms);
}

// 停止定时器
void timer_stop() {
    if (timer != NULL) {
        timerAlarmDisable(timer);
    }
}

// 启动定时器
void timer_start() {
    if (timer != NULL) {
        timerAlarmEnable(timer);
    }
}

// 重启定时器
void timer_restart() {
    if (timer != NULL) {
        timerAlarmDisable(timer);
        timerWrite(timer, 0);  // 重置计数器到0
        timerAlarmEnable(timer);
    }
}