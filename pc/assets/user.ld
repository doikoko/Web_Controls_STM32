ENTRY(loader)

SECTIONS {
    . = 0x0;
    .text : {
        *(.text*) 
    }

    . = 0x790;
    .data : {
        *(.data*)
    }

    . = 0xf04;
    .rodata : {
        *(.rodata*)
    }

    . = 0xf81;
    .data : {
        *(.bss*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
    }
}