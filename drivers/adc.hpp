#pragma once

#include "data.hpp"

enum class Resolution: uint8_t{
    Six     = 0b11,
    Eight   = 0b10,
    Ten     = 0b01,
    Twelve  = 0b00
};
enum class ExternalTrigger: uint8_t{
    Disable     = 0b00,
    Rising      = 0b01,
    Falling     = 0b10,
    RisingFalling = 0b11
};
/// @brief T - Timer
enum class ExternalEvent: uint8_t{
    T1CC1   = 0b0,
    T1CC2   = 0b1,
    T1CC3   = 0b10,
    T2CC2   = 0b11,
    T2CC3   = 0b100,
    T2CC4   = 0b101,
    T2TRGO  = 0b110,
    T3CC1   = 0b111,
    T3TRGO  = 0b1000,
    T4CC4   = 0b1001,
    T5CC1   = 0b1010,
    T5CC2   = 0b1011,
    T5CC3   = 0b1100,
    EXTILine11 = 0b1111
};
enum class DataAlign: uint8_t{
    Left    = 0b1,
    Right   = 0b0,
};
enum class SampleTime: uint8_t{
    Three       = 0b0,
    Fifteen     = 0b1,
    TwentyEight = 0b10,
    FiftySix    = 0b11,
    EightyFour  = 0b100,
    OneHundredTwelve     = 0b101,
    OneHundredFourtyFour = 0b110,
    FourHundredEighty    = 0b111
};
enum class Prescaler: uint8_t{
    Two     = 0b00,
    Four    = 0b01,
    Six     = 0b10,
    Eight   = 0b11
};
typedef struct {
    volatile uint32_t sr;
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t smpr1;
    volatile uint32_t smpr2;
    volatile uint32_t jofr1;
    volatile uint32_t jofr2;
    volatile uint32_t jofr3;
    volatile uint32_t jofr4;
    volatile uint32_t htr;
    volatile uint32_t ltr;
    volatile uint32_t sqr1;
    volatile uint32_t sqr2;
    volatile uint32_t sqr3;
    volatile uint32_t jsqr;
    volatile uint32_t jdr;
    volatile uint32_t dr;
    volatile uint32_t* ccr;
} ADC_Reg;

class ADC {
public:
    ADC_Reg* registers;

    ADC() : registers(reinterpret_cast<ADC_Reg*>(ADC_BASE)){ 
        registers->ccr = reinterpret_cast<uint32_t*>(ADC_CCR_BASE);
    }

    bool is_overrun(){
        return registers->sr & 1 << 5;
    }

    bool is_regular_conversion_start(){
        return registers->sr & 1 << 4;
    }

    bool is_injected_conversion_start(){
        return registers->sr & 1 << 3;
    }

    bool is_injected_conversion_end(){
        return registers->sr & 1 << 2;
    }

    bool is_regular_conversion_end(){
        return registers->sr & 1 << 1;
    }

    bool is_value_bigger_than_programmed(){
        return registers->sr & 1;
    }

    void enable_overrun_interrupt(){
        registers->cr1 |= 1 << 26;
    }
  
    void disable_overrun_interrupt(){
        registers->cr1 &= ~(1 << 26);
    }

    void set_resolution(Resolution resolution){
        registers->cr1 &= ~(0b11 << 24);
        registers->cr1 |= static_cast<uint8_t>(resolution);
    }

    void watchdog_regular_enable(){
        registers->cr1 |= 1 << 23;
    }

    void watchdog_regular_disable(){
        registers->cr1 &= ~(1 << 23);
    }


    void watchdog_injected_enable(){
        registers->cr1 |= 1 << 22;
    }

    void watchdog_injected_disable(){
        registers->cr1 &= ~(1 << 22);
    }

    void set_discounting_mode_channel_count(uint8_t count){
        registers->cr1 &= (0b111 << 13);
        registers->cr1 |= (count >= 8 ? 0b111 : count) << 13;
    }

    void enable_discounting_mode_injected(){
        registers->cr1 |= 1 << 12;
    }

    void disable_discounting_mode_injected(){
        registers->cr1 &= ~(1 << 12);
    }
    
    void enable_discounting_mode_regular(){
        registers->cr1 |= 1 << 11;
    }

    void disable_discounting_mode_regular(){
        registers->cr1 &= ~(1 << 11);
    }

    void enable_auto_injected_conv_after_regular(){
        registers->cr1 |= 1 << 10;
    }

    void disble_auto_injected_conv_after_regular(){
        registers->cr1 &= ~(1 << 10);
    }

    void watchdog_single_mode_enable(){
        registers->cr1 |= 1 << 9;
    }

    void watchdog_single_mode_disable(){
        registers->cr1 &= ~(1 << 9);
    }

    void enable_scan_mode(){
        registers->cr1 |= 1 << 8;
    }

    void disable_scan_mode(){
        registers->cr1 &= ~(1 << 8);
    }

    void enable_interrupts_injected(){
        registers->cr1 |= 1 << 7;
    }

    void disable_interrupts_injected(){
        registers->cr1 &= ~(1 << 7);
    }

    void enable_interrupts_watchdog(){
        registers->cr1 |= 1 << 6;
    }

    void disable_interrupts_watchdog(){
        registers->cr1 &= ~(1 << 6);
    }

    void enable_interrupts_eoc(){
        registers->cr1 |= 1 << 5;
    }

    void disable_interrupts_eoc(){
        registers->cr1 &= ~(1 << 5);
    }
    
    void set_analog_watchdog_channels(uint8_t channels){
        registers->cr1 &= ~0xF;
        registers->cr1 |= channels & 0xF;
    }

    void start_reg_conv(){
        registers->cr2 |= 1 << 30;
    }

    void set_external_trigger_regular(ExternalTrigger trigger){
        registers->cr2 &= ~(0b11 << 28);
        registers->cr2 |= static_cast<uint8_t>(trigger) << 28;
    }

    void set_external_event_reguilar(ExternalEvent event){
        registers->cr2 &= ~(0b1111 << 16);
        registers->cr2 |= static_cast<uint8_t>(event) << 16;
    }

    void set_data_align(DataAlign align){
        registers->cr2 &= ~(1 << 11);
        registers->cr2 |= static_cast<uint8_t>(align) << 11;
    }

    void enable_dma_section(){
        registers->cr2 |= 1 << 9;
    }

    void disable_dma_section(){
        registers->cr2 &= 1 << 9;
    }

    void enable_dma(){
        registers->cr2 |= 1 << 8;
    }

    void disable_dma(){
        registers->cr2 &= 1 << 8;
    }

    void enable_continous_conversion(){
        registers->cr2 |= 1 << 1;
    }

    void disable_continous_conversion(){
        registers->cr2 &= ~(1 << 1);
    }

    void enable(){
        registers->cr2 |= 1;
    }

    void disable(){
        registers->cr2 &= ~1;
    }
    
    void set_time_channels10_18(SampleTime time, uint8_t channel){
        constexpr uint8_t min_channel = 10, max_channel = 18;
        if(channel < min_channel && channel > max_channel) 
            return;

        registers->smpr1 &= ~(0b111 << (channel - min_channel));
        registers->smpr1 |= static_cast<uint8_t>(time); 
    }

    void set_time_channels0_9(SampleTime time, uint8_t channel){
        constexpr uint8_t min_channel = 0, max_channel = 9;
        
        if(channel < min_channel && channel > max_channel) 
            return;

        registers->smpr2 &= ~(0b111 << (channel - min_channel));
        registers->smpr2 |= static_cast<uint8_t>(time); 
    }

    void set_data_offset_injected1(uint8_t channel, uint16_t offset){
        constexpr uint8_t min_channel = 1, max_channel = 4;
        constexpr uint16_t max_offset = 0xFFF;
        
        if(channel < min_channel || channel > max_channel || offset > max_offset)
            return;

        
        volatile uint32_t *jofr = &registers->jofr1;
        volatile uint32_t *jofrx = jofr + (channel - 1);

        *jofrx &= max_offset;
        *jofrx |= offset & max_offset;
    }

    void set_watchdow_higher_threshold(uint8_t threshold){
        constexpr uint16_t higher_threshold = 0xFFF;

        registers->htr &= higher_threshold;
        registers->htr |= threshold & higher_threshold;
    }


    void set_watchdow_lower_threshold(uint8_t threshold){
        constexpr uint16_t lower_threshold = 0xFFF;

        registers->htr &= lower_threshold;
        registers->htr |= threshold & lower_threshold;
    }

    void set_total_conversions_reg(uint8_t conversions){
        constexpr uint8_t max_conversions = 0xF;
        
        registers->sqr1 &= ~(max_conversions << 20);
        registers->sqr1 |= conversions & max_conversions << 20;
    }

    void set_conversions_reg(uint8_t channel, uint8_t conversions){
        constexpr uint8_t max_channel = 16, max_conversions = 0x1F;
        constexpr uint8_t sqr1_min = 13, sqr2_min = 7, sqr3_min = 1, sq_size = 5;

        if(channel > max_channel)
            return;

        if(channel > sqr1_min)
            registers->sqr1 &= ~(max_conversions << (channel - sqr1_min) * sq_size);
        else if(channel > sqr2_min)
            registers->sqr2 &= ~(max_conversions << (channel - sqr2_min) * sq_size);
        else if(channel > sqr1_min)
            registers->sqr3 &= ~(max_conversions << (channel - sqr3_min) * sq_size);
    }
    
    void set_total_conversions_inj(uint8_t conversions){
        constexpr uint8_t max_conversions = 0b11;
        
        registers->jsqr &= ~(max_conversions << 20);
        registers->jsqr |= conversions & max_conversions << 20;
    }

    uint16_t get_inj_data(){
        return registers->jdr;
    }
  
    uint16_t get_reg_data(){
        return registers->dr;
    }  

    void enable_temp_sensor(){
        *registers->ccr |= 1 << 23;
    }
    
    void disable_temp_sensor(){
        *registers->ccr &= ~(1 << 23);
    }

    void enable_vbat_channel(){
        *registers->ccr |= 1 << 22;
    }
    
    void disable_vbat_channel(){
        *registers->ccr &= ~(1 << 22);
    }

    void set_prescaler(Prescaler prescaler){
        *registers->ccr |= static_cast<uint8_t>(prescaler) << 16;
    }
};