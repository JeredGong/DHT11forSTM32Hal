#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f1xx.h"
#define DHT_READ_INTERVAL 30000 // 设置最小读取间隔为 30000 毫秒
#define DHT_DATA_PIN GPIO_PIN_4
#define DHT_GPIO_PORT GPIOB   //更改为你实际使用的引脚，并在main.c里初始化
void Delay_us(uint16_t us);
void DHT_GPIO_SET_OUTPUT(void);
void DHT_GPIO_SET_INPUT(void);
uint8_t DHT_Read_Byte(void);
uint8_t DHT_Read(void);

#endif
