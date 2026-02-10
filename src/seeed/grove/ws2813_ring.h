//
// Grove - RGB LED 环（20 - WS2813 Mini）驱动头文件
//

#ifndef WS2813_RING_H
#define WS2813_RING_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// 默认LED数量
#define WS2813_DEFAULT_NUM_LEDS 20

// RGB LED环类
class WS2813Ring {
private:
    Adafruit_NeoPixel* strip;
    uint16_t numLEDs;
    uint8_t pin;
    uint8_t brightness;

public:
    // 构造函数
    explicit WS2813Ring(uint8_t dataPin, uint16_t numPixels = WS2813_DEFAULT_NUM_LEDS);
    
    // 析构函数
    ~WS2813Ring();
    
    // 初始化LED环
    void begin();
    
    // 设置亮度 (0-255)
    void setBrightness(uint8_t bright);
    
    // 清空所有LED
    void clear();
    
    // 更新显示
    void show();
    
    // 设置单个LED颜色
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void setPixelColor(uint16_t n, uint32_t color);
    
    // 设置所有LED为相同颜色
    void fillColor(uint8_t r, uint8_t g, uint8_t b);
    void fillColor(uint32_t color);
    
    // 颜色擦除效果
    void colorWipe(uint8_t r, uint8_t g, uint8_t b, uint8_t wait);
    void colorWipe(uint32_t color, uint8_t wait);
    
    // 彩虹效果
    void rainbow(uint8_t wait);
    
    // 彩虹循环效果
    void rainbowCycle(uint8_t wait);
    
    // 剧院追逐效果
    void theaterChase(uint8_t r, uint8_t g, uint8_t b, uint8_t wait);
    void theaterChase(uint32_t color, uint8_t wait);
    
    // 彩虹剧院追逐效果
    void theaterChaseRainbow(uint8_t wait);
    
    // 呼吸灯效果
    void breathe(uint8_t r, uint8_t g, uint8_t b, uint8_t wait);
    
    // 旋转效果
    void rotate(uint8_t r, uint8_t g, uint8_t b, uint8_t wait);
    
    // 创建颜色值
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
    
    // 获取LED数量
    uint16_t numPixels() const;

private:
    // 色轮函数 - 生成彩虹色
    uint32_t wheel(byte wheelPos);
};

#endif // WS2813_RING_H

/*
 * 使用示例：
#include "ws2813_ring.h"

WS2813Ring ring(6); // 数据引脚D6

void setup() {
    ring.begin();
    ring.setBrightness(100); // 设置亮度
}

void loop() {
    ring.rainbow(20);              // 彩虹效果
    ring.theaterChase(255, 0, 0, 50); // 红色追逐
    ring.breathe(0, 255, 0, 5);    // 绿色呼吸灯
}
 * */