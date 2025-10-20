#pragma once

#define RCC_BASE        0x40023800
#define GPIO_BASE       0x40020000
#define TIM_BASE        0x40000000
#define FLASH_BASE      0x40023C00
#define SYSTICK_BASE    0xE000E010
#define NVIC_BASE       0xE000E100
#define ICTR_BASE       0xE000E004
#define STIR_BASE       0xE000EF00
#define USART1_BASE     0x40011000

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define UINT32_T_MAX 0xFFFFFFFF

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

typedef struct {
    volatile uint32_t cr;
    volatile uint32_t pllcfgr;
    volatile uint32_t cfgr;
    volatile uint32_t cir;
    volatile uint32_t ahb1rstr;
    volatile uint32_t ahb2rstr;
    volatile uint32_t ahb3rstr;
    volatile uint32_t reserved0;
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

enum class GpioSpeed { Zero, One, Two, Three };
typedef struct {
    volatile uint32_t moder;
    volatile uint32_t otyper;
    volatile uint32_t ospeedr;
    volatile uint32_t pupdr;
    volatile uint32_t idr;
    volatile uint32_t odr;
    volatile uint32_t bsrr;
    volatile uint32_t lckr;
    volatile uint32_t afrl;
    volatile uint32_t afrh;
} GPIO_Reg;    

class GPIO{
protected:    
    uint8_t num;
    uint8_t letter;
public:    
    GPIO_Reg* registers;

    GPIO(uint8_t num, uint8_t letter) : 
        registers(reinterpret_cast<GPIO_Reg*>(
            GPIO_BASE + (0x400 * (letter - 'A'))
        )) {
            this->num = num;
            this->letter = letter;
    }    

    void clock_enable(RCC& rcc) const {
        if(letter >= 'A' && letter <= 'E')
            rcc.registers->ahb1enr |= 1 << (letter - 'A');
        else if (letter == 'H')    
            rcc.registers->ahb1enr |= 1 << 7;
    }        
    void set_input_mode(){
        registers->moder &= ~(0x3 << (2 * num));
    }    

    void set_output_mode(){
        registers->moder &= ~(0x3 << (2 * num));
        registers->moder |= 1 << (2 * num);
    }    

    void enable_push_pull(){
        registers->otyper &= ~(1 << num);
    }    

    void enable_open_drain(){
        registers->otyper |= (1 << num);
    }    

    uint32_t read_data() const {
        return registers->idr & (1 << num);
    }    

    void set_pull_up(){
        registers->pupdr &= ~(0x3 << (num * 2));
        registers->pupdr |= 1 << (num * 2);
    }    

    void set_pull_down(){
        registers->pupdr &= ~(0x3 << (num * 2));
        registers->pupdr |= 0x2 << (num * 2);
    }    

    void set_speed(GpioSpeed speed){
        registers->ospeedr &= ~(0x3 << (2 * num));
        registers->ospeedr |= static_cast<uint8_t>(speed) << (2 * num);
    }    
    
    void no_pull_up_down(){
        registers->pupdr &= ~(0x3 << (2 * num));
    }    

    void set_alt_function(uint8_t function_num){
        if (function_num > 15) return;
        if (num < 8) registers->afrl |= function_num << (num * 4);
        else if (num < 16) registers->afrh |= function_num << (num * 4);
    }
};    

class LED final : public GPIO{
public:    
    LED(uint8_t num, uint8_t letter) : GPIO(num, letter){}
    
    void enable_light(){
        registers->odr &= ~(1 << 13);
    }    

    void disable_light(){
        registers->odr |= 1 << 13;
    }    

    void blink(){
        registers->odr ^= 1 << 13;
    }    
};    

class Button final : public GPIO{
public:    
    Button(uint8_t num, uint8_t letter) : GPIO(num, letter){}

    /// @brief  on my device 1 - pressed
    bool is_pressed() const {
        return read_data();
    }
};    
typedef struct {
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t smcr;
    volatile uint32_t dier;
    volatile uint32_t sr;
    volatile uint32_t egr;
    volatile uint32_t ccmr1;
    volatile uint32_t ccmr2;
    volatile uint32_t ccer;
    volatile uint32_t cnt;
    volatile uint32_t psc;
    volatile uint32_t arr;
    volatile uint32_t ccr1;
    volatile uint32_t ccr2;
    volatile uint32_t ccr3;
    volatile uint32_t ccr4;
    volatile uint32_t reserved;
    volatile uint32_t dcr;
    volatile uint32_t dmar;
    volatile uint32_t tim2;
} TIM_Reg;    

class TIM final{
public:    
    TIM_Reg* registers;
    
    TIM(uint8_t num){
        if(num >= 2 && num <= 5)
        registers = reinterpret_cast<TIM_Reg*>(TIM_BASE + (0x400 * (num - 2)));
    }    
 
    void delay(uint32_t milliseconds){
        stop();
        init();
        start();
        while(registers->cnt < milliseconds);
        stop();
    }    
    void clock_enable(RCC& rcc){
        rcc.registers->apb1enr |= 1;
    }    
    
private:    
    void stop(){
        registers->cr1 &= ~1;
    }    
    void start(){
        registers->cr1 |= 1;
    }    
    void init(){
        registers->cr1 = 0;
        registers->cr2 = 0;
        registers->smcr = 0;
        registers->dier = 0;
        registers->psc = 84000 - 1;
        registers->arr = UINT32_T_MAX;
        registers->cnt = 0;
        registers->egr = 1;
    }    
};    

typedef struct {
    volatile uint32_t iser[16];
    volatile uint32_t icer[16];
    volatile uint32_t ispr[16];
    volatile uint32_t icpr[16];
    volatile uint32_t iabr[16];
    volatile uint32_t reserved1[47];
    volatile uint32_t ipr[123];
    volatile uint32_t reserved2[451];
} NVIC_Reg;

class NVIC final{
public:
    NVIC_Reg* registers;

    NVIC() : registers(reinterpret_cast<NVIC_Reg*>(NVIC_BASE)){}

    uint8_t get_interrupts_count(){
        return ((*reinterpret_cast<uint32_t*>(ICTR_BASE) & 0b1111) + 1) * 32;
    }

    void trigger_interrupt(uint8_t num){
        num %= get_interrupts_count();

        *reinterpret_cast<uint32_t*>(STIR_BASE) |= num;
    }

    uint8_t enable_interrupt(uint16_t num){
        if(num > (32 * 15)) return 1;
        registers->iser[num / 32] |= 1 << (num % 32);

        return 0;
    }
    
    uint8_t disable_interrupt(uint16_t num){
        if(num > (32 * 15)) return 1;
        registers->icer[num / 32] = 1 << (num % 32);

        return 0;
    }
    
    bool is_active(uint16_t interrupt){
        if(interrupt > (32 * 15)) return false;
        return registers->iabr[interrupt / 32] & (1 << (interrupt % 32));
    }

    uint8_t set_priority(uint16_t num, uint8_t priority){
        if(num > (32 * 15) && priority > 15) return 1;
        registers->ipr[num / 4] |= (priority << (8 * (num % 4) + 4));

        return 0;
    }
};

typedef struct {
    volatile uint32_t csr;
    volatile uint32_t rvr;
    volatile uint32_t cvr;
    volatile uint32_t calib;
} Systick_Reg;

class Systick final{
public:
    Systick_Reg* registers;

    Systick() : registers(reinterpret_cast<Systick_Reg*>(SYSTICK_BASE)){};

    bool is_end() const{
        return (registers->csr & (1 << 16));
    }

    void set_is_proc_clock(bool is_proc_clock){
        if(is_proc_clock) registers->csr |= (1 << 2);
        else registers->csr &= ~(1 << 2);
    }

    void set_is_interrupt(bool is_interrupt){
        if(is_interrupt) registers->csr |= (1 << 1);
        else registers->csr &= ~(1 << 1);
    }

    void start(){
        registers->csr |= 1;
    }

    void stop(){
        registers->csr &= ~1;
    }

    void set_ticks(uint32_t ticks){
        ticks &= 0x00FFFFFF;
        registers->rvr = ticks - 1;
    }

    uint32_t get_current_value(){
        return registers->cvr;
    }

    void delay_ms_interrupt(uint32_t milliseconds){
        set_is_proc_clock(true);
        set_is_interrupt(true);
        set_ticks(84000 * milliseconds - 1);

        start();
    }

    /// @brief extreme simple delay function (for my own use)
    void delay(uint32_t milliseconds){
        set_is_proc_clock(true);
        set_is_interrupt(false);
        set_ticks(84000 * 10 - 1);
        registers->cvr = 0;
        start();

        for(uint32_t count = 0; count < milliseconds / 10; count++)
            while(!is_end());
            
        stop();
    }
};

enum class DataBits{ Eight, Nine };
enum class WakeTrigger{ Idle, Address_Mask };
enum class Parity{ Even, Odd };
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
    USART_Reg* usart_registers;

    USART() : usart_registers(reinterpret_cast<USART_Reg*>(USART1_BASE)){}

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
        
        if (fraction >= 16) {
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

    void parity_control_enable(){
        usart_registers->cr1 |= 1 << 10;
    }
    
    void parity_control_disable(){
        usart_registers->cr1 &= ~(1 << 10);
    }

    void configure_parity(Parity parity){
        switch (parity) {
        case Parity::Even:
            usart_registers->cr1 &= ~(1 << 9);
            return;
        case Parity::Odd:
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

class TX final : public GPIO{
    uint8_t num;
    uint8_t letter;
public:
    USART_Reg* usart_registers;

    TX(USART_Reg* usart_registers, uint8_t num, uint8_t letter) : GPIO(num, letter){
        this->num = num;
        this->letter = letter;
        this->usart_registers = usart_registers;
    }

    bool is_empty() const {
        return !((usart_registers->sr >> 7) & 1);
    }

    void write(uint8_t data){
        usart_registers->dr = data;
    }

    void enable(){
        usart_registers->cr1 |= 1 << 3;
    }
    
    void disable(){
        usart_registers->cr1 &= ~(1 << 3);
    }

    void set_break(){
        usart_registers->cr1 |= 1;
    }

    void enable_dma(){
        usart_registers->cr3 |= 1 << 7;
    }

    void disable_dma(){
        usart_registers->cr3 &= ~(1 << 7);
    }
};

class RX final : public GPIO{
    uint8_t num;
    uint8_t letter;
public:
    USART_Reg* usart_registers;
    
    RX(USART_Reg* usart_registers, uint8_t num, uint8_t letter) : GPIO(num, letter){
        this->num = num;
        this->letter = letter;
        this->usart_registers = usart_registers;
    }
    
    bool is_empty() const {
        return !((usart_registers->sr >> 5) & 1);
    }

    uint8_t read() const {
        return usart_registers->dr;
    }

    void enable(){
        usart_registers->cr1 |= 1 << 2;
    }
    
    void disable(){
        usart_registers->cr1 &= ~(1 << 2);
    }

    void sleep(){
        usart_registers->cr1 |= 1 << 1;
    }

    void wake(){
        usart_registers->cr1 &= ~(1 << 1);
    }

    void enable_dma(){
        usart_registers->cr3 |= 1 << 6;
    }

    void disable_dma(){
        usart_registers->cr3 &= ~(1 << 6);
    }
};
