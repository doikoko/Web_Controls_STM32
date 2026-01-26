#pragma once

#include "data.hpp"

typedef struct {
    volatile uint32_t iser[16];
    volatile uint32_t icer[16];
    volatile uint32_t ispr[16];
    volatile uint32_t icpr[16];
    volatile uint32_t iabr[16];
    volatile uint32_t reserved1[47];
    volatile uint32_t ipr[123];
    volatile uint32_t reserved2[451];
} NVIC_Reg;

class NVIC final{
public:
    NVIC_Reg* registers;

    NVIC() : registers(reinterpret_cast<NVIC_Reg*>(NVIC_BASE)){}

    uint8_t get_interrupts_count(){
        return ((*reinterpret_cast<uint32_t*>(ICTR_BASE) & 0b1111) + 1) * 32;
    }

    void trigger_interrupt(uint8_t num){
        num %= get_interrupts_count();

        *reinterpret_cast<uint32_t*>(STIR_BASE) |= num;
    }

    uint8_t enable_interrupt(uint16_t num){
        if(num > (32 * 15)) return 1;
        registers->iser[num / 32] |= 1 << (num % 32);

        return 0;
    }
    
    uint8_t disable_interrupt(uint16_t num){
        if(num > (32 * 15)) return 1;
        registers->icer[num / 32] = 1 << (num % 32);

        return 0;
    }
    
    bool is_active(uint16_t interrupt){
        if(interrupt > (32 * 15)) return false;
        return registers->iabr[interrupt / 32] & (1 << (interrupt % 32));
    }

    uint8_t set_priority(uint16_t num, uint8_t priority){
        if(num > (32 * 15) && priority > 15) return 1;
        registers->ipr[num / 4] |= (priority << (8 * (num % 4) + 4));

        return 0;
    }
};
