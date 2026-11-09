#include "led/timerG6_pwm_rgb.h"
#include "ti_msp_dl_config.h"
#include "string.h"
#define TimerG6_PWM_INST LED_Color_INST
#define TimerG6_PWM_INST_IRQHandler LED_Color_INST_IRQHandler
#define TimerG6_PWM_INST_INT_IRQN LED_Color_INST_INT_IRQN

/*PWM频率800k*/
void timerg6_pwm_rgb_init(void)
{

	DL_TimerG_startCounter(TimerG6_PWM_INST);
	NVIC_EnableIRQ(TimerG6_PWM_INST_INT_IRQN);
}

int RGB_num = 1;
struct
{
	uint8_t G7;
	uint8_t G6;
	uint8_t G5;
	uint8_t G4;
	uint8_t G3;
	uint8_t G2;
	uint8_t G1;
	uint8_t G0;

	uint8_t R7;
	uint8_t R6;
	uint8_t R5;
	uint8_t R4;
	uint8_t R3;
	uint8_t R2;
	uint8_t R1;
	uint8_t R0;

	uint8_t B7;
	uint8_t B6;
	uint8_t B5;
	uint8_t B4;
	uint8_t B3;
	uint8_t B2;
	uint8_t B1;
	uint8_t B0;
} RGB_data;
int H=50;
int L =75;
void set_RGB(uint8_t G, uint8_t R, uint8_t B)
{
	memset(&RGB_data, L, sizeof(RGB_data));
	if (G & 0x80)
		RGB_data.G7 = H;
	if (G & 0x40)
		RGB_data.G6 = H;
	if (G & 0x20)
		RGB_data.G5 = H;
	if (G & 0x10)
		RGB_data.G4 = H;
	if (G & 0x08)
		RGB_data.G3 = H;
	if (G & 0x04)
		RGB_data.G2 = H;
	if (G & 0x02)
		RGB_data.G1 = H;
	if (G & 0x01)
		RGB_data.G0 = H;

	if (R & 0x80)
		RGB_data.R7 = H;
	if (R & 0x40)
		RGB_data.R6 = H;
	if (R & 0x20)
		RGB_data.R5 = H;
	if (R & 0x10)
		RGB_data.R4 = H;
	if (R & 0x08)
		RGB_data.R3 = H;
	if (R & 0x04)
		RGB_data.R2 = H;
	if (R & 0x02)
		RGB_data.R1 = H;
	if (R & 0x01)
		RGB_data.R0 = H;

	if (B & 0x80)
		RGB_data.B7 = H;
	if (B & 0x40)
		RGB_data.B6 = H;
	if (B & 0x20)
		RGB_data.B5 = H;
	if (B & 0x10)
		RGB_data.B4 = H;
	if (B & 0x08)
		RGB_data.B3 = H;
	if (B & 0x04)
		RGB_data.B2 = H;
	if (B & 0x02)
		RGB_data.B1 = H;
	if (B & 0x01)
		RGB_data.B0 = H;
	NVIC_EnableIRQ(TimerG6_PWM_INST_INT_IRQN);
	//DL_TimerG_startCounter(TimerG6_PWM_INST);
}



void TimerG6_PWM_INST_IRQHandler(void)
{
	if (RGB_num >= 26)
		RGB_num--;
	else if (RGB_num == 25)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G7, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 24)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G6, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 23)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G5, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 22)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G4, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 21)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G3, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 20)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G2, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 19)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G1, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 18)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.G0, DL_TIMER_CC_0_INDEX), RGB_num--;

	else if (RGB_num == 17)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R7, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 16)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R6, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 15)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R5, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 14)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R4, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 13)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R3, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 12)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R2, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 11)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R1, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 10)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.R0, DL_TIMER_CC_0_INDEX), RGB_num--;

	else if (RGB_num == 9)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B7, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 8)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B6, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 7)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B5, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 6)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B4, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 5)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B3, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 4)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B2, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 3)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B1, DL_TIMER_CC_0_INDEX), RGB_num--;
	else if (RGB_num == 2)
		DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, RGB_data.B0, DL_TIMER_CC_0_INDEX), RGB_num--;

	else if (RGB_num == 1){
        DL_TimerG_setCaptureCompareValue(TimerG6_PWM_INST, 99, DL_TIMER_CC_0_INDEX);
        RGB_num = 100;
            NVIC_DisableIRQ(TimerG6_PWM_INST_INT_IRQN);
    }
}


