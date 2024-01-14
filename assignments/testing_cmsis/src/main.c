#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

#define LEDPIN  1
#define L_BACKWARD_PIN 15
#define L_FORWARD_PIN 14
#define MASK(x) (1L << (x))

int main(void) {
  // GPIO enable pin output modes, alternate function push pull (GPIOx_CRH)

    // Setup
    // Enable clock in GPIOB and TIM1
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    GPIOB->MODER |= (0x01U << 2); // LED mode output
    GPIOB->MODER |= (0x10U << 30); // motor PIN set to alternate function
    GPIOB->MODER |= (0x10U << 28); // motor PIN set to alternate function

    // timer stuff
    TIM1->CCER |= TIM_CCER_CC2E;
    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
    TIM1->ARR = 65535;
    TIM1->PSC = 655;
    TIM1->CCR4 = 32768;

    TIM1->EGR |= TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;

     //| (0x01 << 30) | (0x01 << 28);
   // GPIOB->OTYPER |= GPIO_OTYPER_OT1;

    // GPIOB->ODR |= MASK(1);
    // ms_delay(1000U);
    // GPIOB->ODR ^= MASK(1);

   // GPIOB->ODR &= ~MASK(L_BACKWARD_PIN);
   // GPIOB->ODR |= MASK(L_FORWARD_PIN);

    // Main loop
    while (true) {
      int cnt = TIM1->CNT;
      if (cnt < 32768) 
        GPIOB->ODR |= MASK(LEDPIN);
      else
        GPIOB->ODR &= ~MASK(LEDPIN);
      //GPIOB->ODR ^= MASK(LEDPIN);
    //  ms_delay(300U);
    }

    return 0;
}
