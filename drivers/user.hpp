#pragma once

#include "data.hpp"
#include "usart.hpp"
#include "mpu.hpp"

class User{
    typedef uint32_t(*user_func)();  
    user_func func;
    uint8_t *code;
    uint32_t returned_value;

    void clear(){
        volatile uint32_t *copy = (uint32_t*)code;
        for(uint16_t i = 0; i < KILOBYTE * 2; i++){
            copy[i] = 0;
        }
    }
public:
    User(uint8_t *code) {
        this->code = code;
        func = reinterpret_cast<user_func>(reinterpret_cast<uint32_t>(code) | 1);

        MPU mpu;

        mpu.disable();
        asm volatile("dsb");
        asm volatile("isb");
        mpu.create_region(reinterpret_cast<uint32_t>(code), 3, 13);
        mpu.is_executed(true);
        mpu.set_permitions(Permitions::FullAccess);
        mpu.disable_interrupts();
        mpu.disable_cache();
        mpu.enable_access();
        mpu.enable_region();
        asm volatile("dsb");
        asm volatile("isb");
        mpu.enable();

        clear();
    }

    void recv_code(USART& usart){
        clear();
        usart.sync_read_buf(code);
    }

    void call(){
        returned_value = func();
    }

    void send_returned_value(USART& usart){
        usart.sync_write_buf((void*)&returned_value, 1);
    }
};