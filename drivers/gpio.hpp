#pragma once

#include "data.hpp"
#include "rcc.hpp"

enum class GpioSpeed { Zero, One, Two, Three };
typedef struct {
    volatile uint32_t moder;
    volatile uint32_t otyper;
    volatile uint32_t ospeedr;
    volatile uint32_t pupdr;
    volatile uint32_t idr;
    volatile uint32_t odr;
    volatile uint32_t bsrr;
    volatile uint32_t lckr;
    volatile uint32_t afrl;
    volatile uint32_t afrh;
} GPIO_Reg;    

class GPIO{
protected:    
    uint8_t num;
    uint8_t letter;
public:    
    GPIO_Reg* registers;

    GPIO(){};
    GPIO(uint8_t num, uint8_t letter) : 
        registers(reinterpret_cast<GPIO_Reg*>(
            GPIO_BASE + (0x400 * (letter - 'A'))
        )) {
            this->num = num;
            this->letter = letter;
    }    

    void clock_enable(RCC& rcc) const {
        if(letter >= 'A' && letter <= 'E')
            rcc.registers->ahb1enr |= 1 << (letter - 'A');
        else if (letter == 'H')    
            rcc.registers->ahb1enr |= 1 << 7;
    }    
        
    void set_input_mode(){
        registers->moder &= ~(0x3 << (2 * num));
    }    

    void set_output_mode(){
        registers->moder &= ~(0x3 << (2 * num));
        registers->moder |= 1 << (2 * num);
    }    

    void set_alt_function_mode(){
        registers->moder &= ~(0x3 << (2 * num));
        registers->moder |= 0b10 << (2 * num);
    }

    void enable_push_pull(){
        registers->otyper &= ~(1 << num);
    }    

    void enable_open_drain(){
        registers->otyper |= (1 << num);
    }    

    uint32_t read_data() const {
        return registers->idr & (1 << num);
    }    

    void set_pull_up(){
        registers->pupdr &= ~(0x3 << (num * 2));
        registers->pupdr |= 1 << (num * 2);
    }    

    void set_pull_down(){
        registers->pupdr &= ~(0x3 << (num * 2));
        registers->pupdr |= 0x2 << (num * 2);
    }    

    void set_speed(GpioSpeed speed){
        registers->ospeedr &= ~(0x3 << (2 * num));
        registers->ospeedr |= static_cast<uint8_t>(speed) << (2 * num);
    }    
    
    void no_pull_up_down(){
        registers->pupdr &= ~(0x3 << (2 * num));
    }    

    void set_alt_function(uint8_t function_num){
        constexpr uint8_t max_func = 16;

        if (function_num >= max_func) return;
        if (num < 8) registers->afrl |= function_num << (num * 4);
        else if (num < max_func) registers->afrh |= function_num << ((num - 8) * 4);
    }
};    
