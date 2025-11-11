# Web_Controls_STM32

DEMONSTRATION: (./demonstration.mp4)
This project made for stmf401C

This project providing ability of programming your MCU
in runtime using web interface and my own drivers

This project contains:
1) Drivers      (./drivers/) 
2) Bootloader   (./startup/startupt.c)
3) Web side     (./pc/src/ && ./web/)

Start:
    you need:
        - cargo
        - arm-none-eabi
        - make
        - stm32f401C
        - USART converter

make: build project
make clean: clean project
make test: flash MCU
make test_pc: run local web server
