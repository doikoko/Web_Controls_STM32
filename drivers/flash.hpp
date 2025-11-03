#pragma once

#include "data.hpp"


enum class ProgramSize{ Eight, Sixteen, ThirtyTwo, SixtyFour };
typedef struct {
    volatile uint32_t acr;
    volatile uint32_t keyr;
    volatile uint32_t optkeyr;
    volatile uint32_t sr;
    volatile uint32_t cr;
    volatile uint32_t optcr;
} Flash_Reg;

class Flash final{
public:
    Flash_Reg* registers;

    Flash() : registers (reinterpret_cast<Flash_Reg*>(FLASH_BASE)) {}

    void unlock_cr_register(){
        registers->keyr = 0x45670123;
        registers->keyr = 0xCDEF89AB;
    }

    void unlock_user_configuration_sector(){
        registers->optkeyr = 0x08192A3B;
        registers->optkeyr = 0x4C5D6E7F;
    }

    uint32_t get_status() const {
        return registers->sr;
    }

    void enable_interrupts(){
        while ((registers->cr & (1 << 31)) != 0);
        registers->cr |= (1 << 25);
    }

    void disable_interrupts(){
        while ((registers->cr & (1 << 31)) != 0);
        registers->cr &= ~(1 << 25);
    }

    void start_erasing(){
        while ((registers->cr & (1 << 31)) != 0);
        registers->cr |= (1 << 16);
    }

    
    void set_program_size(ProgramSize size){
        while ((registers->cr & (1 << 31)) != 0);
        
        registers->cr &= ~(0b11 << 8);

        switch (size){
            case ProgramSize::Sixteen:   registers->cr |= (0b01 << 8); return;
            case ProgramSize::ThirtyTwo: registers->cr |= (0b10 << 8); return;
            case ProgramSize::SixtyFour: registers->cr |= (0b11 << 8); return;
            default:        return;
        }
    }

    /// @brief if you check documentation you can see
    ///         that sector count can be 6 and 7 (depends on device)
    ///         or additional settings, but this driver written for 
    ///         stm32F401CC.. (my MCU)
    /// @param sector_count must be 0..5 inclusive
    /// @return 1 if sector count bigger than 5 (error) or 0 if ok
    uint8_t set_sector_count(uint8_t sector_count){
        if(sector_count > 5) 
            return 1;
        
        registers->cr &= ~(0xF);
        registers->cr |= sector_count << 3;

        return 0;
    }

    void mass_erase(){
        registers->cr |= 0b100;
    }

    void sector_erase(){
        registers->cr |= 0b10;
    }

    void programming(){
        registers->cr |= 0b1;
    }
};