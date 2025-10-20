#include "../drivers/driver.hpp"


[[noreturn]]
int main(){
    RCC rcc;
    TIM tim2 = { 2 };
// Led - part of my development board
    LED led = { 13, 'C' };
// Button - part of my development board
    Button button = { 0, 'A' };
    Systick systick;
    USART usart;
    TX tx = { usart.usart_registers, 9, 'A' };
    RX rx = { usart.usart_registers, 10, 'A' };

// init led
    led.clock_enable(rcc);
    led.set_output_mode();
    led.enable_push_pull();
    led.set_speed(GpioSpeed::Three);
    led.no_pull_up_down();

    button.clock_enable(rcc);
    button.set_input_mode();
    button.set_speed(GpioSpeed::Three);
    button.set_pull_up();
    
    tim2.clock_enable(rcc);
    rcc.config_pll(7, 4, 336, 16);
    led.disable_light();
    
    usart.clock_enable(rcc);
    usart.disable_usart();
    usart.usart_registers->cr1 = 0;
    usart.set_data_bits(DataBits::Eight);
    usart.set_stop_bits(StopBits::One);
    usart.configure_parity(Parity::Even);
    usart.set_baud_rate(9600 * 2);
    
    tx.clock_enable(rcc);
    tx.set_alt_function(7);
    
    rx.clock_enable(rcc);
    rx.set_alt_function(7);
    
    usart.enable_usart();
    
    rx.enable_open_drain();
    tx.enable_open_drain();
    
    rx.enable_push_pull();
    tx.enable_push_pull();

    tx.enable();
    rx.enable();
    
    while(rx.is_empty());
    led.enable_light();

    while(true){}
}
