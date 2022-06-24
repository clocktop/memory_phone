#include <stdio.h>
#include "pico/stdlib.h"
#include "analog_microphone.h"
#include "tusb.h"
#include "hardware/adc.h"


#define IDLE 0
#define INIT 1
#define LOGGING 2
#define CLOSING 3

bool hang_up_flg = false;
uint logging_app_state = IDLE;
const uint led_pin = 25;
const uint hang_up_pin = 15;


void gpio_callback(uint gpio, uint32_t events) 
{
    if(gpio==15) hang_up_flg = 1;    
}

// configuration
const struct analog_microphone_config config = 
{
    // GPIO to use for input, must be ADC compatible (GPIO 26 - 28)
    .gpio = 26,

    // bias voltage of microphone in volts
    .bias_voltage = 1.25,

    // sample rate in Hz
    .sample_rate = 8000,

    // number of samples to buffer
    .sample_buffer_size = 256,
};

// variables
int16_t sample_buffer[256];
volatile int samples_read = 0;

void on_analog_samples_ready()
{
    // callback from library when all the samples in the library
    // internal sample buffer are ready for reading 
    samples_read = analog_microphone_read(sample_buffer, 256);
}

void loginit(void)
{
    // start capturing data from the analog microphone
    if (analog_microphone_start() < 0) {
        printf("PDM microphone start failed!\n");
        while (1) { tight_loop_contents();  }
    }
}

void logdeinit(void)
{
    //analog_microphone_stop();
    adc_run(false);
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
    while (!tud_cdc_connected()) {
        tight_loop_contents();
    }

    // initialize the analog microphone
    if (analog_microphone_init(&config) < 0) {
        printf("analog microphone initialization failed!\n");
        while (1) { tight_loop_contents(); }
    }

    // set callback that is called when all the samples in the library
    // internal sample buffer are ready for reading
    analog_microphone_set_samples_ready_handler(on_analog_samples_ready);

    // Loop forever
    while (true) 
    {
        // check flag
        if(hang_up_flg){
            sleep_ms(100);
            if(gpio_get(hang_up_pin)){
                gpio_put(led_pin, true);
                logging_app_state = INIT;
                printf("New Log Init\r\n");
            }
            else{
                gpio_put(led_pin, false);
                logging_app_state = CLOSING;
                printf("Log Close\r\n");
            }
            hang_up_flg = 0;
        }

        switch (logging_app_state)
        {
            case IDLE:
                break;

            case INIT:
                loginit();
                logging_app_state = LOGGING;
                break;

            case LOGGING:
                if(samples_read != 0)
                {
                    int sample_count = samples_read;
                    samples_read = 0;
                    for (int i = 0; i < sample_count; i++) 
                    {
                        printf("%d\n", sample_buffer[i]);
                    }
                }
                break;

            case CLOSING:
                logdeinit();
                logging_app_state = IDLE;
                printf("Waiting...\r\n");
                break;
        }     


        
    }
}


