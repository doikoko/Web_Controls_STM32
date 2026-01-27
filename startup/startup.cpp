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
    if (!Queue::instance().count) return;
    
    for (uint8_t i = 0; i < MAX_QUEUE_MEMBERS; i++){
        if(Queue::instance().processes[i]){
            Queue::instance().processes[i]();
            Queue::instance().count--;
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

extern uint32_t _sdata, _edata, _sbss, _ebss, _etext;

// entry point
extern "C" void reset_handler(){
    // zero bss
    uint32_t* sdata = &_sdata; 
    uint32_t* edata = &_edata; 
    uint32_t* sbss = &_sbss; 
    uint32_t* ebss = &_ebss;
    uint32_t* etext = &_etext;
    
    uint32_t data_size = (uint32_t)edata - (uint32_t)sdata;
    uint32_t bss_size = (uint32_t)ebss - (uint32_t)sbss;
    
    for(uint32_t i = 0; i < bss_size; i += sizeof(uint32_t)){
        *(sbss + i) = 0;
    }
    
    // copy data from FLASH to SRAM
    for(uint32_t i = 0; i < data_size; i += sizeof(uint32_t)){
        *(sdata + i) = *(etext + i);
    }

    RCC rcc;
    
    NVIC nvic;
    TIM tim2(2);

    Queue::instance().spawn_task(reinterpret_cast<queue_func>(main));
    Queue::init(3000000, nvic, tim2, rcc);
}

