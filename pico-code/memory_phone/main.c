#include <stdio.h>
#include "pico/stdlib.h"
#include "analog_microphone.h"
#include "tusb.h"
#include "hardware/adc.h"
#include "test.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"


#define IDLE 0
#define INIT 1
#define GREETING_START 2
#define GREETING_PLAY 3
#define LOGGING 4
#define CLOSING 5

bool hang_up_flg = false;
uint logging_app_state = IDLE;

const uint led_pin = 25;
const uint hang_up_pin = 15;
const uint audio_pin = 22;

int wav_position = 0;


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
    printf("Start Log \r\n");
}

void logdeinit(void)
{
    //analog_microphone_stop();
    adc_run(false);
}

void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(audio_pin));    
    if (wav_position < (WAV_DATA_LENGTH<<3) - 1) { 
        // set pwm level 
        // allow the pwm value to repeat for 8 cycles this is >>3 
        pwm_set_gpio_level(audio_pin, WAV_DATA[wav_position>>3]);  
        wav_position++;
    } else {
        // reset to start
        irq_set_enabled(PWM_IRQ_WRAP, false);
        pwm_set_gpio_level(audio_pin, 0);
        logging_app_state = INIT;
        printf("End Greeting\r\n");
    }
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
    
    set_sys_clock_khz(176000, true); 
    // initialize the analog microphone
    if (analog_microphone_init(&config) < 0) {
        printf("analog microphone initialization failed!\n");
        while (1) { tight_loop_contents(); }
    }

    // set callback that is called when all the samples in the library
    // internal sample buffer are ready for reading
    analog_microphone_set_samples_ready_handler(on_analog_samples_ready);

    gpio_set_function(audio_pin, GPIO_FUNC_PWM);
    int audio_pin_slice = pwm_gpio_to_slice_num(audio_pin);
    pwm_config config = pwm_get_default_config();
    /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
     * to set the interrupt rate. 
     * 
     * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
     * 
     * 
     * So clkdiv should be as follows for given sample rate
     *  8.0f for 11 KHz
     *  4.0f for 22 KHz
     *  2.0f for 44 KHz etc
     */
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, false);

    // Loop forever
    while (true) 
    {
        // check flag
        if(hang_up_flg){
            sleep_ms(100);
            if(gpio_get(hang_up_pin)){
                gpio_put(led_pin, true);
                logging_app_state = GREETING_START;
                printf("Start Greeting\r\n");
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
                pwm_set_enabled(audio_pin_slice, false);
                loginit();
                logging_app_state = LOGGING;
                break;

            case GREETING_START:
                // Setup PWM interrupt to fire when PWM cycle is complete
                wav_position = 0;
                pwm_clear_irq(audio_pin_slice);
                pwm_set_irq_enabled(audio_pin_slice, true);
                // set the handle function above
                irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
                irq_set_enabled(PWM_IRQ_WRAP, true);
                pwm_set_enabled(audio_pin_slice, true);
                logging_app_state = GREETING_PLAY;
                printf("Playing Greeting \r\n");
                break;

            case GREETING_PLAY:
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


