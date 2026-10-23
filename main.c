#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/timer.h>
#include <hardware/spi.h>

#include "spi.h"
#include "cli.h"

#define GPIO_LED        25

#define GPIO_CLK        6

void clock_generator_init(){
    gpio_set_function(GPIO_CLK, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(GPIO_CLK);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac4(&config, 1, 4);
    pwm_config_set_wrap(&config, 3);

    pwm_init(slice, &config, true);
    pwm_set_gpio_level(GPIO_CLK, 2);
}

bool blink_callback(__unused repeating_timer_t *timer){
    gpio_put(GPIO_LED, !gpio_get(GPIO_LED));
    return true;
}



int main(){
    stdio_init_all();
    gpio_init(GPIO_LED);
    gpio_set_dir(GPIO_LED, GPIO_OUT);
    clock_generator_init();
    SPI_init();

    static repeating_timer_t timer;
    add_repeating_timer_ms(250, &blink_callback, NULL, &timer);

    uint8_t data = 0xFF;
    while (1){
        // spi_write_blocking(spi0, &data, 1);
        if (CLI_read()){
            CLI_execute();
        }
    }
    return 0;
}
