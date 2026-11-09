/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "FreeRTOS.h"
#include "ti_msp_dl_config.h"
#include "task.h"
#include "led/timerG6_pwm_rgb.h"
#include "imu/icm.h"
#include "imu/icm42688.h"
#include "timer/timer.h"
#include "GraySensor.h"
#include "motor.h"
#include "oled/oled.h"
#include "stdio.h"
TaskHandle_t main_hld = NULL;
TaskHandle_t ins_hld = NULL;
TaskHandle_t OLED_hld = NULL;
int g = 0, r = 0, b = 0;
float target_angle = 10.0f;
float current_angle = 0.0f;
extern float Yaw_AngleLast;
int ms_timer_enable = 0; // 计时器使能标志位 (0: 停止, 1: 计时中)

typedef enum
{
    TASK_IDLE = 0,
    TASK_MODE_1,
    TASK_MODE_2,
    TASK_MODE_3,
    TASK_MODE_4,
    TASK_MODE_5,
    TASK_MODE_6,
    TASK_STOPPING
} RunMode_t;
// UART transmit
void PRINT_SendString(char *str)
{
    while (*str != '\0')
    {
        DL_UART_Main_transmitDataBlocking(UART_PRINT_INST, *str++);
    }
}
int ms = 0;
volatile RunMode_t current_mode = TASK_MODE_1; // 默认选中任务 1
volatile uint8_t is_running = 0;               // 运行标志位 (0: 停止/待机, 1: 运行中)
char txbuf[100];
int task_wait = 0;
// press the button Get "1"
uint8_t Read_Button1(void)
{

    if (DL_GPIO_readPins(MESS_BUTTON1_PORT, MESS_BUTTON1_PIN) == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        if (DL_GPIO_readPins(MESS_BUTTON1_PORT, MESS_BUTTON1_PIN) == 0)
        {

            while (DL_GPIO_readPins(MESS_BUTTON1_PORT, MESS_BUTTON1_PIN) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            return 1;
        }
    }
    return 0;
}

// press the button Get "1"
uint8_t Read_Button2(void)
{
    if (DL_GPIO_readPins(MESS_BUTTON2_PORT, MESS_BUTTON2_PIN) == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(20)); // Software debounce
        if (DL_GPIO_readPins(MESS_BUTTON2_PORT, MESS_BUTTON2_PIN) == 0)
        {
            while (DL_GPIO_readPins(MESS_BUTTON2_PORT, MESS_BUTTON2_PIN) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10)); // Wait for release
            }
            return 1;
        }
    }
    return 0;
}
int printflag = 0;
int count = 0;
int count_max = 500; // 全速跑减速停车用[
int test_flag=0;
void main_task()
{
    Motor_init();
    static uint8_t stop_state = 0;

    while (1)
    {
        // ==================== 1. 按键交互逻辑 ====================
        if (!is_running)
        {
            // --- 待机状态 ---
            Motor_ForceStop(); // 确保电机停止

            // 按键 1: 循环切换 任务 1 -> 2 -> 3 -> 4 -> 5 -> 1
            if (current_mode != TASK_MODE_6 && Read_Button1() == 1)
            {
                switch (current_mode)
                {
                case TASK_MODE_1:
                    current_mode = TASK_MODE_2;
                    break;
                case TASK_MODE_2:
                    current_mode = TASK_MODE_3;
                    break;
                case TASK_MODE_3:
                    current_mode = TASK_MODE_4;
                    break;
                case TASK_MODE_4:
                    current_mode = TASK_MODE_5;
                    break;
                case TASK_MODE_5:
                    current_mode = TASK_MODE_6;
                    break;
                default:
                    current_mode = TASK_MODE_1;
                    break;
                }
            }
            if (Read_Button1() == 1 && current_mode == TASK_MODE_6)
            {
                PRINT_SendString("#8!");
            }

            // 按键 2: 确认启动当前选中的任务

            if (Read_Button2() == 1)
            {
                stop_state = 0;
                is_running = 1; // 标记为运行状态
            }
        }
        else
        {
            // --- 运行状态 ---
            // 运行中按下按键 2：一键急停并返回待机
            if (Read_Button2() == 1)
            {
                is_running = 0;
                Motor_ForceStop();
            }
        }

        // ==================== 2. 任务执行逻辑 ====================
        if (is_running)
        {
            int trigger_count = GetGraySensorTriggerCount();
            EncoderDataUpdate();
            float error = ReadGraySensor();

            // --- 任务 1 逻辑 ---
            if (current_mode == TASK_MODE_1)
            {
                ms_timer_enable = 1; // 启动计时器
                if (printflag == 0)
                {
                    PRINT_SendString("#1!");
                    printflag = 1;
                }
                if (stop_state == 0)
                    Motor_setspeed_soft(400, 0);
                Motor_calcu_w(-1.0f * error);
                if (stop_state == 1)
                {
                    ms_timer_enable = 0; // 启动计时器
                    count++;
                    if (count >= count_max)
                    {
                        is_running = 0;
                        stop_state = 0;
                        Motor_setspeed_soft(0, 0);
                        count = 0;
                    }
                }
                if (trigger_count >= 4)
                {
                    // is_running = 0;
                    stop_state = 1;
                    // vTaskDelay(pdMS_TO_TICKS(150));
                    // Motor_setspeed_soft(0, 0);
                }
            }
            // --- 任务 2 逻辑 ---
            else if (current_mode == TASK_MODE_2)//来回走
            {
                // ms_timer_enable=1; // 启动计时器
                if (printflag == 0)
                {
                    PRINT_SendString("#3!");
                    printflag = 1;
                }
                if (Read_Button1() == 1)
                {
                    PRINT_SendString("#2!");
                }
            }
            // --- 任务 3 逻辑 ---
            else if (current_mode == TASK_MODE_3)//中心跑
            {
                
                if (printflag == 0)
                {
                    PRINT_SendString("#3!");
                    printflag = 1;
                }
                if (Read_Button1() == 1)
                {
                    task_wait = 1;
                    //PRINT_SendString("#4!");
                    //ms_timer_enable = 1; // 启动计时器
                }
                if (task_wait == 1)
                {
                    if (stop_state == 0)
                        Motor_setspeed_soft(280, 1600);
                    Motor_calcu_w(-1.0f * error);
                    if (trigger_count >= 4)
                    {
                        ms_timer_enable = 0;
                        // is_running = 0;
                        stop_state = 1;
                        // vTaskDelay(pdMS_TO_TICKS(150));
                        Motor_setspeed_soft(0, 1800);
                    }
                }
            }
            // --- 任务 4 逻辑 ---
            else if (current_mode == TASK_MODE_4)//中心跑
            {
                if (printflag == 0)
                {
                    PRINT_SendString("#4!");
                    printflag = 1;
                }
                if (Read_Button1() == 1)
                {
                    task_wait = 1;
                    //PRINT_SendString("#Go!");
                    ms_timer_enable = 1; // 启动计时器
                }
                if (task_wait == 1)
                {
                    if (stop_state == 0)
                        Motor_setspeed_soft(280, 1600);
                    Motor_calcu_w(-1.0f * error);
                    if (trigger_count >= 4)
                    {
                    PRINT_SendString("#5!");    ms_timer_enable = 0;
                        // is_running = 0;
                        stop_state = 1;
                        // vTaskDelay(pdMS_TO_TICKS(150));
                        Motor_setspeed_soft(0, 1800);
                    }
                }
            }
            // --- 任务 5 逻辑 ---
            else if (current_mode == TASK_MODE_5)
            {

                if (printflag == 0)
                {
                    PRINT_SendString("#5!");
                    printflag = 1;
                }
                if (Read_Button1() == 1)
                {
                    task_wait = 1;
                    PRINT_SendString("#9!");
                    ms_timer_enable = 1; // 启动计时器
                }
                if (task_wait == 1)
                {
                    if (stop_state == 0)
                        Motor_setspeed_soft(380, 3000);
                    Motor_calcu_w(error * -1.0f);

                    if (trigger_count >= 4)
                    {
                        ms_timer_enable = 0; // 启动计时器
                        // is_running = 0;
                        stop_state = 1;
                        // vTaskDelay(pdMS_TO_TICKS(150));
                        Motor_setspeed_soft(0, 1800);
                    }
                }
            }
            else if (current_mode == TASK_MODE_6)
            {
                
            }
        }
        else if (stop_state == 1)
        {
            // 减速缓冲阶段
            float error = ReadGraySensor();
            Motor_calcu_w(error);
            if (Motor_IsStopped())
            {
                stop_state = 2;
            }
        }
        else if (stop_state == 2)
        {
            Motor_ForceStop();
        }

        // 底层数据与软启动更新
        Motor_SoftStart_Update();
        MotorDataUpdate();

        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz 运行周期
        if (ms_timer_enable == 1)
            ms++;
    }
}

/**
 * @brief OLED 屏幕显示任务
 */
void OLED_task(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // 上电稳定缓冲
    OLED_Init();
    OLED_Clear();

    // 绘制固定 UI 框架
    OLED_ShowString(0, 0, (uint8_t *)"Task:", 16);
    OLED_ShowString(0, 2, (uint8_t *)"State:", 16);
    OLED_ShowString(0, 4, (uint8_t *)"Ms :", 16);
    OLED_ShowString(0, 6, (uint8_t *)"Ready:", 16);
    while (1)
    {
        // 1. 动态显示选中的任务 (MODE_1 ~ MODE_5)
        switch (current_mode)
        {
        case TASK_MODE_1:
            OLED_ShowString(48, 0, (uint8_t *)"MODE_1", 16);
            break;
        case TASK_MODE_2:
            OLED_ShowString(48, 0, (uint8_t *)"MODE_2", 16);
            break;
        case TASK_MODE_3:
            OLED_ShowString(48, 0, (uint8_t *)"MODE_3", 16);
            break;
        case TASK_MODE_4:
            OLED_ShowString(48, 0, (uint8_t *)"MODE_4", 16);
            break;
        case TASK_MODE_5:
            OLED_ShowString(48, 0, (uint8_t *)"MODE_5", 16);
            break;
        case TASK_MODE_6:
            OLED_ShowString(48, 0, (uint8_t *)"TEST", 16);
            break;
        default:
            OLED_ShowString(48, 0, (uint8_t *)"IDLE  ", 16);
            break;
        }

        // 2. 动态显示运行状态 (RUN / STOP)
        if (is_running)
        {
            OLED_ShowString(56, 2, (uint8_t *)"RUN ", 16);
        }
        else
        {
            OLED_ShowString(56, 2, (uint8_t *)"STOP", 16);
        }
        
        if (task_wait == 1)
        {
            OLED_ShowString(56, 6, (uint8_t *)"Yes ", 16);
        }
        else
        {
            OLED_ShowString(56, 6, (uint8_t *)"No", 16);
        }


        // 3. 动态显示灰度传感器计数
        OLED_ShowNum(56, 4, ms, 6, 16);

        vTaskDelay(pdMS_TO_TICKS(500)); // 2Hz 刷新频率
    }
}

void ins_task()
{
    while (bsp_Icm42688Init() == -1)
    {
        Delay_long(500);
    }
    while (1)
    {
        get_ICM_data();
        current_angle = Yaw_AngleLast;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
int main(void)
{
    SYSCFG_DL_init();
    DL_TimerG_startCounter(TIMER_1ms_INST);
    NVIC_EnableIRQ(TIMER_1ms_INST_INT_IRQN);
    BaseType_t ret = xTaskCreate(
        main_task,
        "Task_1000Hz",
        5000,
        NULL,
        tskIDLE_PRIORITY + 2,
        &main_hld);

    //    BaseType_t ret_1 = xTaskCreate(
    //        ins_task,
    //        "INS",
    //        4096,
    //        NULL,
    //        tskIDLE_PRIORITY + 3,
    //        &ins_hld
    //    );

    BaseType_t ret_11 = xTaskCreate(
        OLED_task,
        "oled",
        512,
        NULL,
        tskIDLE_PRIORITY + 3,
        &OLED_hld);

    vTaskStartScheduler();

    while (1)
    {
    }
}
