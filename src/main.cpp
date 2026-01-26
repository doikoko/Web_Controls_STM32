#include "../drivers/rcc.hpp"
#include "../drivers/tim.hpp"
#include "../drivers/dev_board_periphy.hpp"
#include "../drivers/systick.hpp"
#include "../drivers/usart.hpp"
#include "../drivers/user.hpp"
#include "../drivers/async.hpp"

uint8_t code[KILOBYTE * 8] __attribute__((section(".user_code")));

[[noreturn]]
int main(){
    RCC rcc;
    TIM tim2 = { 2 };
// Led - part of my development board
    LED led = { 13, 'C' };
// Button - part of my development board
    Systick systick;
    Button button = { 0, 'A' };
    USART usart = {9, 'A', 10, 'A' };
    User user(code);
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
    
    rcc.config_pll(7, 4, 336, 16);
    tim2.clock_enable(rcc);
    
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
        user.recv_code(usart);
        user.call();
        user.send_returned_value(usart);
    }
}
