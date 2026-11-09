/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G351X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G351X
#define CONFIG_MSPM0G3519

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for LED_Color */
#define LED_Color_INST                                                     TIMG6
#define LED_Color_INST_IRQHandler                               TIMG6_IRQHandler
#define LED_Color_INST_INT_IRQN                                 (TIMG6_INT_IRQn)
#define LED_Color_INST_CLK_FREQ                                         80000000
/* GPIO defines for channel 0 */
#define GPIO_LED_Color_C0_PORT                                             GPIOA
#define GPIO_LED_Color_C0_PIN                                     DL_GPIO_PIN_29
#define GPIO_LED_Color_C0_IOMUX                                   (IOMUX_PINCM4)
#define GPIO_LED_Color_C0_IOMUX_FUNC                  IOMUX_PINCM4_PF_TIMG6_CCP0
#define GPIO_LED_Color_C0_IDX                                DL_TIMER_CC_0_INDEX

/* Defines for PWM_Motor */
#define PWM_Motor_INST                                                     TIMG0
#define PWM_Motor_INST_IRQHandler                               TIMG0_IRQHandler
#define PWM_Motor_INST_INT_IRQN                                 (TIMG0_INT_IRQn)
#define PWM_Motor_INST_CLK_FREQ                                         40000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_Motor_C0_PORT                                             GPIOA
#define GPIO_PWM_Motor_C0_PIN                                     DL_GPIO_PIN_12
#define GPIO_PWM_Motor_C0_IOMUX                                  (IOMUX_PINCM34)
#define GPIO_PWM_Motor_C0_IOMUX_FUNC                 IOMUX_PINCM34_PF_TIMG0_CCP0
#define GPIO_PWM_Motor_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_Motor_C1_PORT                                             GPIOA
#define GPIO_PWM_Motor_C1_PIN                                     DL_GPIO_PIN_13
#define GPIO_PWM_Motor_C1_IOMUX                                  (IOMUX_PINCM35)
#define GPIO_PWM_Motor_C1_IOMUX_FUNC                 IOMUX_PINCM35_PF_TIMG0_CCP1
#define GPIO_PWM_Motor_C1_IDX                                DL_TIMER_CC_1_INDEX




/* Defines for QEI_1 */
#define QEI_1_INST                                                         TIMG8
#define QEI_1_INST_IRQHandler                                   TIMG8_IRQHandler
#define QEI_1_INST_INT_IRQN                                     (TIMG8_INT_IRQn)
/* Pin configuration defines for QEI_1 PHA Pin */
#define GPIO_QEI_1_PHA_PORT                                                GPIOC
#define GPIO_QEI_1_PHA_PIN                                         DL_GPIO_PIN_6
#define GPIO_QEI_1_PHA_IOMUX                                     (IOMUX_PINCM84)
#define GPIO_QEI_1_PHA_IOMUX_FUNC                    IOMUX_PINCM84_PF_TIMG8_CCP0
/* Pin configuration defines for QEI_1 PHB Pin */
#define GPIO_QEI_1_PHB_PORT                                                GPIOC
#define GPIO_QEI_1_PHB_PIN                                         DL_GPIO_PIN_7
#define GPIO_QEI_1_PHB_IOMUX                                     (IOMUX_PINCM85)
#define GPIO_QEI_1_PHB_IOMUX_FUNC                    IOMUX_PINCM85_PF_TIMG8_CCP1

/* Defines for QEI_0 */
#define QEI_0_INST                                                         TIMG9
#define QEI_0_INST_IRQHandler                                   TIMG9_IRQHandler
#define QEI_0_INST_INT_IRQN                                     (TIMG9_INT_IRQn)
/* Pin configuration defines for QEI_0 PHA Pin */
#define GPIO_QEI_0_PHA_PORT                                                GPIOB
#define GPIO_QEI_0_PHA_PIN                                        DL_GPIO_PIN_29
#define GPIO_QEI_0_PHA_IOMUX                                     (IOMUX_PINCM66)
#define GPIO_QEI_0_PHA_IOMUX_FUNC                    IOMUX_PINCM66_PF_TIMG9_CCP0
/* Pin configuration defines for QEI_0 PHB Pin */
#define GPIO_QEI_0_PHB_PORT                                                GPIOB
#define GPIO_QEI_0_PHB_PIN                                        DL_GPIO_PIN_30
#define GPIO_QEI_0_PHB_IOMUX                                     (IOMUX_PINCM67)
#define GPIO_QEI_0_PHB_IOMUX_FUNC                    IOMUX_PINCM67_PF_TIMG9_CCP1


/* Defines for TIMER_1ms */
#define TIMER_1ms_INST                                                   (TIMA0)
#define TIMER_1ms_INST_IRQHandler                               TIMA0_IRQHandler
#define TIMER_1ms_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define TIMER_1ms_INST_LOAD_VALUE                                         (312U)




/* Defines for I2C_0 */
#define I2C_0_INST                                                          I2C0
#define I2C_0_INST_IRQHandler                                    I2C0_IRQHandler
#define I2C_0_INST_INT_IRQN                                        I2C0_INT_IRQn
#define I2C_0_BUS_SPEED_HZ                                                100000
#define GPIO_I2C_0_SDA_PORT                                                GPIOA
#define GPIO_I2C_0_SDA_PIN                                        DL_GPIO_PIN_28
#define GPIO_I2C_0_IOMUX_SDA                                      (IOMUX_PINCM3)
#define GPIO_I2C_0_IOMUX_SDA_FUNC                       IOMUX_PINCM3_PF_I2C0_SDA
#define GPIO_I2C_0_SCL_PORT                                                GPIOA
#define GPIO_I2C_0_SCL_PIN                                        DL_GPIO_PIN_31
#define GPIO_I2C_0_IOMUX_SCL                                      (IOMUX_PINCM6)
#define GPIO_I2C_0_IOMUX_SCL_FUNC                       IOMUX_PINCM6_PF_I2C0_SCL


/* Defines for UART_Sys */
#define UART_Sys_INST                                                      UART0
#define UART_Sys_INST_FREQUENCY                                         40000000
#define UART_Sys_INST_IRQHandler                                UART0_IRQHandler
#define UART_Sys_INST_INT_IRQN                                    UART0_INT_IRQn
#define GPIO_UART_Sys_RX_PORT                                              GPIOA
#define GPIO_UART_Sys_TX_PORT                                              GPIOA
#define GPIO_UART_Sys_RX_PIN                                      DL_GPIO_PIN_11
#define GPIO_UART_Sys_TX_PIN                                      DL_GPIO_PIN_10
#define GPIO_UART_Sys_IOMUX_RX                                   (IOMUX_PINCM22)
#define GPIO_UART_Sys_IOMUX_TX                                   (IOMUX_PINCM21)
#define GPIO_UART_Sys_IOMUX_RX_FUNC                    IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_Sys_IOMUX_TX_FUNC                    IOMUX_PINCM21_PF_UART0_TX
#define UART_Sys_BAUD_RATE                                              (115200)
#define UART_Sys_IBRD_40_MHZ_115200_BAUD                                    (21)
#define UART_Sys_FBRD_40_MHZ_115200_BAUD                                    (45)
/* Defines for UART_ESP */
#define UART_ESP_INST                                                      UART3
#define UART_ESP_INST_FREQUENCY                                         80000000
#define UART_ESP_INST_IRQHandler                                UART3_IRQHandler
#define UART_ESP_INST_INT_IRQN                                    UART3_INT_IRQn
#define GPIO_UART_ESP_RX_PORT                                              GPIOA
#define GPIO_UART_ESP_TX_PORT                                              GPIOA
#define GPIO_UART_ESP_RX_PIN                                      DL_GPIO_PIN_26
#define GPIO_UART_ESP_TX_PIN                                      DL_GPIO_PIN_25
#define GPIO_UART_ESP_IOMUX_RX                                   (IOMUX_PINCM59)
#define GPIO_UART_ESP_IOMUX_TX                                   (IOMUX_PINCM55)
#define GPIO_UART_ESP_IOMUX_RX_FUNC                    IOMUX_PINCM59_PF_UART3_RX
#define GPIO_UART_ESP_IOMUX_TX_FUNC                    IOMUX_PINCM55_PF_UART3_TX
#define UART_ESP_BAUD_RATE                                                (9600)
#define UART_ESP_IBRD_80_MHZ_9600_BAUD                                     (520)
#define UART_ESP_FBRD_80_MHZ_9600_BAUD                                      (53)
/* Defines for UART_PRINT */
#define UART_PRINT_INST                                                    UART1
#define UART_PRINT_INST_FREQUENCY                                       40000000
#define UART_PRINT_INST_IRQHandler                              UART1_IRQHandler
#define UART_PRINT_INST_INT_IRQN                                  UART1_INT_IRQn
#define GPIO_UART_PRINT_RX_PORT                                            GPIOB
#define GPIO_UART_PRINT_TX_PORT                                            GPIOC
#define GPIO_UART_PRINT_RX_PIN                                     DL_GPIO_PIN_7
#define GPIO_UART_PRINT_TX_PIN                                     DL_GPIO_PIN_0
#define GPIO_UART_PRINT_IOMUX_RX                                 (IOMUX_PINCM24)
#define GPIO_UART_PRINT_IOMUX_TX                                 (IOMUX_PINCM74)
#define GPIO_UART_PRINT_IOMUX_RX_FUNC                  IOMUX_PINCM24_PF_UART1_RX
#define GPIO_UART_PRINT_IOMUX_TX_FUNC                  IOMUX_PINCM74_PF_UART1_TX
#define UART_PRINT_BAUD_RATE                                            (115200)
#define UART_PRINT_IBRD_40_MHZ_115200_BAUD                                  (21)
#define UART_PRINT_FBRD_40_MHZ_115200_BAUD                                  (45)




/* Defines for SPI_0 */
#define SPI_0_INST                                                         SPI0
#define SPI_0_INST_IRQHandler                                   SPI0_IRQHandler
#define SPI_0_INST_INT_IRQN                                       SPI0_INT_IRQn
#define GPIO_SPI_0_PICO_PORT                                              GPIOB
#define GPIO_SPI_0_PICO_PIN                                      DL_GPIO_PIN_17
#define GPIO_SPI_0_IOMUX_PICO                                   (IOMUX_PINCM43)
#define GPIO_SPI_0_IOMUX_PICO_FUNC                   IOMUX_PINCM43_PF_SPI0_PICO
#define GPIO_SPI_0_POCI_PORT                                              GPIOB
#define GPIO_SPI_0_POCI_PIN                                      DL_GPIO_PIN_19
#define GPIO_SPI_0_IOMUX_POCI                                   (IOMUX_PINCM45)
#define GPIO_SPI_0_IOMUX_POCI_FUNC                   IOMUX_PINCM45_PF_SPI0_POCI
/* GPIO configuration for SPI_0 */
#define GPIO_SPI_0_SCLK_PORT                                              GPIOB
#define GPIO_SPI_0_SCLK_PIN                                      DL_GPIO_PIN_18
#define GPIO_SPI_0_IOMUX_SCLK                                   (IOMUX_PINCM44)
#define GPIO_SPI_0_IOMUX_SCLK_FUNC                   IOMUX_PINCM44_PF_SPI0_SCLK



/* Port definition for Pin Group Button */
#define Button_PORT                                                      (GPIOB)

/* Defines for button: GPIOB.31 with pinCMx 68 on package pin 27 */
#define Button_button_PIN                                       (DL_GPIO_PIN_31)
#define Button_button_IOMUX                                      (IOMUX_PINCM68)
/* Port definition for Pin Group LED_Board */
#define LED_Board_PORT                                                   (GPIOA)

/* Defines for Blue: GPIOA.14 with pinCMx 36 on package pin 43 */
#define LED_Board_Blue_PIN                                      (DL_GPIO_PIN_14)
#define LED_Board_Blue_IOMUX                                     (IOMUX_PINCM36)
/* Defines for Orange: GPIOA.17 with pinCMx 39 on package pin 54 */
#define LED_Board_Orange_PIN                                    (DL_GPIO_PIN_17)
#define LED_Board_Orange_IOMUX                                   (IOMUX_PINCM39)
/* Defines for PIN1: GPIOA.16 with pinCMx 38 on package pin 45 */
#define GraySensor_PIN1_PORT                                             (GPIOA)
#define GraySensor_PIN1_PIN                                     (DL_GPIO_PIN_16)
#define GraySensor_PIN1_IOMUX                                    (IOMUX_PINCM38)
/* Defines for PIN2: GPIOA.22 with pinCMx 47 on package pin 62 */
#define GraySensor_PIN2_PORT                                             (GPIOA)
#define GraySensor_PIN2_PIN                                     (DL_GPIO_PIN_22)
#define GraySensor_PIN2_IOMUX                                    (IOMUX_PINCM47)
/* Defines for PIN3: GPIOC.2 with pinCMx 76 on package pin 50 */
#define GraySensor_PIN3_PORT                                             (GPIOC)
#define GraySensor_PIN3_PIN                                      (DL_GPIO_PIN_2)
#define GraySensor_PIN3_IOMUX                                    (IOMUX_PINCM76)
/* Defines for PIN4: GPIOC.3 with pinCMx 77 on package pin 51 */
#define GraySensor_PIN4_PORT                                             (GPIOC)
#define GraySensor_PIN4_PIN                                      (DL_GPIO_PIN_3)
#define GraySensor_PIN4_IOMUX                                    (IOMUX_PINCM77)
/* Defines for PIN5: GPIOC.4 with pinCMx 78 on package pin 52 */
#define GraySensor_PIN5_PORT                                             (GPIOC)
#define GraySensor_PIN5_PIN                                      (DL_GPIO_PIN_4)
#define GraySensor_PIN5_IOMUX                                    (IOMUX_PINCM78)
/* Defines for PIN6: GPIOC.5 with pinCMx 79 on package pin 53 */
#define GraySensor_PIN6_PORT                                             (GPIOC)
#define GraySensor_PIN6_PIN                                      (DL_GPIO_PIN_5)
#define GraySensor_PIN6_IOMUX                                    (IOMUX_PINCM79)
/* Defines for PIN7: GPIOB.3 with pinCMx 16 on package pin 19 */
#define GraySensor_PIN7_PORT                                             (GPIOB)
#define GraySensor_PIN7_PIN                                      (DL_GPIO_PIN_3)
#define GraySensor_PIN7_IOMUX                                    (IOMUX_PINCM16)
/* Defines for PIN8: GPIOB.2 with pinCMx 15 on package pin 18 */
#define GraySensor_PIN8_PORT                                             (GPIOB)
#define GraySensor_PIN8_PIN                                      (DL_GPIO_PIN_2)
#define GraySensor_PIN8_IOMUX                                    (IOMUX_PINCM15)
/* Defines for PIN9: GPIOC.8 with pinCMx 86 on package pin 65 */
#define GraySensor_PIN9_PORT                                             (GPIOC)
#define GraySensor_PIN9_PIN                                      (DL_GPIO_PIN_8)
#define GraySensor_PIN9_IOMUX                                    (IOMUX_PINCM86)
/* Defines for PIN10: GPIOC.9 with pinCMx 87 on package pin 66 */
#define GraySensor_PIN10_PORT                                            (GPIOC)
#define GraySensor_PIN10_PIN                                     (DL_GPIO_PIN_9)
#define GraySensor_PIN10_IOMUX                                   (IOMUX_PINCM87)
/* Defines for PIN11: GPIOB.23 with pinCMx 51 on package pin 70 */
#define GraySensor_PIN11_PORT                                            (GPIOB)
#define GraySensor_PIN11_PIN                                    (DL_GPIO_PIN_23)
#define GraySensor_PIN11_IOMUX                                   (IOMUX_PINCM51)
/* Defines for PIN12: GPIOB.6 with pinCMx 23 on package pin 30 */
#define GraySensor_PIN12_PORT                                            (GPIOB)
#define GraySensor_PIN12_PIN                                     (DL_GPIO_PIN_6)
#define GraySensor_PIN12_IOMUX                                   (IOMUX_PINCM23)
/* Defines for CS: GPIOA.24 with pinCMx 54 on package pin 73 */
#define ICM_CS_PORT                                                      (GPIOA)
#define ICM_CS_PIN                                              (DL_GPIO_PIN_24)
#define ICM_CS_IOMUX                                             (IOMUX_PINCM54)
/* Defines for INT: GPIOB.27 with pinCMx 58 on package pin 77 */
#define ICM_INT_PORT                                                     (GPIOB)
#define ICM_INT_PIN                                             (DL_GPIO_PIN_27)
#define ICM_INT_IOMUX                                            (IOMUX_PINCM58)
/* Defines for BUTTON1: GPIOC.1 with pinCMx 75 on package pin 47 */
#define MESS_BUTTON1_PORT                                                (GPIOC)
#define MESS_BUTTON1_PIN                                         (DL_GPIO_PIN_1)
#define MESS_BUTTON1_IOMUX                                       (IOMUX_PINCM75)
/* Defines for BUTTON2: GPIOB.8 with pinCMx 25 on package pin 32 */
#define MESS_BUTTON2_PORT                                                (GPIOB)
#define MESS_BUTTON2_PIN                                         (DL_GPIO_PIN_8)
#define MESS_BUTTON2_IOMUX                                       (IOMUX_PINCM25)
/* Defines for BEEP: GPIOB.22 with pinCMx 50 on package pin 69 */
#define MESS_BEEP_PORT                                                   (GPIOB)
#define MESS_BEEP_PIN                                           (DL_GPIO_PIN_22)
#define MESS_BEEP_IOMUX                                          (IOMUX_PINCM50)
/* Defines for Motor_DirA1: GPIOA.0 with pinCMx 1 on package pin 1 */
#define MESS_Motor_DirA1_PORT                                            (GPIOA)
#define MESS_Motor_DirA1_PIN                                     (DL_GPIO_PIN_0)
#define MESS_Motor_DirA1_IOMUX                                    (IOMUX_PINCM1)
/* Defines for Motor_DirA2: GPIOA.1 with pinCMx 2 on package pin 2 */
#define MESS_Motor_DirA2_PORT                                            (GPIOA)
#define MESS_Motor_DirA2_PIN                                     (DL_GPIO_PIN_1)
#define MESS_Motor_DirA2_IOMUX                                    (IOMUX_PINCM2)
/* Defines for Motor_DirB1: GPIOB.4 with pinCMx 17 on package pin 20 */
#define MESS_Motor_DirB1_PORT                                            (GPIOB)
#define MESS_Motor_DirB1_PIN                                     (DL_GPIO_PIN_4)
#define MESS_Motor_DirB1_IOMUX                                   (IOMUX_PINCM17)
/* Defines for Motor_DirB2: GPIOB.5 with pinCMx 18 on package pin 21 */
#define MESS_Motor_DirB2_PORT                                            (GPIOB)
#define MESS_Motor_DirB2_PIN                                     (DL_GPIO_PIN_5)
#define MESS_Motor_DirB2_IOMUX                                   (IOMUX_PINCM18)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_LED_Color_init(void);
void SYSCFG_DL_PWM_Motor_init(void);
void SYSCFG_DL_QEI_1_init(void);
void SYSCFG_DL_QEI_0_init(void);
void SYSCFG_DL_TIMER_1ms_init(void);
void SYSCFG_DL_I2C_0_init(void);
void SYSCFG_DL_UART_Sys_init(void);
void SYSCFG_DL_UART_ESP_init(void);
void SYSCFG_DL_UART_PRINT_init(void);
void SYSCFG_DL_SPI_0_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
