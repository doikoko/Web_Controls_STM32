#pragma once

#include "data.hpp"

extern uint32_t _stasks_mem, _etasks_mem;

typedef void(*queue_func)();

/// @brief if (pid_t == -1) error
typedef int8_t pid_t;

class Queue{
    static Queue *inst;
    
    Queue(){
        static Queue obj __attribute__((section(".queue")));
        inst = reinterpret_cast<Queue*>(&obj);
    };

public:
    uint8_t count{};
    queue_func processes[MAX_QUEUE_MEMBERS]{};

    static Queue& instance(){
        return *inst;
    }

    /// @brief if (pid_t == -1) error
    pid_t spawn_task(queue_func task){
        for (uint8_t i = 0; i < MAX_QUEUE_MEMBERS; i++)
            if (!processes[i]){
                count++;
                processes[i] = task;
                return i;
            }
        return -1;
    }

    void kill_task(pid_t pid){
        if (!processes[pid]) return;
        processes[pid] = nullptr;
        count--;
    }

    Queue operator=(const Queue&) = delete;
    Queue(const Queue&) = delete;
};

// this class provides memory buffer which can use all tasks
class TasksMemory{
    static TasksMemory *inst;
    static uint8_t raw[MAX_MEM_SIZE];
    TasksMemory(){
        static TasksMemory obj __attribute__((section(".tasks_mem")));
        inst = reinterpret_cast<TasksMemory*>(&obj);
    }

public:
    static TasksMemory& instance(){
        return *inst;
    }



    TasksMemory operator=(const TasksMemory&) = delete;
    TasksMemory(const TasksMemory&) = delete;
};

