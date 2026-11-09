#ifndef CAR
#define CAR

void Motor_init(void);
void MotorDataUpdate(void);
void EncoderDataUpdate(void);
void Motor_setspeed(int speed);
void Motor_calcu_w(float w);
void Motor_control_angle_spin(float target_angle, float current_angle);
void Motor_calcu_w1(float w);
void Motor_setspeed_soft(int target_speed, int ramp_time_ms);
void Motor_SoftStart_Update(void);
int Motor_IsStopped(void);
void Motor_ForceStop(void);

extern int motor_soft_flag;
#endif
