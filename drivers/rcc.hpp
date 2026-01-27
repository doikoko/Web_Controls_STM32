#pragma once

#include "data.hpp"
#include "flash.hpp"

typedef struct {
    volatile uint32_t cr;
    volatile uint32_t pllcfgr;
    volatile uint32_t cfgr;
    volatile uint32_t cir;
    volatile uint32_t ahb1rstr;
    volatile uint32_t ahb2rstr;
    volatile uint32_t reserved0[2];
    volatile uint32_t apb1rstr;
    volatile uint32_t apb2rstr;
    volatile uint32_t reserved1[2];
    volatile uint32_t ahb1enr;
    volatile uint32_t ahb2enr;
    volatile uint32_t reserved2[2];
    volatile uint32_t apb1enr;
    volatile uint32_t apb2enr;
    volatile uint32_t reserved3[2];
    volatile uint32_t rcc_ahb1lpenr;
    volatile uint32_t rcc_ahb2lpenr;
    volatile uint32_t reserved4[2];
    volatile uint32_t rcc_apb1lpenr;
    volatile uint32_t rcc_apb2lpenr;
    volatile uint32_t reserved5[2];
    volatile uint32_t rcc_bdcr;
    volatile uint32_t rcc_csr;
    volatile uint32_t reserved6[2];
    volatile uint32_t rcc_sscgr;
    volatile uint32_t rcc_plli2scfgr;
    volatile uint32_t reserved;
    volatile uint32_t rcc_dckcfgr;
} RCC_Reg;        

class RCC final{
public:    
    RCC_Reg* registers;

    RCC() : registers(reinterpret_cast<RCC_Reg*>(RCC_BASE)) {}

    void enable_pll(){
        registers->cr |= 1 << 24;
    }    
    
    void diasble_pll(){
        registers->cr &= ~(1 << 24);
    }    

    bool is_locked() const {
        return registers->cr & 1 << 25;
    }    

    void enable_hsi(){
        registers->cr |= 1;
        while(((registers->cr >> 1) & 1) != 1);
    }    

    void switch_to_pll(){
        registers->cfgr &= ~0b11;
        registers->cfgr |= 0b10;
        while(((registers->cfgr >> 2) & 0b11) != 0b10);
    }    

    /// @brief 7, 4, 336, 16
    uint8_t config_pll(uint8_t pllq, uint8_t pllp, uint32_t plln, uint16_t pllm){
        enable_hsi();
        diasble_pll();
        while(is_locked());
        
        registers->pllcfgr = 0;
        registers->pllcfgr |= (pllq & 0xF) << 24;
        registers->pllcfgr |= (plln & 0x1FF) << 6;
        registers->pllcfgr |= pllm & 0x3F;
        registers->pllcfgr &= ~(1 << 22);
        
        switch(pllp) {
            case 2:  registers->pllcfgr |= (0b00 << 16); break;
            case 4:  registers->pllcfgr |= (0b01 << 16); break;
            case 6:  registers->pllcfgr |= (0b10 << 16); break;
            case 8:  registers->pllcfgr |= (0b11 << 16); break;
            default: return 1;
        }    

        enable_pll();
        
        Flash_Reg* flash = reinterpret_cast<Flash_Reg*>(FLASH_BASE);
        flash->acr &= ~3;
        flash->acr |= 3;
        
        while(!is_locked());
        switch_to_pll();

        return 0;
    }    
};    
