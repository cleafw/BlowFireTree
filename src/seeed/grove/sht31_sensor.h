//
// Grove 温湿度传感器驱动头文件
// 型号: Grove - Temperature&Humidity Sensor (SHT31)
// 工作电压: 3.3V - 5V
// 接口: I2C，地址 0x44 (默认) 或 0x45
// 温度范围: -40℃ ~ +125℃
// 温度精度: ±0.3℃
// 湿度范围: 0-100% RH
// 湿度精度: ±2% RH
// 响应时间: < 8 秒 (tau 63%)
//

#ifndef SHT31_SENSOR_H
#define SHT31_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

// SHT31 I2C 地址
#define SHT31_ADDR_DEFAULT      0x44    // 默认 I2C 地址（ADDR 引脚接地）
#define SHT31_ADDR_ALTERNATE    0x45    // 备用 I2C 地址（ADDR 引脚接 VDD）

// SHT31 命令（高重复性测量）
#define SHT31_CMD_MEASURE_HIGH  0x2C06  // 高重复性测量（推荐）
#define SHT31_CMD_MEASURE_MED   0x2C0D  // 中重复性测量
#define SHT31_CMD_MEASURE_LOW   0x2C10  // 低重复性测量
#define SHT31_CMD_READSTATUS    0xF32D  // 读取状态寄存器
#define SHT31_CMD_CLEARSTATUS   0x3041  // 清除状态寄存器
#define SHT31_CMD_SOFTRESET     0x30A2  // 软复位
#define SHT31_CMD_HEATER_ON     0x306D  // 开启加热器
#define SHT31_CMD_HEATER_OFF    0x3066  // 关闭加热器

class SHT31Sensor {
private:
    uint8_t i2cAddress;     // I2C 地址
    int8_t  sdaPin;         // SDA 引脚（-1 表示使用默认引脚）
    int8_t  sclPin;         // SCL 引脚（-1 表示使用默认引脚）

    float   lastTemp;       // 上次读取的温度值
    float   lastHumi;       // 上次读取的湿度值

    // 传感器初始化的内部实现
    bool initSensor();

    // 发送命令
    bool sendCommand(uint16_t cmd);

    // 读取原始数据（6字节：温度2字节+CRC1字节+湿度2字节+CRC1字节）
    bool readRawData(uint16_t &rawTemp, uint16_t &rawHumi);

    // CRC8 校验
    uint8_t calculateCRC(const uint8_t *data, uint8_t len);
    bool verifyCRC(const uint8_t *data, uint8_t len, uint8_t crc);

public:
    // 构造函数 - 使用默认 I2C 引脚和地址
    explicit SHT31Sensor(uint8_t addr = SHT31_ADDR_DEFAULT);

    // 构造函数 - 自定义 I2C 引脚（适用于 ESP32）
    // 参数: I2C地址, SDA引脚, SCL引脚
    SHT31Sensor(uint8_t addr, int8_t sda, int8_t scl);

    // 初始化（使用默认引脚）
    bool begin();

    // 初始化（自定义引脚）- 适用于 ESP32
    // 参数: SDA引脚, SCL引脚, I2C频率（默认400kHz，SHT31支持最高1MHz）
    bool begin(int8_t sda, int8_t scl, uint32_t frequency = 400000);

    // 读取温度（摄氏度）
    float getTemperature();

    // 读取湿度（百分比）
    float getHumidity();

    // 同时读取温度和湿度（更高效，推荐）
    bool readTempHumi(float &temp, float &humi);

    // 读取多次采样平均值（提高稳定性，减少噪声）
    // 参数: samples - 采样次数（1-20，默认5次）
    //       delayMs - 每次采样间隔时间（ms，默认50ms）
    bool readTempHumiAvg(float &temp, float &humi, uint8_t samples = 5, uint16_t delayMs = 50);

    // 获取上次读取的温度值（不进行新的测量）
    float getLastTemperature();

    // 获取上次读取的湿度值（不进行新的测量）
    float getLastHumidity();

    // 软复位
    bool reset();

    // 读取状态寄存器
    uint16_t readStatus();

    // 清除状态寄存器
    bool clearStatus();

    // 开启加热器（用于去除传感器表面的凝露）
    bool enableHeater();

    // 关闭加热器
    bool disableHeater();

    // 检查加热器状态
    bool isHeaterEnabled();
};

#endif // SHT31_SENSOR_H