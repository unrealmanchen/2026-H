#include "motor.h"
#include "pid.h"
#include "ti_msp_dl_config.h"

#define PWM_PERIOD  1000                    /* PWM 最大占空比 (LOAD-1)                   */
static int target_straight_speed = 0;   
static float current_straight_speed = 0.0f; 
static float speed_ramp_step = 0.0f;  


#define LimitMax(input, max) \
  {                          \
    if (input > max)         \
    {                        \
      input = max;           \
    }                        \
    else if (input < -max)   \
    {                        \
      input = -max;          \
    }                        \
  }




#define ENCODER_LINES     14000.0f  /* 一圈脉冲数      */
#define TIRE_RADIUS_MM     67.0f    /* 轮胎半径 mm     */
#define ENCODER_READ_FREQ   1000.0f    /* 读取频率 Hz     */

#include "pid.h"

typedef struct
{
    volatile int EncoderCount; /* 脉冲累计 (ISR 写, 主循环读), volatile 防优化 */
    int LastCount;             /* 上一周期锁存的脉冲数 (调试用)                */
    float Speed;               /* 线速度 (mm/s)                                */
    float Distance;            /* 累计行驶距离 (mm)                            */
} Encoder_t;

typedef struct
{
    int MotorL;         
    Encoder_t EncoderL; 
    pid_t PidSpeedL;

    int MotorR;
    Encoder_t EncoderR;
    pid_t PidSpeedR;

    pid_t w;
		pid_t angle;
    int straight_speed;
} motor_t;

motor_t Motor_INS = {
    .MotorL = 0,
    .MotorR = 0,
    .straight_speed =100
};
void Motor_setspeed(int speed)
{
    Motor_INS.straight_speed = speed;
	  Motor_INS.MotorL = 0;
    Motor_INS.MotorR = 0;
}



int Motor_IsStopped(void) {
    return (Motor_INS.straight_speed == 0);
}

void Motor_ForceStop(void) {
    Motor_INS.straight_speed = 0;
    Motor_INS.MotorL = 0;
    Motor_INS.MotorR = 0;
}

int motor_soft_flag = 0;

void Motor_setspeed_soft(int target_speed, int ramp_time_ms)
{
    motor_soft_flag = 1;
    target_straight_speed = target_speed;
    
    // No soft start
    if (ramp_time_ms == 0) {
        motor_soft_flag = 0;
        current_straight_speed = (float)target_speed;
        Motor_INS.straight_speed = target_speed;
        return;
    }

    speed_ramp_step = (float)(target_speed - current_straight_speed) / (float)ramp_time_ms;
}


void Motor_SoftStart_Update(void)
{
    
    if (current_straight_speed != (float)target_straight_speed)
    {
        if (speed_ramp_step > 0.0f) 
        {
            current_straight_speed += speed_ramp_step;
            if (current_straight_speed >= target_straight_speed)
            {
                current_straight_speed = (float)target_straight_speed; 
            }
        }
        else 
        {
            current_straight_speed += speed_ramp_step;
            if (current_straight_speed <= target_straight_speed)
            {
                current_straight_speed = (float)target_straight_speed;
            }
        }
        
    
        Motor_INS.straight_speed = (int)current_straight_speed;
    }
}




void Motor_control_angle_spin(float target_angle, float current_angle)
{
    float angle_err = target_angle - current_angle;
    while (angle_err > 180.0f)  angle_err -= 360.0f;
    while (angle_err < -180.0f) angle_err += 360.0f;

    
    float target_w = PID_calc(&Motor_INS.angle, current_angle, current_angle + angle_err);

    Motor_INS.straight_speed = 0; 
 
    Motor_calcu_w1(target_w);
}

extern float Yaw_AngleLast;
void Motor_calcu_w1(float w)
{
  
    float w_current = Yaw_AngleLast;
   
    float w_output = PID_calc(&Motor_INS.w, w_current, w);
    
    LimitMax(w_output, PWM_PERIOD);

    int speed0 = Motor_INS.straight_speed + (int)w_output; 
    int speed1 = Motor_INS.straight_speed - (int)w_output; 
 
    LimitMax(speed0, PWM_PERIOD);
    LimitMax(speed1, PWM_PERIOD);

    Motor_INS.MotorL = speed0;
    Motor_INS.MotorR = speed1;
}


float Kp = 20.0f;
void Motor_calcu_w(float w){
    float w_set = Kp * w;
    float w_current = (Motor_INS.EncoderL.Speed - Motor_INS.EncoderR.Speed) / 120.0f;
    float w_output = PID_calc(&Motor_INS.w, w_set, w_current);
    LimitMax(w_output, 1000.0f);
    if(motor_soft_flag == 1){
        w_output = w_output /260.0f * Motor_INS.straight_speed;
    }
    

    int speed[2]; //1左 0右
    speed[0]=Motor_INS.straight_speed+w_output;
    speed[1]=Motor_INS.straight_speed-w_output;
    
    if(speed[0]>PWM_PERIOD){
        speed[1]-=speed[0]-PWM_PERIOD;
        speed[0]=PWM_PERIOD ;
    }else if(speed[1]>PWM_PERIOD){
        speed[0]-=speed[1]-PWM_PERIOD;
        speed[1]=PWM_PERIOD;  
    }else if(speed[0]<0){
        speed[1]+= -1*speed[0];
        speed[0]=0;
    }else if(speed[1]<0){
        speed[0]+= -1*speed[1];
        speed[1]=0;
    }
    Motor_INS.MotorL = speed[0];
    Motor_INS.MotorR = speed[1];

}
void EncodersInit(void);
void Motor_init(void){
    PID_init(&Motor_INS.w, 0.9f, 0.003000007f,0.0f,0,300,50
        ,2);
		PID_init(&Motor_INS.angle, -100.0f, 0, 0, 0.0f, 500.0f, 0.0f, 10.0f);
    PID_init(&Motor_INS.PidSpeedL, 0, 0, 0, 0, 0, 0, 100000);
    PID_init(&Motor_INS.PidSpeedR, 0, 0, 0, 0, 0, 0, 100000);
    /* 电机 PWM 初始化 */
    EncodersInit();
    DL_GPIO_setPins(MESS_Motor_DirA1_PORT, MESS_Motor_DirA1_PIN);
    DL_GPIO_setPins(MESS_Motor_DirA2_PORT, MESS_Motor_DirA2_PIN);
    DL_GPIO_setPins(MESS_Motor_DirB1_PORT, MESS_Motor_DirB1_PIN);
    DL_GPIO_setPins(MESS_Motor_DirB2_PORT, MESS_Motor_DirB2_PIN);

    DL_TimerA_startCounter(PWM_Motor_INST);    
}


void MotorDataUpdate(void)
{   
        
    static int pwm[2];

    motor_t *Motor = &Motor_INS;
    if(Motor->MotorL > 0){
        DL_GPIO_setPins(MESS_Motor_DirA1_PORT, MESS_Motor_DirA1_PIN);
        DL_GPIO_clearPins(MESS_Motor_DirA2_PORT, MESS_Motor_DirA2_PIN);
        pwm[0] = Motor->MotorL;
    }else{
        DL_GPIO_clearPins(MESS_Motor_DirA1_PORT, MESS_Motor_DirA1_PIN);
        DL_GPIO_setPins(MESS_Motor_DirA2_PORT, MESS_Motor_DirA2_PIN);
        pwm[0] = Motor->MotorL * -1;
    }


    if(Motor->MotorR < 0){
        DL_GPIO_clearPins(MESS_Motor_DirB1_PORT, MESS_Motor_DirB1_PIN);
        DL_GPIO_setPins(MESS_Motor_DirB2_PORT, MESS_Motor_DirB2_PIN);
        pwm[1] = Motor->MotorR * -1;
    }else{
        DL_GPIO_setPins(MESS_Motor_DirB1_PORT, MESS_Motor_DirB1_PIN);
        DL_GPIO_clearPins(MESS_Motor_DirB2_PORT, MESS_Motor_DirB2_PIN);
        pwm[1] = Motor->MotorR;
    }

                                            
    if (pwm[0] > PWM_PERIOD) pwm[0] = PWM_PERIOD;
    if (pwm[1] > PWM_PERIOD) pwm[1] = PWM_PERIOD;

    if (pwm[0] < 100 &&pwm[0] != 0 ) pwm[0] = 100;
    if (pwm[1] < 100 &&pwm[1] != 0 ) pwm[1] = 100;

    /* PWM 输出 (反相: period - duty)                                      */
    DL_TimerG_setCaptureCompareValue(PWM_Motor_INST,
        (uint16_t)(PWM_PERIOD - pwm[0]), GPIO_PWM_Motor_C0_IDX);
    DL_TimerG_setCaptureCompareValue(PWM_Motor_INST,
        (uint16_t)(PWM_PERIOD - pwm[1]), GPIO_PWM_Motor_C1_IDX);
    return;
}

static void EncoderInit(Encoder_t * e)
{
    e->EncoderCount = 0;  
    e->LastCount = 0;
    e->Speed = 0.0f;     
    e->Distance = 0.0f;
}

void EncodersInit(void)
{
    motor_t *Motors = &Motor_INS;
    static Encoder_t encL, encR;
    Motors->EncoderL = encL;
    Motors->EncoderR = encR;

    NVIC_EnableIRQ(GPIOB_INT_IRQn);         /* 开 NVIC 中断门 */
    EncoderInit(&Motors->EncoderL);
    EncoderInit(&Motors->EncoderR);
}



static uint32_t motorL_last_qei = 0;
void EncoderDataUpdate(void)
{
    int pL, pR;
    float coeff;
    motor_t *Motors = &Motor_INS;
    /* 临界区: 关中断, 读走 + 清零脉冲 (防止 ISR 半途改写) */
    __disable_irq();
    

    uint32_t current_count = DL_Timer_getTimerCount(QEI_0_INST);
    int32_t delta = (int32_t)(current_count - Motors->EncoderL.LastCount);
    Motors->EncoderL.EncoderCount = delta;
    Motors->EncoderL.LastCount = current_count;

     current_count = DL_Timer_getTimerCount(QEI_1_INST);
	delta = (int32_t)(current_count - Motors->EncoderR.LastCount);
    Motors->EncoderR.EncoderCount = delta;
    Motors->EncoderR.LastCount = current_count;


    pL = Motors->EncoderL.EncoderCount;
    pR = Motors->EncoderR.EncoderCount;

    __enable_irq();

    /* 脉冲 → 线速度 (mm/s) */
    coeff = (2.0f * 3.14159265f * TIRE_RADIUS_MM * ENCODER_READ_FREQ)
            / ENCODER_LINES;

    Motors->EncoderL.Speed    = (float)pL * coeff;
    Motors->EncoderR.Speed    = (float)pR * coeff;

    /* 累计行驶距离 */
    Motors->EncoderL.Distance += Motors->EncoderL.Speed / ENCODER_READ_FREQ;
    Motors->EncoderR.Distance += Motors->EncoderR.Speed / ENCODER_READ_FREQ;
}



// void GROUP1_IRQHandler(void)
// {
// 	uint32_t groupFlag = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

//     uint32_t gpio_interrupt1, gpio_interrupt2;

//     /* 读左编码器中断状态 (PB13=A相, PB15=B相) */
//     gpio_interrupt1 = DL_GPIO_getEnabledInterruptStatus(GPIOB,
//         Encoder1_EA1_PIN | Encoder1_EB1_PIN);

//     /* 读右编码器中断状态 (PB14=A相, PB16=B相) */
//     gpio_interrupt2 = DL_GPIO_getEnabledInterruptStatus(GPIOB,
//         Encoder2_EA2_PIN | Encoder2_EB2_PIN);

//     /*---- 左轮 ----*/
//     if ((gpio_interrupt1 & Encoder1_EA1_PIN) == Encoder1_EA1_PIN)
//     {
//         /* PB13 边沿 → 读 PB15 判方向 */
//         if (!DL_GPIO_readPins(Encoder1_PORT, Encoder1_EB1_PIN))
//             Motor_INS.EncoderL.EncoderCount--;
//         else
//             Motor_INS.EncoderL.EncoderCount++;
//     }
//     else if ((gpio_interrupt1 & Encoder1_EB1_PIN) == Encoder1_EB1_PIN)
//     {
//         /* PB15 边沿 → 读 PB13 判方向 */
//         if (!DL_GPIO_readPins(Encoder1_PORT, Encoder1_EA1_PIN))
//             Motor_INS.EncoderL.EncoderCount++;
//         else
//             Motor_INS.EncoderL.EncoderCount--;
//     }

//     /*---- 右轮 ----*/
//     if ((gpio_interrupt2 & Encoder2_EA2_PIN) == Encoder2_EA2_PIN)
//     {
//         /* PB14 边沿 → 读 PB16 判方向 */
//         if (!DL_GPIO_readPins(Encoder2_PORT, Encoder2_EB2_PIN))
//             Motor_INS.EncoderR.EncoderCount--;
//         else
//             Motor_INS.EncoderR.EncoderCount++;
//     }
//     else if ((gpio_interrupt2 & Encoder2_EB2_PIN) == Encoder2_EB2_PIN)
//     {
//         /* PB16 边沿 → 读 PB14 判方向 */
//         if (!DL_GPIO_readPins(Encoder2_PORT, Encoder2_EA2_PIN))
//             Motor_INS.EncoderR.EncoderCount++;
//         else
//             Motor_INS.EncoderR.EncoderCount--;
//     }

//     /* 统一清除中断标志 */
//     DL_GPIO_clearInterruptStatus(GPIOB,
//         Encoder1_EA1_PIN | Encoder1_EB1_PIN);
//     DL_GPIO_clearInterruptStatus(GPIOB,
//         Encoder2_EA2_PIN | Encoder2_EB2_PIN);
// }
