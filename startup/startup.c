#define uintptr_t unsigned int
#define uint32_t unsigned int

#define SRAM_START                  0x20000000U
#define SRAM_SIZE                   64U * 1024U //64K
#define SRAM_END                    SRAM_START + SRAM_SIZE
#define STACK_POINTER_FIRST_ADDR    ((uint32_t)SRAM_END)

#define VECTOR_TABLE_SIZE_WORDS     255

void reset_handler(void);
void nmi_handler(void)  __attribute((weak, alias("default_handler")));
void hard_fault_handler(void)   __attribute((weak, alias("default_handler")));
void memory_management_fault_handler(void)  __attribute((weak, alias("default_handler")));
void bus_fault_handler(void)    __attribute((weak, alias("default_handler")));
void usage_fault_handler(void)  __attribute((weak, alias("default_handler")));
void svcall_handler(void)   __attribute((weak, alias("default_handler")));
void pend_sv_handler(void)  __attribute((weak, alias("default_handler")));
void systick_handler(void)  __attribute((weak, alias("default_handler")));

void default_handler(void){ while(1){ asm("wfi"); } }

static volatile uintptr_t isr_vector[VECTOR_TABLE_SIZE_WORDS] 
__attribute__((section(".isr_vector"))) = {
    STACK_POINTER_FIRST_ADDR,
	(uintptr_t)&reset_handler,
	(uintptr_t)&nmi_handler,
	(uintptr_t)&hard_fault_handler,
	(uintptr_t)&memory_management_fault_handler,
	(uintptr_t)&bus_fault_handler,
	(uintptr_t)&usage_fault_handler,
	(uintptr_t)0,
	(uintptr_t)0,
	(uintptr_t)0,
	(uintptr_t)0,
	(uintptr_t)&svcall_handler,
	(uintptr_t)0,
	(uintptr_t)0,
	(uintptr_t)&pend_sv_handler,
	(uintptr_t)&systick_handler
};

void main(void);

extern uintptr_t _sdata, _edata, _sbss, _ebss, _etext;
// entry point
void reset_handler(void){
    // zero bss
    uintptr_t* sdata = &_sdata; 
    uintptr_t* edata = &_edata; 
    uintptr_t* sbss = &_sbss; 
    uintptr_t* ebss = &_ebss;
    uintptr_t* etext = &_etext;
    
    uintptr_t data_size = (uintptr_t)edata - (uintptr_t)sdata;
    uintptr_t bss_size = (uintptr_t)ebss - (uintptr_t)sbss;
    
    for(uintptr_t i = 0; i < bss_size; i += sizeof(uintptr_t)){
        *(sbss + i) = 0;
    }
    
    // copy data from FLASH to SRAM
    for(uintptr_t i = 0; i < data_size; i += sizeof(uintptr_t)){
        *(sdata + i) = *(etext + i);
    }
    
    main();
}

