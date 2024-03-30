#  DHT11 Library for STM32 hal-Library

This project is a driver library for the DHT 11 temperature and humidity sensor based on the STM32 hal library.

## Works suitable for this repository

- Projects developed using the STM32 hal library
- Using DHT11 for your Projects

## How to Begin

1. Configure clocks and timers in STM32CubeMX, ensuring that one of the available timer frequencies is set to $1\mathrm{MHz}$.(default timer is tim2  `htim2`)
2. Modify the macro definitions in dht11.h to make sure you are using the correct **PIN**.(**default PIN is GPIOB PIN_4**)
3. Copy the four files `delay.h dht11.h delay.c dht11.c` to the appropriate location in your project.
4. Considering the communication interval of DHT11 module, the interval reading time after successful reading is set (default is 30s). You can modify the macro definition `DHT_READ_INTERVAL` in `dht11.h` as required
5. The value read is stored in the global variable `data[]`, which you can access via `extern uint8_t Data[5];`.

## Key Functions

1. `void delayus(uint16_t us)`

Used to achieve microsecond delays. Make sure your timer has a **frequency of $1\mathrm{MHz}$ and a period of $65535$.**

```c
void delayus(uint16_t us)
{
    uint16_t differ = 0xffff-us-5;				
	__HAL_TIM_SET_COUNTER(&htim2,differ);	//Set TIM2 counter start value
	HAL_TIM_Base_Start(&htim2);		//Run TIM2	
	
	while(differ < 0xffff-5){	
		differ = __HAL_TIM_GET_COUNTER(&htim2);		//Query the count value of the counter
	}
	HAL_TIM_Base_Stop(&htim2);
    }
```

2. `uint8_t DHT_Read(void)`

Reads the value of the sensor and places it in a global variable `Data[]`.  `Data[0]` is humidity integer and `Data[1]` is humidity decimal.`Data[3]` is temperature integer and `Data[4]` is temperature decimal.

```c
uint8_t DHT_Read(void)
{
    static uint32_t last_read_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // Check if a sufficient reading interval has passed
    if (((current_time - last_read_time) < DHT_READ_INTERVAL)&&(Success==1)) {
        return 2; 
    }
    else {
        last_read_time = current_time;
    }
	 uint8_t retry=0;
	 uint8_t i;
		
	 DHT_GPIO_SET_OUTPUT();    
	 HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_DATA_PIN,GPIO_PIN_RESET);   
	 HAL_Delay(18);
	 HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_DATA_PIN,GPIO_PIN_SET);   
	 delayus(20);
	DHT_GPIO_SET_INPUT();
	delayus(20);
	if(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==0) 
	{
		while(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==0 && retry<100)  
		{
		   delayus(1);
			retry++;
		}
		retry=0;
		while(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==1 && retry<100) 
		{
		   delayus(1);
			retry++;
		}
		retry=0;
		
		for(i=0; i<5; i++)
		{
			 Data[i] = DHT_Read_Byte();  //Read a byte for each loop
		}
		delayus(50);
		// Description: Data[0] humidity, Data[2] temperature. data[1] and data[3] are decimal places of 0 and         //              2 respectively. data[4] is used for checksum.
	}
	
	 uint32_t sum=Data[0]+Data[1]+Data[2]+Data[3];  //check
	 if((sum)==Data[4])   { Success=1;  return 1;  }
	 else                 { Success=0;  return 0;  }
	
}
```

