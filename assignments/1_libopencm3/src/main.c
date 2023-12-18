#include "utils.hpp"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdbool.h>


int main(void) {
    // Setup
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1 | GPIO3 | GPIO14 | GPIO15);

    //gpio_set(GPIOA, GPIO8);
    //gpio_set(GPIOB, GPIO3);
    gpio_set(GPIOB, GPIO14);
    gpio_clear(GPIOB, GPIO15);

    gpio_set(GPIOA, GPIO8);
    gpio_clear(GPIOB, GPIO3);

    // Main loop
    while (true) {
      gpio_toggle(GPIOB, GPIO1); // NOT A motor 
     // gpio_toggle(GPIOA, GPIO8);
      ms_delay(1000U);
    }

    return 0;
}
