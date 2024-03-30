#include "stm32f1xx.h"
#include "delay.h"
#include "dht11.h"
static char Success=0;  //Success用于判断是否成功读取到温湿度数据
uint8_t Data[5]={0x00,0x00,0x00,0x00,0x00}; 
void DHT_GPIO_SET_OUTPUT(void)     //设置GPIOx为输出模式（MCU的IO口向DHT11发激活信号）
{
	GPIO_InitTypeDef GPIO_InitStructure;    //在GPIO_InitTypeDef结构体中修改IO口参数（结构体成员）
	GPIO_InitStructure.Pin=DHT_DATA_PIN;      //设置的格式必须严格遵循注释，比如GPIO_PIN_define
	GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT_GPIO_PORT,&GPIO_InitStructure);
}
void DHT_GPIO_SET_INPUT(void)     //设置GPIOx为输入模式（DHT11向MUC的IO发电平信号，信号里包含了温湿度信息）
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin=DHT_DATA_PIN;
	GPIO_InitStructure.Mode=GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT_GPIO_PORT,&GPIO_InitStructure);
}
uint8_t DHT_Read_Byte(void)  //从DHT11读取一位（8字节）信号
{
	 uint8_t ReadData=0;  //ReadData用于存放8bit数据，即8个单次读取的1bit数据的组合
	 uint8_t temp;      //临时存放信号电平（0或1）
	 uint8_t retry=0;   //retry用于防止卡死
	 uint8_t i;    
	 for(i=0; i<8; i++)   //一次温湿度信号读取八位
	 {
			while(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==0 && retry<100)  
			//等待直到DHT11输出高电平：当PA5=1，上升沿，表示开始接受数据，可以判断0 or 1，跳出循环，执行后续判断（若PA5=0，将一直循环等待）
			{
				delayus(1);
				retry++;             //retry防止PA5读取不到数据卡死在这一步，当经历100us后retry自增到100，跳出循环。
			}
			retry=0;
			
			delayus(40);    //延时30us
			//根据时序图，DHT传回高电平信号维持26us~28us表示0，	维持70us表示1
		  //延时30us后，如果IO读取到仍是高电平，说明采集到1；如果IO读取到低电平，说明采集到0
			//读取电平信号暂存temp内，随后会压入ReadData中
			if(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==1)   temp=1;
			 else   temp=0;

			 while(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==1 && retry<100)
			//等待直到DHT11输出低电平，表示退出。本轮1bit信号接收完毕。
			 {
				 delayus(1);
				 retry++;
			 }
			 retry=0;
			 
			 ReadData<<=1;    //ReadData内信号先全部左移一位，空出末尾位置
			 ReadData |= temp;        //将temp写入ReadData
	 }

		return ReadData;
}
uint8_t DHT_Read(void)
{
    static uint32_t last_read_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 检查是否已经过了足够的读取间隔
    if (((current_time - last_read_time) < DHT_READ_INTERVAL)&&(Success==1)) {
        return 2; // 早于读取间隔，返回2表示未执行读取
    }
    else {
        last_read_time = current_time;
    }
	 uint8_t retry=0;
	 uint8_t i;
		
	 DHT_GPIO_SET_OUTPUT();    //IO设置为输出模式。在传输的最开始，MCU要向DHT11发送信号
	 HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_DATA_PIN,GPIO_PIN_RESET);   //IO->DHT11:先拉低电平18ms（应时序要求）
	 HAL_Delay(18);
	 HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_DATA_PIN,GPIO_PIN_SET);   //IO->DHT11:随后拉高电平20us
	 delayus(20);
	
	//MCU通过IO向DHT11发送请求完毕。接下来DHT11向IO发送响应,IO转为输入模式。在这之后就开始信号的转译读取。
	DHT_GPIO_SET_INPUT();
	delayus(20);
	if(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_DATA_PIN)==0) //DHT11发回低电平响应（读取到低电平，说明DHT11有响应）
	{
		//接下来，DHT11拉低电平一段时间后拉高电平一段时间
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
		
		//一共传输40位，一次DHT_Read_Byte返回8位，共读取5次。存储在Data[]中。（Data[]定义为全局）
		for(i=0; i<5; i++)
		{
			 Data[i] = DHT_Read_Byte();  //每次读取一字节（8位）
		}
		delayus(50);
		//说明：Data[0]湿度， Data[2]温度。Data[1]和Data[3]分别为0和2的小数位。Data[4]用于校验。
	}
	
	 uint32_t sum=Data[0]+Data[1]+Data[2]+Data[3];  //校验
	 if((sum)==Data[4])   { Success=1;  return 1;  }
	 else                 { Success=0;  return 0;  }
	
}