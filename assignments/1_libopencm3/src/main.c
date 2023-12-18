#include "utils.hpp"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdbool.h>


int main(void) {
    // Setup
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);

    // Main loop
    while (true) {
      gpio_toggle(GPIOC, GPIO13);
      gpio_toggle(GPIOB, GPIO1);
      ms_delay(300U);
    }

    return 0;
}
