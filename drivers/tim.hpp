#pragma once

#include "data.hpp"    
#include "rcc.hpp"
#include "nvic.hpp"

enum Direction{ UP, DOWN };
typedef struct {
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t smcr;
    volatile uint32_t dier;
    volatile uint32_t sr;
    volatile uint32_t egr;
    volatile uint32_t ccmr1;
    volatile uint32_t ccmr2;
    volatile uint32_t ccer;
    volatile uint32_t cnt;
    volatile uint32_t psc;
    volatile uint32_t arr;
    volatile uint32_t ccr1;
    volatile uint32_t ccr2;
    volatile uint32_t ccr3;
    volatile uint32_t ccr4;
    volatile uint32_t reserved;
    volatile uint32_t dcr;
    volatile uint32_t dmar;
    volatile uint32_t tim2;
} TIM_Reg;    

class TIM final{
public:    
    TIM_Reg* registers;
    
    TIM(uint8_t num){
        if(num >= 2 && num <= 5)
        registers = reinterpret_cast<TIM_Reg*>(TIM_BASE + (0x400 * (num - 2)));
    }    
 
    void delay(uint32_t milliseconds){
        stop();
        init();
        start();
        while(registers->cnt < milliseconds);
        stop();
    }    
    void clock_enable(RCC& rcc){
        rcc.registers->apb1enr |= 1;
    }    
    
    void stop(){
        registers->cr1 &= ~1;
    }    
    void start(){
        registers->cr1 |= 1;
    }    

private:    
    void init(){
        registers->cr1 = 0;
        registers->cr2 = 0;
        registers->smcr = 0;
        registers->dier = 0;
        registers->psc = 84000 - 1;
        registers->arr = UINT32_T_MAX;
        registers->cnt = 0;
        registers->egr = 1;
    }    
};