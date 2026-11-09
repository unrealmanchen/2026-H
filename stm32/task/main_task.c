#include "cmsis_os.h"
#include "filter.h"
#include "bsp.h"
#include "pid.h"
#include "bsp_led.h"
#include "main_task.h"
#include "usart.h"
#include "motor.h"
#include "string.h"
#include "usart.h"
#include "bsp_dwt.h"

typedef int flag;
#include "math.h"
osThreadId main_task_hld;
osThreadId ins_task_hld;

QueueHandle_t imu_data_Queue;

flag data_flag;
flag stop_flag = 0;
volatile char send[30];
void VOFA_send(float *data, int num)
{
    int total_bytes = (num + 1) * 4;

    // 先拷贝所有 float 数据
    memcpy(send, data, num * 4);

    // 添加帧尾
    send[total_bytes - 4] = 0x00;
    send[total_bytes - 3] = 0x00;
    send[total_bytes - 2] = 0x80;
    send[total_bytes - 1] = 0x7f;

    HAL_UART_Transmit_DMA(&huart1, (char *)send, total_bytes);
}
float absfloat(float a)
{
    float b = a;
    if (b < 0)
        b *= -1.0f;
    return b;
}
char buffer[50];
char buffer4[50];
pid_t accel, speed, axis;
Filter QD_angle;
float alpha, beta, sigma;
int definate_axis;
void main_task_init()
{

    LED_toggle(RED, 0);
    LED_toggle(BLUE, 0);
    LED_toggle(GREEN, 0);

    imu_data_Queue = xQueueCreate(1, sizeof(float) * 1);
    osThreadDef(maintask, main_task, osPriorityNormal, 0, 256);
    main_task_hld = osThreadCreate(osThread(maintask), NULL);

    // osThreadDef(ins_task, INS_Task_instance, osPriorityAboveNormal, 0, 4096);
    // ins_task_hld = osThreadCreate(osThread(ins_task), NULL);

    // if(imu_data_Queue == NULL || main_task_hld == NULL || ins_task_hld == NULL)
    // {
    //     LED_toggle(BLUE,1);
    //     Error_Handler();
    // }

    // PID_init(&yaw_deg, 0.13, 0.000006, 0, 0, 100, 30, 1000);
    // PID_init(&pitch_deg, 0.04, 0, 0, 0, 100, 30, 1000);w_speed
    PID_init(&accel, 1.2, 0, 1, 0, 2, 1.5, 0.02);
    PID_init(&speed, 1.3, 0.000900000043, 0, 0, 50, 35, 0.2);
    PID_init(&axis, 1, 0, 0, 0, 15, 0, 1);
    alpha = 0.1f;
    beta = -350.0f;
    sigma = -40.f;
}

//////中心点+-5cm
// PID_init(&accel,1.2,0,1,0,2,1.5,0.02);
// PID_init(&speed,1.3,0.000900000043,0,0,50,35,0.2);
// PID_init(&axis,1,0,0,0,15,0,1);
// alpha=0.1f;
// beta = -250.0f;
// sigma = -40.f;

//     QD_angle = LowPassFilter_Create_A(0.4);
// Axis_f = LowPassFilter_Create_A(0.3);
// Speed_f_ = LowPassFilter_Create_A(0.6);
float quene_data[5] = {0};

float deg_rotate;
float ins_init[2] = {0, 0};

float a, b, c;
float axis__, speed__, accel__;
float angle_abs, length_short, length_long, cos__, cos_alpha;
float qd_angle = 0;
Filter Axis_f, Speed_f_;

float func(float x)
{
    if (x <= 166.0f)
    {
        return 0.6f;
    }
    else if (x < 220.0f)
    {
        float t = (x - 166.0f) / 54.0f; // 220 - 166 = 54
        return 0.6f + 0.6f * t * t * (3.0f - 2.0f * t);
    }
    else
    {
        return 1.0f;
    }
}
float re = 0;
void main_task()
{
    uint32_t flag = 0;
    // while (flag == 0)
    // {
    //     flag = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // }
    QD_angle = LowPassFilter_Create_A(0.5);
    Axis_f = LowPassFilter_Create_A(0.6);
    Speed_f_ = LowPassFilter_Create_A(0.6);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, buffer, sizeof(buffer));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, buffer4, sizeof(buffer4));
    //__HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

    can_filter_init();

    DWT_Init(168);
    motor_init();
    for (int i = 0; i < 2000; i++)
    {
        QD4310_deg_smooth(0);
        osDelay(1);
    }
    static int loss_count;
    while (1)
    {
        if (stop_flag == 1)
        {
            QD4310_deg_smooth(0);
            LED_toggle(GREEN, 1);
            osDelay(1);
            continue;
        }
        else
        {
            LED_toggle(GREEN, 0);
        }
        if (data_flag == 0)
        {
            LED_toggle(BLUE, 0);
            // speed__ += accel__ * 0.001;
            // axis__ += speed__ * 0.001;
            loss_count++;
        }
        else
        {
            LED_toggle(BLUE, 1);
            loss_count = 0;
        }

        if (loss_count > 5000)
        {
            LED_toggle(BLUE, 0);
            QD4310_deg_smooth(0);
            osDelay(1);
            continue;
        }

        // xQueueReceive(imu_data_Queue, quene_data, 0);
        QD4310_CMD(0, 1);

        a = PID_calc(&(axis), 0, axis__) * -1.0f;
        b = PID_calc(&(speed), speed__ * beta * -1.0f, a);
        // c = PID_calc(&(accel), accel__ * sigma, b+quene_data[0]* alpha); //9.8* sin = a    a / 9.8 *
        float accel_theta = asin(c / 9.8);
        float length = sin(accel_theta) * 22.0f;
        deg_rotate = length / (37.0f * 3.1415926f) * 360.0f;
        qd_angle = LowPassFilter_Process(QD_angle, b);
        re = func(definate_axis);
        qd_angle *= re;
        QD4310_Deg(0, qd_angle);
        data_flag = 0;

        // a = PID_calc(&(axis), 0, axis__);
        // b = PID_calc(&(speed), speed__ * beta * -1.0f, a);
        // c = PID_calc(&(accel), accel__ * sigma, b+quene_data[0]* alpha); //9.8* sin = a    a / 9.8 *
        // float accel_theta= asin(c / 9.8);
        // float length = sin(accel_theta) * 22.0f;
        // deg_rotate = length / (37.0f * 3.1415926f ) * 360.0f;
        // QD4310_Deg(0,-a);

        osDelay(1);
    }
}

uint32_t getDWTCountDx(uint32_t startTick, uint32_t stopTick)
{
    uint32_t tick;
    if (stopTick < startTick)
    {
        tick = (0xffffffff - startTick) + stopTick + 1;
    }
    else
    {
        tick = stopTick - startTick;
    }

    return tick;
}

volatile long int tick, start, stop;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {

        // start = DWT->CYCCNT;
        // stop = DWT->CYCCNT;
        // tick = getDWTCountDx(start,stop) / 168;
    }
}

int last_count = 0;
float last_axis = 0, last_speed;
float tick_s;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, buffer, sizeof(buffer));
        int a, b, d;
        float c;

        sscanf(buffer, "#b%d %d %d!", &a, &b, &d);
        definate_axis = d;
        if (a < 0)
            c = -1.0f;
        else
            c = 1.0f;
        // axis__ = sqrt(a*a + b*b) *c;
        int axis_temp = a;
        memset(buffer, 0, sizeof(buffer));

        axis_temp = LowPassFilter_Process(Axis_f, axis_temp);

        angle_abs = absfloat(qd_angle);
        length_short = (angle_abs / 360.0f * 3.14159f * 3.70f * cos_alpha);
        length_long = sqrt(25.0f * 25.0f - length_short * length_short);
        cos__ = length_long / 25.0f;

        axis_temp /= cos__;
        axis__ = axis_temp;

        if ((axis__ - last_axis) > 30)
        {
            last_axis = axis__;
            return;
        }
        start = DWT->CYCCNT;
        tick_s = getDWTCountDx(start, last_count) / 168000000.0f;
        last_count = start;
        speed__ = LowPassFilter_Process(Speed_f_, (axis__ - last_axis) / tick_s);
        ;
        accel__ = (speed__ - last_speed) / tick_s;
        last_axis = axis__;
        last_speed = speed__;
        data_flag = 1;
    }
    if (huart->Instance == UART4)
    {

        int mode = 0;
        sscanf(buffer4, "#%d!", &mode);
        switch (mode)
        {
        case 1:
            beta = -350;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"1", 1);
            break;
        case 2:
            beta = -210;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"2", 1);
						HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"2", 1);
						HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"2", 1);
            break;
        case 3:
            beta = -350;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"3", 1);
            break;
        case 4:
            beta = -350;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"4", 1);
            break;
        case 5:
            beta = -350;
            stop_flag = 1;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"5", 1);
            break;
        case 8:
            beta = -350;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"8", 1);// 调试
            break;
        case 9:
            beta = -350;
            stop_flag = 0;
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"9", 1);
            break;

        default:
            break;
        }
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, buffer4, sizeof(buffer4));
        memset(buffer4, 0, sizeof(buffer4));
    }
}