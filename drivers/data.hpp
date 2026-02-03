#pragma once

#define KILOBYTE 1024

#define MAX_QUEUE_MEMBERS   5
#define MAX_MEM_SIZE        64

#define RCC_BASE        0x40023800
#define GPIO_BASE       0x40020000
#define TIM_BASE        0x40000000
#define FLASH_BASE      0x40023C00
#define SYSTICK_BASE    0xE000E010
#define NVIC_BASE       0xE000E100
#define ICTR_BASE       0xE000E004
#define STIR_BASE       0xE000EF00
#define USART1_BASE     0x40011000
#define MPU_BASE        0xE000ED90
#define ADC_BASE        0x40012000
#define ADC_CCR_BASE    ADC_BASE + 0x4

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define UINT32_T_MAX 0xFFFFFFFF

#define LIGHT() {\
    RCC rcc;\
    LED led = { 13, 'C' };\
    rcc.config_pll(7, 4, 336, 16);\
    led.clock_enable(rcc);\
    led.set_output_mode();\
    led.enable_push_pull();\
    led.set_speed(GpioSpeed::Three);\
    led.no_pull_up_down();\
}
#define BLINK() {\
    LED led = { 13, 'C' };\
    led.blink();\
}
#define CONF() {\
    RCC rcc;\
    LED led = { 13, 'C' };\
    rcc.config_pll(7, 4, 336, 16);\
    led.clock_enable(rcc);\
    led.set_output_mode();\
    led.enable_push_pull();\
    led.set_speed(GpioSpeed::Three);\
    led.no_pull_up_down();\
}
