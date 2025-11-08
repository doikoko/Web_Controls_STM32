#pragma once 

#include "data.hpp"

enum class Permitions: uint8_t{
    NoAccess = 0b0,
    PrivelegedRWOrNone = 0b1,
    PrivelegedRWOrRO = 0b10,
    FullAccess = 0b11,
    PrivelegedROOrNone = 0b101,
    RO = 0b110
};
typedef struct {
    volatile uint32_t type;
    volatile uint32_t ctrl;
    volatile uint32_t rnr;
    volatile uint32_t rbar;
    volatile uint32_t rasr;
    volatile uint32_t rbar_a1;
    volatile uint32_t rasr_a1;
    volatile uint32_t rbar_a2;
    volatile uint32_t rasr_a2;
    volatile uint32_t rbar_a3;
    volatile uint32_t rasr_a3;
    volatile uint32_t mair0;
    volatile uint32_t mair1;
} MPU_reg;
class MPU{
    MPU_reg *registers;

    void select_region(uint8_t region_num){
        registers->rnr &= ~0xFF;
        registers->rnr |= region_num;
    }   
    
    void set_region_size(uint8_t log2_size) {
        registers->rasr &= ~(0x1F << 1);
        registers->rasr |= ((log2_size - 1) & 0x1F) << 1;
    }

public:
    MPU(): registers(reinterpret_cast<MPU_reg*>(MPU_BASE)) {}
    
    bool is_unified(){
        return !(registers->type & 1);
    }

    bool is_supported(){
        if (get_regions_count() == 0) 
            return false;
        else 
            return true;
    }

    uint8_t get_regions_count(){
        return registers->type >> 8 & 0xFF;
    }

    void enable_access(){
        registers->ctrl |= 1 << 2;
    }

    void disable_access(){
        registers->ctrl &= ~(1 << 2);
    }

    void enable_interrupts(){
        registers->ctrl |= 1 << 1;
    }

    void disable_interrupts(){
        registers->ctrl &= ~(1 << 1);
    }

    void enable(){
        registers->ctrl |= 1;
    }

    void disable(){
        registers->ctrl &= ~1;
    }
    
    void create_region(uint32_t addr, uint8_t region_num, uint8_t size_power) {
        select_region(region_num);
        registers->rbar = 0;
        registers->rbar = (addr & 0xFFFFFFE0) | (region_num & 0xF) | (1 << 4);
        set_region_size(size_power);
    }

    void is_executed(bool is_executed){
        if (is_executed)
            registers->rasr &= ~(1 << 28);
        else
            registers->rasr |= 1 << 28;
    }

    void set_permitions(Permitions permitions){
        registers->rasr &= ~(0b111 << 24);
        registers->rasr |= (static_cast<uint8_t>(permitions) & 0b111) << 24;
    }

    /// @brief extremely simplified
    void disable_cache(){
        registers->rasr &= ~(0b11011 << 16);
    }

    void enable_cache(){
        registers->rasr |= 0b11011 << 16;
    }

    void is_shareable(bool is_shareable){
        registers->rasr &= ~(1 << 18);
        if (is_shareable)
            registers->rasr |= 1 << 18;
    }
    
    void set_subregions(uint8_t subregions){
        registers->rasr &= ~(0b11111 << 19);
        registers->rasr |= (subregions & 0b11111) << 19;
    }

    void enable_region() {
        registers->rasr |= 1;
    }
};