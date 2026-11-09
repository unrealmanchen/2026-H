
#ifndef _BSP_SYSTICK_H_
#define _BSP_SYSTICK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//SysTick 是 24 位计数器
#define SYSTICK_MAX_COUNT       (0x00FFFFFFUL)
#define SYSTICK_OVERFLOW_CYCLES (SYSTICK_MAX_COUNT + 1UL)

typedef struct {
    uint64_t s;    
    uint32_t ms;     
    uint32_t us;     
} SYSTICK_Time_t;

//不开中断???应该不需要我初始化吧
void SYSTICK_Time_Init(uint32_t CPU_Freq_Hz);


float SYSTICK_GetDeltaT(uint32_t *cnt_last);
double SYSTICK_GetDeltaT64(uint32_t *cnt_last);//高精度？？？


float SYSTICK_GetTimeline_s(void);//自启动以来的时间
float SYSTICK_GetTimeline_ms(void);
uint64_t SYSTICK_GetTimeline_us(void);

void SYSTICK_Delay(float Delay_s);//阻塞计时器
void Delay(int ms);
void Delay_long(int ms);
long long Get_Currernt_Time(void);


//更新溢出 应该不需要
void SYSTICK_Time_UpdateIRQ(void);

#ifdef __cplusplus
}
#endif

#endif