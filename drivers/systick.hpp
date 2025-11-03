#pragma once

#include "data.hpp"

typedef struct {
    volatile uint32_t csr;
    volatile uint32_t rvr;
    volatile uint32_t cvr;
    volatile uint32_t calib;
} Systick_Reg;

class Systick final{
public:
    Systick_Reg* registers;

    Systick() : registers(reinterpret_cast<Systick_Reg*>(SYSTICK_BASE)){};

    bool is_end() const{
        return (registers->csr & (1 << 16));
    }

    void set_is_proc_clock(bool is_proc_clock){
        if(is_proc_clock) registers->csr |= (1 << 2);
        else registers->csr &= ~(1 << 2);
    }

    void set_is_interrupt(bool is_interrupt){
        if(is_interrupt) registers->csr |= (1 << 1);
        else registers->csr &= ~(1 << 1);
    }

    void start(){
        registers->csr |= 1;
    }

    void stop(){
        registers->csr &= ~1;
    }

    void set_ticks(uint32_t ticks){
        ticks &= 0x00FFFFFF;
        registers->rvr = ticks - 1;
    }

    uint32_t get_current_value(){
        return registers->cvr;
    }

    void delay_ms_interrupt(uint32_t milliseconds){
        set_is_proc_clock(true);
        set_is_interrupt(true);
        set_ticks(84000 * milliseconds - 1);

        start();
    }

    /// @brief extreme simple delay function (for my own use)
    void delay(uint32_t milliseconds){
        set_is_proc_clock(true);
        set_is_interrupt(false);
        set_ticks(84000 * 10 - 1);
        registers->cvr = 0;
        start();

        for(uint32_t count = 0; count < milliseconds / 10; count++)
            while(!is_end());
            
        stop();
    }
};
