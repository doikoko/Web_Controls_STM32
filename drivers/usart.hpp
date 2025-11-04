#pragma once

#include "data.hpp"
#include "gpio.hpp"

enum class DataBits{ Eight, Nine };
enum class WakeTrigger{ Idle, Address_Mask };
enum class Parity{ None, Even, Odd };
enum class StopBits{ Half, One, OneAndHalf, Two };
typedef struct {
    volatile uint32_t sr;
    volatile uint32_t dr;
    volatile uint32_t brr;
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t cr3;
    volatile uint32_t gtpr;
} USART_Reg;

class USART final{
public:
    GPIO tx, rx;
    USART_Reg* usart_registers;

    USART(uint8_t tx_num, char tx_letter, uint8_t rx_num, char rx_letter){
        usart_registers = reinterpret_cast<USART_Reg*>(USART1_BASE);
        tx = GPIO(tx_num, tx_letter);
        rx = GPIO(rx_num, rx_letter);
    }

    bool is_tx_empty() const {
        return (usart_registers->sr >> 7) & 1;
    }

    void sync(){
        clear_data_reg();
        while(!is_rx_empty());
        while (usart_registers->dr != 0xFF);
        usart_registers->dr = -1;
        while(!is_transmition_complete());
    }
    
    void sync_write_buf(void* buf, uint8_t len){
        sync();
        
        for(int i = 0; i < len; i++){
            while (!is_tx_empty());
            usart_registers->dr = reinterpret_cast<uint8_t*>(buf)[i];
            while (!is_transmition_complete());
        }
        usart_registers->dr = '\0';
        while(!is_transmition_complete());
    }

    void tx_enable(){
        usart_registers->cr1 |= 1 << 3;
    }
    
    void tx_disable(){
        usart_registers->cr1 &= ~(1 << 3);
    }

    bool is_rx_empty() const {
        return (usart_registers->sr >> 5) & 1;
    }

    void sync_read_buf(uint8_t* buf, uint16_t max){
        sync();
        
        for(uint16_t count = 0; buf[count] != '\0' && count < max; count++){
            while(!is_rx_empty());
            buf[count] = usart_registers->dr;
            while (!is_transmition_complete());
        }
    }
    
    void rx_enable(){
        usart_registers->cr1 |= 1 << 2;
    }
    
    void rx_disable(){
        usart_registers->cr1 &= ~(1 << 2);
    }

    void sleep(){
        usart_registers->cr1 |= 1 << 1;
    }

    void wake(){
        usart_registers->cr1 &= ~(1 << 1);
    }

    void rx_enable_dma(){
        usart_registers->cr3 |= 1 << 6;
    }

    void rx_disable_dma(){
        usart_registers->cr3 &= ~(1 << 6);
    }
    
    void set_break(){
        usart_registers->cr1 |= 1;
    }

    void tx_enable_dma(){
        usart_registers->cr3 |= 1 << 7;
    }

    void tx_disable_dma(){
        usart_registers->cr3 &= ~(1 << 7);
    }

    void clear_data_reg(){
        usart_registers->dr = 0;
    }

    void clock_enable(RCC& rcc){
        rcc.registers->apb2enr |= 1 << 4;
    }

    bool is_transmition_complete() const {
        return (usart_registers->sr >> 6) & 1;
    }
   
    bool is_len_break() const {
        return (usart_registers->sr >> 8) & 1;
    }
    
    bool is_cts_changed() const {
        return (usart_registers->sr >> 9) & 1;
    }

    void set_baud_rate(float bauds){
        usart_registers->cr1 &= ~(1 << 15);

        float div = 84000000.0 / bauds;
        uint16_t mantissa = div;
        uint32_t fraction = (div - mantissa) * 16 + 0.5;
        
        if (fraction >= 0xF) {
            mantissa += 1;
            fraction = 0;
        }
        
        usart_registers->brr = (mantissa << 4) | (fraction & 0xF);
    }

    void enable_usart(){
        usart_registers->cr1 |= 1 << 13;
    }
    
    void disable_usart(){
        usart_registers->cr1 &= ~(1 << 13);
    }

    void set_data_bits(DataBits data_bits){
        switch (data_bits) {
        case DataBits::Eight:
            usart_registers->cr1 &= ~(1 << 12);
            return;
        case DataBits::Nine:
            usart_registers->cr1 |= 1 << 12;
        }
    }

    void set_wake_trigger(WakeTrigger trigger){
        switch (trigger) {
        case WakeTrigger::Idle:
            usart_registers->cr1 |= 1 << 11;
            return;
        case WakeTrigger::Address_Mask:
            usart_registers->cr1 &= ~(1 << 11);
        }
    }

    void configure_parity(Parity parity){
        switch (parity) {
        case Parity::None:
            usart_registers->cr1 &= ~(1 << 10);   
            return;     
        case Parity::Even:
            usart_registers->cr1 |= 1 << 10;
            usart_registers->cr1 &= ~(1 << 9);
            return;
        case Parity::Odd:
            usart_registers->cr1 |= 1 << 10;
            usart_registers->cr1 |= 1 << 9;
        }
    }

    void interrupt_pe_enable(){
        usart_registers->cr1 |= 1 << 8;
    }

    void interrupt_pe_disable(){
        usart_registers->cr1 &= ~(1 << 8);
    }
    
    void interrupt_txe_enable(){
        usart_registers->cr1 |= 1 << 7;
    }

    void interrupt_txe_disable(){
        usart_registers->cr1 &= ~(1 << 7);
    }
    
    void interrupt_tc1_enable(){
        usart_registers->cr1 |= 1 << 6;
    }

    void interrupt_tc1_disable(){
        usart_registers->cr1 &= ~(1 << 6);
    }

    void interrupt_rxne_enable(){
        usart_registers->cr1 |= 1 << 5;
    }

    void interrupt_rxne_disable(){
        usart_registers->cr1 &= ~(1 << 5);
    }

    void interrupt_idle_enable(){
        usart_registers->cr1 |= 1 << 4;
    }

    void interrupt_idle_disable(){
        usart_registers->cr1 &= ~(1 << 4);
    }
    
    void lin_mode_enable(){
        usart_registers->cr2 |= 1 << 15;
    }

    void lin_mode_disable(){
        usart_registers->cr2 &= ~(1 << 15);
    }

    void set_stop_bits(StopBits stop_bits){
        usart_registers->cr2 &= ~(0b11 << 12);

        switch (stop_bits) {
        case StopBits::Half:
            usart_registers->cr2 |= 1 << 12;
            return;        
        case StopBits::One:
            return;
        case StopBits::Two:
            usart_registers->cr2 |= 0b10 << 12;
            return;
        case StopBits::OneAndHalf:
            usart_registers->cr2 |= 0b11 << 12;
        }
    }
    
    void smatcard_enable(){
        usart_registers->cr3 |= 1 << 5;
    }

    void smatcard_disable(){
        usart_registers->cr3 &= ~(1 << 5);
    }
};