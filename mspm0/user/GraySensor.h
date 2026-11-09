/**
 * @file    GraySensor.h
 * @brief   12路灰度传感器
 */
#ifndef __GraySensor_H
#define __GraySensor_H

float ReadGraySensor(void);
int GetGraySensorTriggerCount(void);
#define LOST_TARGET -9999

#endif
