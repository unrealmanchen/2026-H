#include "GraySensor.h"
#include "ti_msp_dl_config.h"

/* 
 * 灰度传感器引脚映射 (根据 SysConfig 配置更新):
 * R0  -> GraySensor_PIN1  (PA16)
 * R1  -> GraySensor_PIN2  (PA22)
 * R2  -> GraySensor_PIN3  (PC2)
 * R3  -> GraySensor_PIN4  (PC3)
 * R4  -> GraySensor_PIN5  (PC4)
 * R5  -> GraySensor_PIN6  (PC5)
 * R6  -> GraySensor_PIN7  (PB3)
 * R7  -> GraySensor_PIN8  (PB2)
 * R8  -> GraySensor_PIN9  (PC8)
 * R9  -> GraySensor_PIN10 (PC9)
 * R10 -> GraySensor_PIN11 (PB23)
 * R11 -> GraySensor_PIN12 (PB6)
 */

#define R0   !((GraySensor_PIN1_PORT->DIN31_0  & GraySensor_PIN1_PIN  ) ? 1 : 0)
#define R1   !((GraySensor_PIN2_PORT->DIN31_0  & GraySensor_PIN2_PIN  ) ? 1 : 0)
#define R2   !((GraySensor_PIN3_PORT->DIN31_0  & GraySensor_PIN3_PIN  ) ? 1 : 0)
#define R3   !((GraySensor_PIN4_PORT->DIN31_0  & GraySensor_PIN4_PIN  ) ? 1 : 0)
#define R4   !((GraySensor_PIN5_PORT->DIN31_0  & GraySensor_PIN5_PIN  ) ? 1 : 0)
#define R5   !((GraySensor_PIN6_PORT->DIN31_0  & GraySensor_PIN6_PIN  ) ? 1 : 0)
#define R6   !((GraySensor_PIN7_PORT->DIN31_0  & GraySensor_PIN7_PIN  ) ? 1 : 0)
#define R7   !((GraySensor_PIN8_PORT->DIN31_0  & GraySensor_PIN8_PIN  ) ? 1 : 0)
#define R8   !((GraySensor_PIN9_PORT->DIN31_0  & GraySensor_PIN9_PIN  ) ? 1 : 0)
#define R9   !((GraySensor_PIN10_PORT->DIN31_0 & GraySensor_PIN10_PIN ) ? 1 : 0)
#define R10  !((GraySensor_PIN11_PORT->DIN31_0 & GraySensor_PIN11_PIN ) ? 1 : 0)
#define R11  !((GraySensor_PIN12_PORT->DIN31_0 & GraySensor_PIN12_PIN ) ? 1 : 0)


float weight[6]={15,3,2.5,2,1.5,1};//5中心权重 0是边缘权重

int nodata_count = 0;
#define READ_Fc 200.0f
#define READ_INTERVAL (float)1.0f / READ_Fc
	
float last_gray_count = 0;
float ReadGraySensor(void)//输出正值偏左
{
    int count = 0;
    float data[12];
    data[0] = R0;
    data[1] = R1;
    data[2] = R2;
    data[3] = R3;
    data[4] = R4;
    data[5] = R5;
    data[6] = R6;
    data[7] = R7;
    data[8] = R8;
    data[9] = R9;
    data[10] = R10;
    data[11] = R11;


    for(int i=0;i<12;i++)
    {
        if(data[i]==0)
            count++;
    }
    float output = 0;
    output += data[0] * weight[0];
    output += data[1] * weight[1];
    output += data[2] * weight[2];
    output += data[3] * weight[3];
    output += data[4] * weight[4];
    output += data[5] * weight[5];
    output += data[6] * weight[5] *-1.0f;
    output += data[7] * weight[4] *-1.0f;
    output += data[8] * weight[3] *-1.0f;
    output += data[9] * weight[2] *-1.0f;
    output += data[10] * weight[1] *-1.0f;
    output += data[11] * weight[0] *-1.0f;    
    if(count != 0)
    output /= (float)count;
    
    if(count == 0)
        return last_gray_count;
    last_gray_count = output;
    return output;
}
int GetGraySensorTriggerCount(void)
{
    return (!R0 + !R1 + !R2 + !R3 + !R4 + !R5 + !R6 + !R7 + !R8 + !R9 + !R10 + !R11);
}
