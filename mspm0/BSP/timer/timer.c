/**
 ******************************************************************************
 * @file    bsp_systick.c
 * @brief   MSPM0 SysTick 高精度时间库 (替代 DWT CYCCNT)
 * @note    适用于 MSPM0G3507
 ******************************************************************************
 */

#include "timer.h"
#include "ti_msp_dl_config.h"   // DriverLib 配置头文件
#include <stdlib.h>              // 用于 __disable_irq / __enable_irq

/* ========== 静态变量 ========== */
static uint32_t CPU_FREQ_Hz = 80000000;          // CPU 主频
static uint32_t CPU_FREQ_Hz_ms;                  // 每毫秒周期数
static uint32_t CPU_FREQ_Hz_us;                  // 每微秒周期数

/* 64 位时间戳维护变量 */
static volatile uint32_t systick_overflow_count = 0; // 溢出次数
static volatile uint32_t systick_last_cnt = 0;       // 上次读取的计数值
static volatile uint64_t systick_cnt64 = 0;          // 64 位计数值

/* 时间结构体缓存 */
static SYSTICK_Time_t SysTime;

/* 简易自旋锁 (防止中断嵌套冲突) */
static volatile uint8_t bit_locker = 0;

/* ========== 私有函数 ========== */

/**
 * @brief 更新溢出计数 (在中断或读取时调用)
 */
static void SYSTICK_CNT_Update(void)
{
    if (!bit_locker)
    {
        bit_locker = 1;
        
        /* 读取当前计数值 (SysTick->VAL 是向下递减的) */
        uint32_t cnt_now = SysTick->VAL;
        
        /* 因为 VAL 是向下计数的，用 cnt_now < systick_last_cnt 来判断溢出 */
        if (cnt_now < systick_last_cnt)
        {
            systick_overflow_count++;
        }
        
        systick_last_cnt = cnt_now;
        bit_locker = 0;
    }
}

/**
 * @brief 更新 64 位时间戳
 */
static void SYSTICK_SysTimeUpdate(void)
{
    uint32_t cnt_now;
    
    /* 禁止中断，防止在读取过程中被 SysTick 中断修改 */
    __disable_irq();
    
    cnt_now = SysTick->VAL;
    SYSTICK_CNT_Update();  // 更新溢出计数
    
    /* 拼接 64 位计数值: (溢出次数 << 24) | (当前值取反) */
    /* 因为 VAL 是向下计数，需要转换为递增计数值: (MAX - VAL) */
    uint32_t cnt_increment = SYSTICK_MAX_COUNT - cnt_now;
    systick_cnt64 = ((uint64_t)systick_overflow_count << 24) | (uint64_t)cnt_increment;
    
    __enable_irq();
    
    /* 将周期数换算成 秒/毫秒/微秒 */
    uint64_t total_us = systick_cnt64 / (CPU_FREQ_Hz / 1000000);
    
    SysTime.s = total_us / 1000000;
    SysTime.ms = (total_us % 1000000) / 1000;
    SysTime.us = (total_us % 1000000) % 1000;
}

/* ========== 公有函数 ========== */

void SYSTICK_Time_Init(uint32_t CPU_Freq_Hz)
{
    CPU_FREQ_Hz = CPU_Freq_Hz;
    CPU_FREQ_Hz_ms = CPU_FREQ_Hz / 1000;
    CPU_FREQ_Hz_us = CPU_FREQ_Hz / 1000000;
    
    systick_overflow_count = 0;
    systick_last_cnt = 0;
    systick_cnt64 = 0;
    
    /* 配置 SysTick：24位，自由运行模式，不开启中断 */
    /* 设置 LOAD 为最大值 (0x00FFFFFF) */
    SysTick->LOAD = SYSTICK_MAX_COUNT;
    
    /* 写入任意值启动计数器，时钟源为 MCLK (CPU_FREQ_Hz) */
    SysTick->VAL = SYSTICK_MAX_COUNT;
    
    /* 使能计数器，时钟源 = MCLK，不使能中断 */
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    /* 注意：不使能 TICKINT 位 */
    
    /* 初始化读取一次 */
    systick_last_cnt = SysTick->VAL;
}

float SYSTICK_GetDeltaT(uint32_t *cnt_last)
{
    uint32_t cnt_now = SysTick->VAL;
    float dt;
    
    /* SysTick->VAL 是向下计数的，所以用 cnt_last - cnt_now */
    if (cnt_now <= *cnt_last)
    {
        /* 正常情况：计数器递减中 */
        dt = (float)(*cnt_last - cnt_now) / (float)CPU_FREQ_Hz;
    }
    else
    {
        /* 发生了一次溢出：计数器从 0 滚到了 MAX，然后继续递减到 cnt_now */
        dt = (float)(*cnt_last + SYSTICK_OVERFLOW_CYCLES - cnt_now) / (float)CPU_FREQ_Hz;
    }
    
    *cnt_last = cnt_now;
    return dt;
}

double SYSTICK_GetDeltaT64(uint32_t *cnt_last)
{
    uint32_t cnt_now = SysTick->VAL;
    double dt;
    
    if (cnt_now <= *cnt_last)
    {
        dt = (double)(*cnt_last - cnt_now) / (double)CPU_FREQ_Hz;
    }
    else
    {
        dt = (double)(*cnt_last + SYSTICK_OVERFLOW_CYCLES - cnt_now) / (double)CPU_FREQ_Hz;
    }
    
    *cnt_last = cnt_now;
    return dt;
}

float SYSTICK_GetTimeline_s(void)
{
    SYSTICK_SysTimeUpdate();
    return (float)SysTime.s + (float)SysTime.ms * 0.001f + (float)SysTime.us * 0.000001f;
}

float SYSTICK_GetTimeline_ms(void)
{
    SYSTICK_SysTimeUpdate();
    return (float)SysTime.s * 1000.0f + (float)SysTime.ms + (float)SysTime.us * 0.001f;
}

uint64_t SYSTICK_GetTimeline_us(void)
{
    SYSTICK_SysTimeUpdate();
    return SysTime.s * 1000000ULL + SysTime.ms * 1000ULL + SysTime.us;
}

void SYSTICK_Delay(float Delay_s)
{   
    if(Delay_s > 0.2f){
        HardFault_Handler();
    }
    uint32_t tickstart = SysTick->VAL;
    float wait = Delay_s * (float)CPU_FREQ_Hz;
    
    while (1)
    {
        uint32_t cnt_now = SysTick->VAL;
        uint32_t delta;
        
        if (cnt_now <= tickstart)
        {
            delta = tickstart - cnt_now;
        }
        else
        {
            delta = tickstart + SYSTICK_OVERFLOW_CYCLES - cnt_now;
        }
        
        if ((float)delta >= wait)
            break;
    }
}
void Delay( int ms){
    SYSTICK_Delay(ms*0.001f);
}
void SYSTICK_Time_UpdateIRQ(void)
{
    /* 在中断中更新溢出计数 */
    if (!bit_locker)
    {
        uint32_t cnt_now = SysTick->VAL;
        if (cnt_now < systick_last_cnt)
        {
            systick_overflow_count++;
        }
        systick_last_cnt = cnt_now;
    }
    
    /* 清除中断标志 (SysTick 硬件自动清除) */
}


volatile long long counter_1ms;

long long Get_Currernt_Time(void){
    return counter_1ms;
}
void Delay_long( int ms){
    long long tickstart = counter_1ms;
    while (1)
    {
        uint32_t cnt_now = counter_1ms;
        uint32_t delta;

            delta = tickstart - cnt_now;

        if ((float)delta >= ms)
            break;
    }
}



void TIMA0_IRQHandler(void){
    counter_1ms++;
}

