#include "../drivers/driver.hpp"

enum Commands{
    SendData, RecieveCode
};
[[noreturn]]
int main(){
    RCC rcc;
    TIM tim2 = { 2 };
// Led - part of my development board
    LED led = { 13, 'C' };
// Button - part of my development board
    Button button = { 0, 'A' };
    Systick systick;
    USART usart = { 9, 'A', 10, 'A' };

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
    
    usart.clear_data_reg();
    usart.clock_enable(rcc);
    
    usart.tx.set_alt_function_mode();
    usart.rx.set_alt_function_mode();

    usart.tx.clock_enable(rcc);
    usart.rx.clock_enable(rcc);

    usart.tx.set_alt_function(7);
    usart.rx.set_alt_function(7);

    usart.tx.enable_push_pull();
    usart.rx.enable_push_pull();
    usart.tx.no_pull_up_down();
    usart.rx.no_pull_up_down();
    usart.tx.set_speed(GpioSpeed::Three);
    usart.rx.set_speed(GpioSpeed::Three);

    usart.disable_usart();
    usart.set_data_bits(DataBits::Eight);
    usart.set_stop_bits(StopBits::One);
    usart.configure_parity(Parity::None);
    usart.set_baud_rate(9600);

    usart.tx_enable(); 
    usart.rx_enable(); 
    usart.enable_usart(); 
    
    led.disable_light();
    while(true){
        
    }
}
