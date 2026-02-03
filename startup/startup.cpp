#include "../drivers/async.hpp"
#include "../drivers/dev_board_periphy.hpp"

#define SRAM_START                  0x20000000U
#define SRAM_SIZE                   64U * 1024U //64K
#define SRAM_END                    SRAM_START + SRAM_SIZE
#define STACK_POINTER_FIRST_ADDR    ((uint32_t)SRAM_END)

#define VECTOR_TABLE_SIZE_WORDS     255

extern "C" void reset_handler();
extern "C" void nmi_handler()  __attribute((weak, alias("default_handler")));
extern "C" void hard_fault_handler()   __attribute((weak, alias("default_handler")));
extern "C" void memory_management_fault_handler()  __attribute((weak, alias("default_handler")));
extern "C" void bus_fault_handler()    __attribute((weak, alias("default_handler")));
extern "C" void usage_fault_handler()  __attribute((weak, alias("default_handler")));
extern "C" void svcall_handler()   __attribute((weak, alias("default_handler")));
extern "C" void pend_sv_handler()  __attribute((weak, alias("default_handler")));
extern "C" void systick_handler()  __attribute((weak, alias("default_handler")));

extern "C" void default_handler(){ while(1){ asm("wfi"); } }

int main();

// for async
extern "C" void tim2_int_handle(){   
    BLINK();
    if (!queue.count) return;
    
    for (uint8_t i = 0; i < MAX_QUEUE_MEMBERS; i++){
        if(queue.processes[i]){
            queue.processes[i]();
            queue.count--;
            
            return;
        }
    }
}

static volatile uint32_t isr_vector[VECTOR_TABLE_SIZE_WORDS]
__attribute__((section(".isr_vector"))) = {
    STACK_POINTER_FIRST_ADDR,             
    (uint32_t)&reset_handler,
    (uint32_t)&nmi_handler,
    (uint32_t)&hard_fault_handler,
    (uint32_t)&memory_management_fault_handler,
    (uint32_t)&bus_fault_handler,
    (uint32_t)&usage_fault_handler,
    0,0,0,0,
    (uint32_t)&svcall_handler,
    0,0,
    (uint32_t)&pend_sv_handler,
    (uint32_t)&systick_handler,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    (uint32_t)&tim2_int_handle  
};

extern "C" uint32_t _sdata, _edata, _sbss, _ebss, _etext;
extern "C" uint32_t _squeue, _etasks_mem;

//extern "C" Queue queue; 
//extern "C" TasksMemory tasks_mem; 

// entry point
extern "C" void reset_handler(){
    // zero bss
    volatile uint32_t* sdata = &_sdata; 
    volatile uint32_t* edata = &_edata; 
    volatile uint32_t* sbss = &_sbss; 
    volatile uint32_t* ebss = &_ebss;
    volatile uint32_t* etext = &_etext;
    
    volatile uint32_t* squeue = reinterpret_cast<uint32_t*>(&_squeue);
    volatile uint32_t* etasks_mem = reinterpret_cast<uint32_t*>(&_etasks_mem);

    volatile uint32_t data_size = 
        reinterpret_cast<uint32_t>(edata) - reinterpret_cast<uint32_t>(sdata);
    
    volatile uint32_t bss_size = 
        reinterpret_cast<uint32_t>(ebss) - reinterpret_cast<uint32_t>(sbss);

    volatile uint32_t queue_tasks_size = 
        reinterpret_cast<uint32_t>(etasks_mem) - reinterpret_cast<uint32_t>(squeue);
        
    for(uint32_t i = 0; i < bss_size / sizeof(uint32_t); i++){
        *(sbss + i) = 0;
    }
    
    // copy data from FLASH to SRAM
    for(uint32_t i = 0; i < data_size / sizeof(uint32_t); i++){
        *(sdata + i) = *(etext + i);
    }

    for(uint32_t i = 0; i < queue_tasks_size / sizeof(uint32_t); i++){
        *(squeue + i) = 0;
    }
        
    RCC rcc;
    
    NVIC nvic;
    TIM tim2(2);

    CONF();
    
    queue.spawn_task(reinterpret_cast<queue_func>(main));
    queue.init(1000, nvic, tim2, rcc);
}