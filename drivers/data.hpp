#pragma once

#define KILOBYTE 1024

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
