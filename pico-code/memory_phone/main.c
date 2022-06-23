#include <stdio.h>
#include "pico/stdlib.h"

bool hang_up_flg = false;
const uint led_pin = 25;
const uint hang_up_pin = 15;

void gpio_callback(uint gpio, uint32_t events) {
    if(gpio==15) hang_up_flg = 1;    
}

int main() {

    

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_init(hang_up_pin);
    gpio_set_dir(hang_up_pin, GPIO_IN); 
    gpio_set_irq_enabled_with_callback(hang_up_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Initialize chosen serial port
    stdio_init_all();

    // Loop forever
    while (true) {

        // check flag
        if(hang_up_flg){
            if(gpio_get(hang_up_pin)){
                gpio_put(led_pin, true);
                printf("Pin is High\r\n");
            }
            else{
                gpio_put(led_pin, false);
                printf("Pin is Low\r\n");
            }
            hang_up_flg = 0;
        }
        
    }
}


