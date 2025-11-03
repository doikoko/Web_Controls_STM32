#pragma once

#include "gpio.hpp"

class LED final : public GPIO{
public:    
    LED(uint8_t num, uint8_t letter) : GPIO(num, letter){}
    
    void enable_light(){
        registers->odr &= ~(1 << 13);
    }    

    void disable_light(){
        registers->odr |= 1 << 13;
    }    

    void blink(){
        registers->odr ^= 1 << 13;
    }    
};    

class Button final : public GPIO{
public:    
    Button(uint8_t num, uint8_t letter) : GPIO(num, letter){}

    /// @brief  on my device 1 - pressed
    bool is_pressed() const {
        return read_data();
    }
};