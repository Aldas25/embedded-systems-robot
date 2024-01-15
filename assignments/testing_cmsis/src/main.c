#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

#define LEDPIN  1
#define L_BACKWARD_PIN 15
#define L_FORWARD_PIN 14
#define MASK(x) (1L << (x))

/*
int main() {
  // R motor: B3 : Tim2 Ch2

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  GPIOB->MODER |= GPIO_MODER_MODE1_0; // output mode B1 (LED)
  GPIOB->MODER |= GPIO_MODER_MODE3_1; // AF mode B3
  GPIOA->MODER |= GPIO_MODER_MODE8_1; // AF mode A8

  GPIOB->AFR[0] |= GPIO_AFRL_AFRL1_0; // AF1 for B3

  // TIM2_ARR
  // TIM2_CCR2
  // TIM2_CCMR2: OC2M to 110 (pwm mode2
  // TIM2_CCMR2: OC2PE
  // TIM2_CR1: ARPE
  // TIM2_CRG: UG

  TIM2->ARR = 60000;
  TIM2->CCR2 = 50000;

  TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
  TIM2->CCMR1 |= TIM_CCMR1_OC2PE;
  TIM2->CR1 |= TIM_CR1_ARPE;
  TIM2->CRG |= TIM_CRG_URG

  GPIOB->ODR |= MASK(1); // LED 

  while (true) {}
  return 0;
}
*/


int main(void) {

  // GPIO enable pin output modes, alternate function push pull (GPIOx_CRH)
    ms_delay(500);
    // Setup
    // Enable clock in GPIOB and TIM1
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    GPIOB->MODER |= (0x1U << 2); // LED mode output
    GPIOB->MODER |= GPIO_MODER_MODER15_1;//  (0x2U << 30); // motor PIN set to alternate function
    GPIOB->MODER |= GPIO_MODER_MODER14_1;//(0x2U << 28); // motor PIN set to alternate function

    GPIOB->AFR[1] |= (0x1U << 28) | (0x1U << 24);

    ms_delay(500);
    int curSpeed = 1000;

    // timer stuff
    
    //TIM1->CR1 &= ~TIM_CR1_CEN;
    // TIM1->CCER |= TIM_CCER_CC2E;
    TIM1->CCER |= TIM_CCER_CC2NE;
    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
    TIM1->ARR = 2500;
    TIM1->PSC = 16 - 1;
    //TIM1->CCR4 = 32768;
    TIM1->CCR2 = 2400;

    TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSI | TIM_BDTR_OSSR;
    TIM1->EGR |= TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;

    int speed = 2400;
    while(true) {
        ms_delay(1000U);
        speed -= 100;
        if (speed < 0) speed = 1;
        TIM1->CCR2 = speed;
    }

    // while (curSpeed < 2500-1) {
    //   ms_delay(1000);
    //   curSpeed += 500;
    //   if (curSpeed >= 2500) curSpeed = 2500-1;
    //   TIM1->CCR2 = curSpeed;
    // }

    

    GPIOB->ODR |= MASK(LEDPIN);

    while(true) {}

    return 0;
}
