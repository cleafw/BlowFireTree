//
// Grove - RGB LED 环（20 - WS2813 Mini）驱动实现
//

#include "ws2813_ring.h"

// 构造函数
WS2813Ring::WS2813Ring(uint8_t dataPin, uint16_t numPixels) 
    : pin(dataPin), numLEDs(numPixels), brightness(255) {
    strip = new Adafruit_NeoPixel(numPixels, dataPin, NEO_GRB + NEO_KHZ800);
}

// 析构函数
WS2813Ring::~WS2813Ring() {
    if (strip) {
        delete strip;
    }
}

// 初始化LED环
void WS2813Ring::begin() {
    if (strip) {
        strip->begin();
        strip->setBrightness(brightness);
        strip->show(); // 初始化所有像素为"关闭"
    }
}

// 设置亮度
void WS2813Ring::setBrightness(uint8_t bright) {
    brightness = bright;
    if (strip) {
        strip->setBrightness(bright);
    }
}

// 清空所有LED
void WS2813Ring::clear() {
    if (strip) {
        strip->clear();
    }
}

// 更新显示
void WS2813Ring::show() {
    if (strip) {
        strip->show();
    }
}

// 设置单个LED颜色
void WS2813Ring::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (strip && n < numLEDs) {
        strip->setPixelColor(n, strip->Color(r, g, b));
    }
}

void WS2813Ring::setPixelColor(uint16_t n, uint32_t color) {
    if (strip && n < numLEDs) {
        strip->setPixelColor(n, color);
    }
}

// 设置所有LED为相同颜色
void WS2813Ring::fillColor(uint8_t r, uint8_t g, uint8_t b) {
    fillColor(Color(r, g, b));
}

void WS2813Ring::fillColor(uint32_t color) {
    if (strip) {
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, color);
        }
    }
}

// 颜色擦除效果
void WS2813Ring::colorWipe(uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
    colorWipe(Color(r, g, b), wait);
}

void WS2813Ring::colorWipe(uint32_t color, uint8_t wait) {
    if (strip) {
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, color);
            strip->show();
            delay(wait);
        }
    }
}

// 彩虹效果
void WS2813Ring::rainbow(uint8_t wait) {
    if (!strip) return;
    
    for (uint16_t j = 0; j < 256; j++) {
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, wheel((i + j) & 255));
        }
        strip->show();
        delay(wait);
    }
}

// 彩虹循环效果
void WS2813Ring::rainbowCycle(uint8_t wait) {
    if (!strip) return;
    
    for (uint16_t j = 0; j < 256 * 5; j++) {
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, wheel(((i * 256 / numLEDs) + j) & 255));
        }
        strip->show();
        delay(wait);
    }
}

// 剧院追逐效果
void WS2813Ring::theaterChase(uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
    theaterChase(Color(r, g, b), wait);
}

void WS2813Ring::theaterChase(uint32_t color, uint8_t wait) {
    if (!strip) return;
    
    for (int j = 0; j < 10; j++) {
        for (int q = 0; q < 3; q++) {
            for (uint16_t i = 0; i < numLEDs; i += 3) {
                strip->setPixelColor(i + q, color);
            }
            strip->show();
            delay(wait);
            
            for (uint16_t i = 0; i < numLEDs; i += 3) {
                strip->setPixelColor(i + q, 0);
            }
        }
    }
}

// 彩虹剧院追逐效果
void WS2813Ring::theaterChaseRainbow(uint8_t wait) {
    if (!strip) return;
    
    for (int j = 0; j < 256; j++) {
        for (int q = 0; q < 3; q++) {
            for (uint16_t i = 0; i < numLEDs; i += 3) {
                strip->setPixelColor(i + q, wheel((i + j) % 255));
            }
            strip->show();
            delay(wait);
            
            for (uint16_t i = 0; i < numLEDs; i += 3) {
                strip->setPixelColor(i + q, 0);
            }
        }
    }
}

// 呼吸灯效果
void WS2813Ring::breathe(uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
    if (!strip) return;
    
    // 渐亮
    for (int brightness = 0; brightness < 256; brightness++) {
        uint8_t adjustedR = (r * brightness) / 255;
        uint8_t adjustedG = (g * brightness) / 255;
        uint8_t adjustedB = (b * brightness) / 255;
        
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, strip->Color(adjustedR, adjustedG, adjustedB));
        }
        strip->show();
        delay(wait);
    }
    
    // 渐暗
    for (int brightness = 255; brightness >= 0; brightness--) {
        uint8_t adjustedR = (r * brightness) / 255;
        uint8_t adjustedG = (g * brightness) / 255;
        uint8_t adjustedB = (b * brightness) / 255;
        
        for (uint16_t i = 0; i < numLEDs; i++) {
            strip->setPixelColor(i, strip->Color(adjustedR, adjustedG, adjustedB));
        }
        strip->show();
        delay(wait);
    }
}

// 旋转效果
void WS2813Ring::rotate(uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
    if (!strip) return;
    
    uint32_t color = Color(r, g, b);
    
    for (uint16_t j = 0; j < numLEDs; j++) {
        clear();
        strip->setPixelColor(j, color);
        strip->show();
        delay(wait);
    }
}

// 创建颜色值
uint32_t WS2813Ring::Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// 获取LED数量
uint16_t WS2813Ring::numPixels() const {
    return numLEDs;
}

// 色轮函数
uint32_t WS2813Ring::wheel(byte wheelPos) {
    wheelPos = 255 - wheelPos;
    if (wheelPos < 85) {
        return Color(255 - wheelPos * 3, 0, wheelPos * 3);
    }
    if (wheelPos < 170) {
        wheelPos -= 85;
        return Color(0, wheelPos * 3, 255 - wheelPos * 3);
    }
    wheelPos -= 170;
    return Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}