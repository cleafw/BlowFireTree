//
// Created by 99081 on 2026/2/7.
//

#include "app.h"
#include "task/timer.h"

// ==================== 全局状态变量 ====================
SystemState currentState = STATE_NORMAL;        // 当前系统状态
volatile bool timeoutFlag = false;              // 定时器超时标志
uint8_t breathTriggerCount = 0;                 // 吹气触发计数（0-8）

// 湿度监测变量
float baselineHumidity = 0.0;                   // 基准湿度值（上一次读取的湿度）
unsigned long lastSensorRead = 0;               // 上次读取传感器的时间

// 内部函数声明
void handleBreathDetected();
void enterInteractiveState();
void enterNormalState();
void updateProgress();
void handleTimeout();
void updateBaselineHumidity();
bool checkBreathDetection();

// ==================== 定时器回调 ====================
void timer_callback() {
    Serial.println("[DEBUG] 定时器触发!");
    timeoutFlag = true;  // 仅设置标志位，不做其他操作
}

// ==================== 灯带测试（调试用） ====================
void strips_Test() {
    Serial.println("\n=== 开始灯带测试 ===");

    for (int i = 0; i < STRIP_COUNT; i++) {
        strips[i]->setBrightness(NORMAL_BRIGHTNESS);
    }

    // 红色测试
    Serial.println("测试: 红色");
    for (int i = 0; i < STRIP_COUNT; i++){
        strips[i]->fillColor(255, 0, 0);
        strips[i]->show();
    }
    delay(1000);

    // 绿色测试
    Serial.println("测试: 绿色");
    for (int i = 0; i < STRIP_COUNT; i++){
        strips[i]->fillColor(0, 255, 0);
        strips[i]->show();
    }
    delay(1000);

    // 蓝色测试
    Serial.println("测试: 蓝色");
    for (int i = 0; i < STRIP_COUNT; i++){
        strips[i]->fillColor(0, 0, 255);
        strips[i]->show();
    }
    delay(1000);

    // 逐个点亮测试
    Serial.println("测试: 逐个点亮");
    for(int n = 0; n < STRIP_NUM_LEDS; n++){
        for(int i = 0; i < STRIP_COUNT; i++){
            strips[i]->setPixelColor(n, 255, 255, 255);
            strips[i]->show();
        }
        delay(10);
    }
    delay(1000);

    Serial.println("=== 灯带测试完成 ===\n");
}

// ==================== 温湿度传感器测试 ====================
void tempHumi_Sensor_Test() {
    Serial.println("\n=== 开始温湿度传感器测试 ===");

    for(int i = 0; i < 10; i++) {
        float temp, humi;
        if(sht31Sensor.readTempHumi(temp, humi)) {
            Serial.printf("第 %d 次: 温度: %.2f°C, 湿度: %.2f%%\n", i+1, temp, humi);
        } else {
            Serial.printf("第 %d 次: 读取失败\n", i+1);
        }
        delay(500);
    }

    Serial.println("=== 温湿度传感器测试完成 ===\n");
}

// ==================== 更新基准湿度 ====================
void updateBaselineHumidity() {
    float temp, humi;
    if(sht31Sensor.readTempHumi(temp, humi)) {
        baselineHumidity = humi;
        Serial.printf("[基准更新] 湿度: %.2f%% (温度: %.2f°C)\n", humi, temp);
    } else {
        Serial.println("[警告] 无法读取传感器数据");
    }
}

// ==================== 检测吹气 ====================
// 检测逻辑：
// 1. 每次读取传感器都与上一次读取值比较
// 2. 如果湿度突然增加 >= 阈值，判定为吹气
// 3. 每次读取后都更新基准值，这样可以：
//    - 实时跟踪环境湿度的缓慢变化（温度变化、自然波动）
//    - 只有突然的湿度增加才会被识别为吹气
//    - 吹气后湿度逐渐下降的过程不会误触发
bool checkBreathDetection() {
    unsigned long currentTime = millis();

    // 控制读取频率，避免过于频繁
    if(currentTime - lastSensorRead < TEMP_HUMI_READ_INTERVAL) {
        return false;
    }
    lastSensorRead = currentTime;

    // 读取当前湿度
    float temp, humi;
    if(!sht31Sensor.readTempHumi(temp, humi)) {
        return false;
    }

    // 检测湿度突然增加（相比上一次读取）
    float humidityIncrease = humi - baselineHumidity;

    // 调试输出（可选，取消注释以启用）
    // Serial.printf("[监测] 当前: %.2f%%, 上次: %.2f%%, 变化: %.2f%%\n",
    //               humi, baselineHumidity, humidityIncrease);

    // 判断是否检测到吹气（湿度突然增加）
    if(humidityIncrease >= HUMIDITY_THRESHOLD) {
        Serial.printf(">>> 检测到吹气! 湿度从 %.2f%% 突然增加到 %.2f%% (+%.2f%%)\n",
                      baselineHumidity, humi, humidityIncrease);

        // 更新基准为当前值
        baselineHumidity = humi;
        return true;
    }

    // 每次读取都更新基准值（跟踪环境缓慢变化）
    baselineHumidity = humi;

    return false;
}

// ==================== 处理吹气触发 ====================
void handleBreathDetected() {
    Serial.println("\n>>> 检测到吹气信号");

    // 情况1: 当前在正常状态 → 进入交互状态
    if (currentState == STATE_NORMAL) {
        enterInteractiveState();
        return;
    }

    // 情况2: 当前在交互状态 → 增加计数
    if (currentState == STATE_INTERACTIVE) {
        if (breathTriggerCount < MAX_BREATH_TRIGGERS) {
            breathTriggerCount++;

            // 更新进度（渐变动画，会阻塞TRANSITION_TIME_MS）
            updateProgress();

            // 渐变完成后重启定时器
            timer_restart();
            timeoutFlag = false;

            Serial.printf("定时器已重启，重新计时 %d 秒\n", TIMEOUT_MS / 1000);
        } else {
            Serial.printf("已达到最大次数(%d/%d)，忽略此信号", breathTriggerCount, MAX_BREATH_TRIGGERS);
        }
    }
}

// ==================== 进入交互状态 ====================
void enterInteractiveState() {
    Serial.println("\n========== 进入交互状态 ==========");

    currentState = STATE_INTERACTIVE;
    breathTriggerCount = 0;  // 从0开始，第一次吹气算第1次
    timeoutFlag = false;

    // 更新基准湿度
    updateBaselineHumidity();

    // 立即设置灯带为初始状态（最暗，颜色最浅）
    for (int i = 0; i < STRIP_COUNT; i++) {
        strips[i]->setBrightness(INTERACTIVE_MIN_BRIGHTNESS);
        strips[i]->fillColor(INTERACTIVE_MIN_COLOR_R,
                             INTERACTIVE_MIN_COLOR_G,
                             INTERACTIVE_MIN_COLOR_B);
        strips[i]->show();
    }
    Serial.printf("灯带: 初始状态 (亮度: %d, 颜色: RGB(%d,%d,%d))\n",
                  INTERACTIVE_MIN_BRIGHTNESS,
                  INTERACTIVE_MIN_COLOR_R, INTERACTIVE_MIN_COLOR_G, INTERACTIVE_MIN_COLOR_B);

    // 启动定时器
    timer_restart();
    Serial.printf("定时器: 已启动 (%d 秒倒计时)\n", TIMEOUT_MS / 1000);

    Serial.println("等待吹气信号... (进度: 0/8)");
    Serial.println("=====================================\n");
}

// ==================== 更新进度（渐变效果） ====================
void updateProgress() {
    // 计算目标进度比例 (0.0 - 1.0)
    float targetProgress = (float)breathTriggerCount / (float)MAX_BREATH_TRIGGERS;

    // 计算目标亮度和颜色
    uint8_t targetBrightness = INTERACTIVE_MIN_BRIGHTNESS +
                               (uint8_t)((INTERACTIVE_MAX_BRIGHTNESS - INTERACTIVE_MIN_BRIGHTNESS) * targetProgress);

    // 使用int16_t处理可能的负数差值
    int16_t deltaR = INTERACTIVE_MAX_COLOR_R - INTERACTIVE_MIN_COLOR_R;
    int16_t deltaG = INTERACTIVE_MAX_COLOR_G - INTERACTIVE_MIN_COLOR_G;
    int16_t deltaB = INTERACTIVE_MAX_COLOR_B - INTERACTIVE_MIN_COLOR_B;

    uint8_t targetR = INTERACTIVE_MIN_COLOR_R + (int16_t)(deltaR * targetProgress);
    uint8_t targetG = INTERACTIVE_MIN_COLOR_G + (int16_t)(deltaG * targetProgress);
    uint8_t targetB = INTERACTIVE_MIN_COLOR_B + (int16_t)(deltaB * targetProgress);

    Serial.printf("进度: %d/%d (%.0f%%) → 目标: 亮度=%d, RGB(%d,%d,%d)\n",
                  breathTriggerCount, MAX_BREATH_TRIGGERS, targetProgress * 100,
                  targetBrightness, targetR, targetG, targetB);

    // 获取当前状态（假设所有灯带状态相同，读取第一条）
    // 注意：Adafruit_NeoPixel没有getBrightness，需要自己记录
    static uint8_t currentBrightness = INTERACTIVE_MIN_BRIGHTNESS;
    static uint8_t currentR = INTERACTIVE_MIN_COLOR_R;
    static uint8_t currentG = INTERACTIVE_MIN_COLOR_G;
    static uint8_t currentB = INTERACTIVE_MIN_COLOR_B;

    // 如果是第一次进入交互模式（进度0→1），从初始值开始
    if(breathTriggerCount == 1) {
        currentBrightness = INTERACTIVE_MIN_BRIGHTNESS;
        currentR = INTERACTIVE_MIN_COLOR_R;
        currentG = INTERACTIVE_MIN_COLOR_G;
        currentB = INTERACTIVE_MIN_COLOR_B;
    }

    // 渐变动画：分步过渡
    const int steps = 50;  // 渐变步数
    const int delayPerStep = TRANSITION_TIME_MS / steps;  // 每步延迟

    for(int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;  // 插值系数 0.0 → 1.0

        // 线性插值计算当前值（使用int16_t处理差值）
        int16_t brightnessDelta = targetBrightness - currentBrightness;
        int16_t rDelta = targetR - currentR;
        int16_t gDelta = targetG - currentG;
        int16_t bDelta = targetB - currentB;

        uint8_t stepBrightness = currentBrightness + (int16_t)(brightnessDelta * t);
        uint8_t stepR = currentR + (int16_t)(rDelta * t);
        uint8_t stepG = currentG + (int16_t)(gDelta * t);
        uint8_t stepB = currentB + (int16_t)(bDelta * t);

        // 更新所有灯带
        for (int j = 0; j < STRIP_COUNT; j++) {
            strips[j]->setBrightness(stepBrightness);
            strips[j]->fillColor(stepR, stepG, stepB);
            strips[j]->show();
        }

        delay(delayPerStep);
    }

    // 保存最终值作为下次的起始值
    currentBrightness = targetBrightness;
    currentR = targetR;
    currentG = targetG;
    currentB = targetB;

    Serial.println("渐变完成");

    // 检查是否完成（8/8）
    if (breathTriggerCount >= MAX_BREATH_TRIGGERS) {
        Serial.println("\n!!! 进度完成 8/8 - 已达到最大亮度 !!!\n");
    }
}

// ==================== 处理超时 ====================
void handleTimeout() {
    Serial.println("\n[DEBUG] handleTimeout 被调用");
    timeoutFlag = false;

    Serial.println("\n========== 超时 ==========");
    Serial.printf("%d秒内无吹气信号 (当前进度: %d/%d)\n",
                  TIMEOUT_MS / 1000,
                  breathTriggerCount, MAX_BREATH_TRIGGERS);
    Serial.println("返回正常状态...");
    Serial.println("===========================\n");

    enterNormalState();
}

// ==================== 进入正常状态 ====================
void enterNormalState() {
    Serial.println("\n--- 进入正常状态 ---");

    currentState = STATE_NORMAL;
    breathTriggerCount = 0;
    timeoutFlag = false;

    // 停止定时器
    timer_stop();
    Serial.println("定时器: 已停止");

    // 更新基准湿度
    updateBaselineHumidity();

    // 灯带恢复正常状态（全亮）
    for (int i = 0; i < STRIP_COUNT; i++) {
        strips[i]->setBrightness(NORMAL_BRIGHTNESS);
        strips[i]->fillColor(NORMAL_COLOR_R, NORMAL_COLOR_G, NORMAL_COLOR_B);
        strips[i]->show();
    }
    Serial.printf("灯带: 正常状态 (亮度: %d, 颜色: RGB(%d,%d,%d))\n\n",
                  NORMAL_BRIGHTNESS, NORMAL_COLOR_R, NORMAL_COLOR_G, NORMAL_COLOR_B);
}

// ==================== 初始化 ====================
void app_Init() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n╔════════════════════════════════╗");
    Serial.println("║    许愿树系统初始化 v2.0       ║");
    Serial.println("║    吹气交互模式               ║");
    Serial.println("╚════════════════════════════════╝\n");

    // 1. 初始化灯带
    Serial.println("[1/4] 初始化灯带...");
    for (int i = 0; i < STRIP_COUNT; i++) {
        strips[i]->begin();
        strips[i]->setBrightness(NORMAL_BRIGHTNESS);
        Serial.printf("  ? 灯带 %d 初始化完成 (%d LEDs)\n", i, STRIP_NUM_LEDS);
    }

    // 2. 初始化温湿度传感器
    Serial.println("\n[2/4] 初始化温湿度传感器...");
    if(sht31Sensor.begin()) {
        Serial.println("  ? SHT31 传感器初始化成功");

        // 读取初始湿度作为基准
        float temp, humi;
        if(sht31Sensor.readTempHumi(temp, humi)) {
            baselineHumidity = humi;
            Serial.printf("  ? 初始湿度: %.2f%% (温度: %.2f°C)\n", humi, temp);
        }
    } else {
        Serial.println("  ? 传感器初始化失败！");
    }
    lastSensorRead = millis();

    // 3. 初始化定时器
    Serial.println("\n[3/4] 初始化定时器...");
    timer_task_init(TIMEOUT_MS, timer_callback);
    Serial.printf("  ? 定时器初始化完成 (超时: %d ms)\n", TIMEOUT_MS);

    // 4. 进入正常状态
    Serial.println("\n[4/4] 设置初始状态...");
    currentState = STATE_NORMAL;
    breathTriggerCount = 0;
    timeoutFlag = false;

    for (int i = 0; i < STRIP_COUNT; i++) {
        strips[i]->fillColor(NORMAL_COLOR_R, NORMAL_COLOR_G, NORMAL_COLOR_B);
        strips[i]->show();
    }
    Serial.println("  ? 灯带设置为正常状态（常亮）");

    Serial.println("\n╔════════════════════════════════╗");
    Serial.println("║      初始化完成！             ║");
    Serial.println("║                                ║");
    Serial.println("║  吹气检测阈值: 湿度增加 3%    ║");
    Serial.println("║  吹气次数: 8次完全点亮        ║");
    Serial.println("║  超时时间: 5秒                ║");
    Serial.println("╚════════════════════════════════╝\n");
}

// ==================== 主任务循环 ====================
void app_Task() {
    // 1. 检测吹气（基于湿度变化）
    if(checkBreathDetection()) {
        handleBreathDetected();
        // 渐变动画已经包含了足够的延迟，不需要额外防抖
    }

    // 2. 在交互状态下检查超时
    if (currentState == STATE_INTERACTIVE && timeoutFlag) {
        handleTimeout();
    }

    // 3. 调试：定期输出状态
    static unsigned long lastDebug = 0;
    if(millis() - lastDebug > 2000 && currentState == STATE_INTERACTIVE) {
        Serial.printf("[DEBUG] 状态: 交互模式, 进度: %d/8, timeoutFlag: %d\n",
                      breathTriggerCount, timeoutFlag);
        lastDebug = millis();
    }

    // 4. 循环延迟
    delay(50);
}

// ==================== 测试任务（注释掉的代码） ====================
/*
void app_Task() {
    Serial.begin(115200);
    delay(1000);

    // 测试选项
    //strips_Test();              // 灯带测试
    //tempHumi_Sensor_Test();     // 温湿度传感器测试

    delay(1000);
}
*/