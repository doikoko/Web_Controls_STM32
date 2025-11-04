#pragma once

#include "data.hpp"
#include "usart.hpp"

class User{
    typedef uint32_t(*user_func)();  
    uint8_t code[KILOBYTE * 8];
    uint32_t returned_value;

    void clear(){
        volatile uint32_t *copy = (uint32_t*)code;
        for(int i = 0; i < KILOBYTE * 2; i++){
            copy[i] = 0;
        }
    }
public:
    User() {
        clear();
    }

    void recv_code(USART& usart){
        clear();
        usart.clear_data_reg();
        usart.sync_read_buf(code, KILOBYTE * 8);
    }

    void call(){
        user_func func = reinterpret_cast<user_func>(code);
        returned_value = func();
    }

    void send_returned_value(USART& usart){
        usart.sync_write_buf((void*)&returned_value, 1);
    }
};