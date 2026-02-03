#pragma once

#include "data.hpp"
#include "tim.hpp"
#include "nvic.hpp"
#include "dev_board_periphy.hpp"


class Queue;
class TasksMemory;

typedef void(*queue_func)();

/// @brief if (pid_t == -1) error
typedef signed char pid_t;

class Queue{  
public:
    Queue() = default; 

    volatile uint8_t count;
    volatile queue_func processes[MAX_QUEUE_MEMBERS];

    /// @brief if (pid_t == -1) error
    pid_t spawn_task(queue_func task){
        for (uint8_t i = 0; i < MAX_QUEUE_MEMBERS; i++)
            if (processes[i] == nullptr){
                count++;
                processes[i] = task;
                return i;
            }
            
        return -1;
    }

    void kill_task(pid_t pid){
        if (pid < 0) return;
        if (!processes[pid]) return;
        processes[pid] = nullptr;
        count--;
    }

    /// @brief function initialize tim2 for async work
    /// @param time time between interrupts for switch current task in milliseconds
    void init(uint32_t time_ms, NVIC& nvic, TIM& tim2, RCC& rcc){
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
};

static Queue queue __attribute__((section(".queue")));

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

