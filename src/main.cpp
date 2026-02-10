#include <Arduino.h>
#include "seeed/grove/ws2813_ring.h"
#include "global/GData.h"
#include "global/GObject.h"
#include "user/app.h"

void setup() {
    delay(100);  // 稳定延时

    app_Init(); // 系统初始化
}

void loop() {
    app_Task(); // 灯带任务

}