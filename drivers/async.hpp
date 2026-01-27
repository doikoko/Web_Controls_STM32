#pragma once

#include "data.hpp"
#include "tim.hpp"
#include "nvic.hpp"
#include "dev_board_periphy.hpp"

typedef void(*queue_func)();

/// @brief if (pid_t == -1) error
typedef signed char pid_t;

class Queue{
    Queue() {
        for(int i = 0; i < MAX_QUEUE_MEMBERS; i++)
            processes[i] = nullptr;
    };

public:
    volatile uint8_t count{};
    volatile queue_func processes[MAX_QUEUE_MEMBERS]{};
    
    static Queue& instance() {
        static Queue inst;
        return inst;    
    }
    
    /// @brief if (pid_t == -1) error
    static pid_t spawn_task(queue_func task){
        for (uint8_t i = 0; i < MAX_QUEUE_MEMBERS; i++)
            if (!Queue::instance().processes[i]){
            RCC rcc;

            LED led = { 13, 'C' };
            
            rcc.config_pll(7, 4, 336, 16);
            led.clock_enable(rcc);
            led.set_output_mode();
            led.enable_push_pull();
            led.set_speed(GpioSpeed::Three);
            led.no_pull_up_down();
            led.enable_light();
                Queue::instance().count++;
                Queue::instance().processes[i] = task;
                return i;
            }
        return -1;
    }

    static void kill_task(pid_t pid){
        if (pid < 0) return;
        if (!Queue::instance().processes[pid]) return;
        Queue::instance().processes[pid] = nullptr;
        Queue::instance().count--;
    }

    /// @brief function initialize tim2 for async work
    /// @param time time between interrupts for switch current task in milliseconds
    static void init(uint32_t time_ms, NVIC& nvic, TIM& tim2, RCC& rcc){
        tim2.clock_enable(rcc);

        tim2.registers->cr1 = 0;
        tim2.registers->cr2 = 0;
        tim2.registers->smcr = 0;
        tim2.registers->dier = 0;

        tim2.registers->psc = 84000 - 1;     // 84MHz -> 1kHz
        tim2.registers->arr = time_ms - 1;   // period in ms
        tim2.registers->cnt = 0;

        tim2.registers->sr = 0;

        tim2.registers->dier |= 1;
        tim2.registers->egr |= 1;
        
        nvic.enable_interrupt(28);
        tim2.start();
    }

    Queue operator=(const Queue&) = delete;
    Queue(const Queue&) = delete;
};

// this class provides memory buffer which can use all tasks
class TasksMemory{
    volatile uint8_t raw[MAX_MEM_SIZE]{};
    
    TasksMemory() = default;

public:
    static TasksMemory& instance(){
        static TasksMemory inst;
        return inst;
    }

    /// 1 error, 0 success
    uint8_t append(void* data, uint16_t len, uint16_t pos){
        if (pos + len >= MAX_MEM_SIZE) return 1;
        
        for (int i = 0; i < len; i++)
            if (raw[pos + i]) return 1;

        for(int i = 0; i < len; i++){
            raw[pos + i] = *(reinterpret_cast<uint8_t*>(data) + pos);
        }

        return 0;
    }

    uint8_t get(void* buf, uint16_t len, uint16_t pos){
        if (pos + len >= MAX_MEM_SIZE) return 1;
        
        for (int i = 0; i < len; i++){
            *(reinterpret_cast<uint8_t*>(buf)) = raw[pos + i];
            raw[pos + i] = 0;
        }
    }

    TasksMemory operator=(const TasksMemory&) = delete;
    TasksMemory(const TasksMemory&) = delete;
};

