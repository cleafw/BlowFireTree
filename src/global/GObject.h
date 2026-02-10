//
// 全局对象管理 - 集中管理所有外设实例
//

#ifndef WISHINGTREE_GOBJECT_H
#define WISHINGTREE_GOBJECT_H

#include "GData.h"
#include "seeed/grove/ws2813_ring.h"
#include "seeed/grove/sht31_sensor.h"


// ==================== 全局对象集合 ====================
extern WS2813Ring strip0;
extern WS2813Ring strip1;
extern WS2813Ring strip2;
extern WS2813Ring strip3;
extern WS2813Ring* strips[STRIP_COUNT];

extern SHT31Sensor sht31Sensor;

#endif // WISHINGTREE_GOBJECT_H
