//
// Grove 温湿度传感器驱动实现
// 支持 ESP32 自定义 I2C 引脚配置
//

#include "sht31_sensor.h"

// 构造函数 - 使用默认引脚
SHT31Sensor::SHT31Sensor(uint8_t addr)
        : i2cAddress(addr), sdaPin(-1), sclPin(-1), lastTemp(0), lastHumi(0) {
}

// 构造函数 - 自定义 I2C 引脚
SHT31Sensor::SHT31Sensor(uint8_t addr, int8_t sda, int8_t scl)
        : i2cAddress(addr), sdaPin(sda), sclPin(scl), lastTemp(0), lastHumi(0) {
}

// 初始化 - 使用默认引脚或构造函数指定的引脚
bool SHT31Sensor::begin() {
    // 如果在构造函数中指定了引脚，使用指定的引脚
    if (sdaPin >= 0 && sclPin >= 0) {
        return begin(sdaPin, sclPin);
    }

    // 否则使用默认引脚
    Wire.begin();
    delay(100);  // 等待 I2C 初始化

    return initSensor();
}

// 初始化 - 自定义引脚（ESP32 支持）
bool SHT31Sensor::begin(int8_t sda, int8_t scl, uint32_t frequency) {
    // 保存引脚配置
    sdaPin = sda;
    sclPin = scl;

    // 使用自定义引脚初始化 I2C
    Wire.begin(sda, scl, frequency);
    delay(100);  // 等待 I2C 初始化

    Serial.printf("SHT31: I2C 初始化 SDA=%d, SCL=%d, 频率=%u Hz\n", sda, scl, frequency);

    return initSensor();
}

// 传感器初始化的内部实现
bool SHT31Sensor::initSensor() {
    // 扫描 I2C 设备
    Wire.beginTransmission(i2cAddress);
    uint8_t error = Wire.endTransmission();
    if (error != 0) {
        Serial.printf("SHT31 I2C 错误: 地址 0x%02X 无响应 (错误码: %d)\n", i2cAddress, error);
        Serial.println("请检查:");
        Serial.println("1. SDA/SCL 引脚连接是否正确");
        Serial.println("2. 传感器是否供电 (VCC/GND)");
        Serial.printf("3. I2C 地址是否正确 (0x44 或 0x45)\n");
        return false;  // 设备未连接
    }

    // 软复位传感器
    if (!reset()) {
        Serial.println("SHT31 软复位失败！");
        return false;
    }

    delay(50);  // 等待复位完成

    // 读取状态寄存器验证
    uint16_t status = readStatus();
    Serial.printf("SHT31 状态寄存器: 0x%04X\n", status);

    // 测试读取一次数据
    float temp, humi;
    if (readTempHumi(temp, humi)) {
        Serial.println("SHT31 初始化成功！");
        Serial.printf("初始读数 - 温度: %.2f °C, 湿度: %.2f %%\n", temp, humi);
        return true;
    } else {
        Serial.println("SHT31 测试读取失败！");
        return false;
    }
}

// 发送命令
bool SHT31Sensor::sendCommand(uint16_t cmd) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(cmd >> 8);    // 高字节
    Wire.write(cmd & 0xFF);  // 低字节
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        Serial.printf("SHT31 命令发送失败 (0x%04X): 错误 %d\n", cmd, error);
        return false;
    }
    return true;
}

// CRC8 校验（多项式 0x31，初始值 0xFF）
uint8_t SHT31Sensor::calculateCRC(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

// 验证 CRC
bool SHT31Sensor::verifyCRC(const uint8_t *data, uint8_t len, uint8_t crc) {
    return calculateCRC(data, len) == crc;
}

// 读取原始数据
bool SHT31Sensor::readRawData(uint16_t &rawTemp, uint16_t &rawHumi) {
    // 发送测量命令（高重复性）
    if (!sendCommand(SHT31_CMD_MEASURE_HIGH)) {
        return false;
    }

    // 等待测量完成（高重复性测量需要 15ms）
    delay(20);

    // 读取 6 字节数据
    uint8_t bytesRead = Wire.requestFrom(i2cAddress, (uint8_t)6);
    if (bytesRead != 6) {
        Serial.printf("SHT31 数据读取错误: 期望 6 字节，实际 %d 字节\n", bytesRead);
        return false;
    }

    uint8_t data[6];
    for (uint8_t i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }

    // 验证 CRC（温度）
    if (!verifyCRC(&data[0], 2, data[2])) {
        Serial.println("SHT31 温度 CRC 校验失败！");
        return false;
    }

    // 验证 CRC（湿度）
    if (!verifyCRC(&data[3], 2, data[5])) {
        Serial.println("SHT31 湿度 CRC 校验失败！");
        return false;
    }

    // 组合原始数据
    rawTemp = (data[0] << 8) | data[1];
    rawHumi = (data[3] << 8) | data[4];

    return true;
}

// 同时读取温度和湿度（推荐）
bool SHT31Sensor::readTempHumi(float &temp, float &humi) {
    uint16_t rawTemp, rawHumi;

    if (!readRawData(rawTemp, rawHumi)) {
        return false;
    }

    // 转换温度：T = -45 + 175 * (rawTemp / 2^16)
    temp = -45.0 + 175.0 * ((float)rawTemp / 65535.0);

    // 转换湿度：RH = 100 * (rawHumi / 2^16)
    humi = 100.0 * ((float)rawHumi / 65535.0);

    // 湿度限制在 0-100%
    if (humi < 0) humi = 0;
    if (humi > 100) humi = 100;

    // 保存最新值
    lastTemp = temp;
    lastHumi = humi;

    return true;
}

// 读取温度
float SHT31Sensor::getTemperature() {
    float temp, humi;
    if (readTempHumi(temp, humi)) {
        return temp;
    }
    return -999.0;  // 错误值
}

// 读取湿度
float SHT31Sensor::getHumidity() {
    float temp, humi;
    if (readTempHumi(temp, humi)) {
        return humi;
    }
    return -999.0;  // 错误值
}

// 获取上次读取的温度值
float SHT31Sensor::getLastTemperature() {
    return lastTemp;
}

// 获取上次读取的湿度值
float SHT31Sensor::getLastHumidity() {
    return lastHumi;
}

// 读取多次采样平均值
bool SHT31Sensor::readTempHumiAvg(float &temp, float &humi, uint8_t samples, uint16_t delayMs) {
    // 限制采样次数在合理范围内
    if (samples == 0) {
        samples = 1;
    } else if (samples > 20) {
        samples = 20;
    }

    float tempSum = 0;
    float humiSum = 0;
    uint8_t validSamples = 0;

    for (uint8_t i = 0; i < samples; i++) {
        float t, h;

        // 读取一次数据
        if (readTempHumi(t, h)) {
            tempSum += t;
            humiSum += h;
            validSamples++;
        } else {
            Serial.printf("采样 %d/%d 失败\n", i + 1, samples);
        }

        // 最后一次采样不需要延迟
        if (i < samples - 1) {
            delay(delayMs);
        }
    }

    // 如果没有有效样本，返回失败
    if (validSamples == 0) {
        Serial.println("所有采样均失败！");
        return false;
    }

    // 计算平均值
    temp = tempSum / validSamples;
    humi = humiSum / validSamples;

    // 更新缓存值
    lastTemp = temp;
    lastHumi = humi;

    // 如果部分采样失败，给出提示
    if (validSamples < samples) {
        Serial.printf("采样完成: %d/%d 有效\n", validSamples, samples);
    }

    return true;
}

// 软复位
bool SHT31Sensor::reset() {
    bool success = sendCommand(SHT31_CMD_SOFTRESET);
    if (success) {
        delay(50);  // 等待复位完成
    }
    return success;
}

// 读取状态寄存器
uint16_t SHT31Sensor::readStatus() {
    if (!sendCommand(SHT31_CMD_READSTATUS)) {
        return 0xFFFF;
    }

    delay(10);

    uint8_t bytesRead = Wire.requestFrom(i2cAddress, (uint8_t)3);
    if (bytesRead != 3) {
        return 0xFFFF;
    }

    uint8_t data[3];
    for (uint8_t i = 0; i < 3; i++) {
        data[i] = Wire.read();
    }

    // 验证 CRC
    if (!verifyCRC(&data[0], 2, data[2])) {
        Serial.println("SHT31 状态寄存器 CRC 校验失败！");
        return 0xFFFF;
    }

    return (data[0] << 8) | data[1];
}

// 清除状态寄存器
bool SHT31Sensor::clearStatus() {
    return sendCommand(SHT31_CMD_CLEARSTATUS);
}

// 开启加热器
bool SHT31Sensor::enableHeater() {
    return sendCommand(SHT31_CMD_HEATER_ON);
}

// 关闭加热器
bool SHT31Sensor::disableHeater() {
    return sendCommand(SHT31_CMD_HEATER_OFF);
}

// 检查加热器状态
bool SHT31Sensor::isHeaterEnabled() {
    uint16_t status = readStatus();
    // 位 13: 加热器状态（1 = 开启，0 = 关闭）
    return (status & 0x2000) != 0;
}