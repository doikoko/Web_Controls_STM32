ENTRY(reset_handler)

MEMORY{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
SECTIONS{
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH
    
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.rodata)
        . = ALIGN(4);
        _etext = .;
    } > FLASH
    
    .data : {    
        _sdata = .;
        *(.data)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
}