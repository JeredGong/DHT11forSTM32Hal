#include "delay.h"

extern TIM_HandleTypeDef htim2; // 确保htim2是可访问的

// 实现微秒级延迟的函数
void delayus(uint16_t us)
{
    uint16_t differ = 0xffff-us-5;				
	__HAL_TIM_SET_COUNTER(&htim2,differ);	//设定TIM1计数器起始值
	HAL_TIM_Base_Start(&htim2);		//启动定时器	
	
	while(differ < 0xffff-5){	//判断
		differ = __HAL_TIM_GET_COUNTER(&htim2);		//查询计数器的计数值
	}
	HAL_TIM_Base_Stop(&htim2);
    }