/**
 * Student name: Aldas Lenksas
 * Student number: 5714192
 *
 */

#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

#define R_BACKWARD 8
#define R_FORWARD 9
#define L_FORWARD 10
#define L_BACKWARD 11
#define RED_LED 1
#define YELLOW_LED 2
#define MASK(x) (1L << (x))
#define T_CCR_L_BACKWARD TIM1->CCR3
#define T_CCR_L_FORWARD TIM1->CCR4
#define T_CCR_R_BACKWARD TIM1->CCR2
#define T_CCR_R_FORWARD TIM1->CCR1

int motorCCR = 2500;
int prescaler = 16;

void setupTimer() {
  // Values
  TIM1->ARR = motorCCR;
  TIM1->PSC = prescaler - 1;
  TIM1->CCR1 = motorCCR;
  TIM1->CCR2 = motorCCR;
  TIM1->CCR3 = motorCCR;
  TIM1->CCR4 = motorCCR;

  // Capture/compare enable register: enable CCxE
  TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  // Auto-reload preload enable
  TIM1->CR1 |= TIM_CR1_ARPE;

  // Set channel modes to PW mode 1 ("110") and enable preload
  TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
  TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
  TIM1->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;
  TIM1->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;

  // Enable MOE, OSSI, OSSR, and UG
  TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSI | TIM_BDTR_OSSR;
  TIM1->EGR |= TIM_EGR_UG;

  // Enable timer
  TIM1->CR1 |= TIM_CR1_CEN;
}

void setupAll() {
  // Setup RCC (turn on power on GPIOA, GPIOB and TIM1)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

  // Setup LEDs pin mode (B1, B2)
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  GPIOB->MODER |= GPIO_MODER_MODE2_0;

  // Setup Motor pin modes (A8-A11) all to Alt.Functions (mode "10")
  GPIOA->MODER |= GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1;

  // Setup alternating functions in motor pins (to AF1 due to TIM1)
  // Using AFRH (8-11)
  // AF1 is "0001"
  GPIOA->AFR[1] |= (0x1U) | (0x1U << 4) | (0x1U << 8) | (0x1U << 12);

  setupTimer();
}

void warmMotors() {
  GPIOB->ODR ^= MASK(YELLOW_LED);

  ms_delay(500);
  TIM1->CCR1 = 100;
  TIM1->CCR3 = 100;
  ms_delay(2000);

  TIM1->CCR1 = motorCCR;
  TIM1->CCR3 = motorCCR;
  GPIOB->ODR &= ~MASK(YELLOW_LED);
}

void testMotorsDigital() {
  // Setup RCC (turn on power on GPIOA, GPIOB and TIM1)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

  // Setup LEDs pin mode (B1, B2)
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  GPIOB->MODER |= GPIO_MODER_MODE2_0;

  // Setup Motor pin modes (A8-A11) all to Output (mode "01")
  GPIOA->MODER |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0;

  GPIOA->ODR |= MASK(L_FORWARD);
  GPIOA->ODR |= MASK(R_BACKWARD);
}

int halfSpeed = 400;
int fullSpeed = 10;
int noSpeed = 1500;

void driveHalfSpeed() {
  GPIOB->ODR ^= MASK(YELLOW_LED);

  T_CCR_L_FORWARD = halfSpeed;
  T_CCR_R_FORWARD = halfSpeed;
  s_delay(3);

  GPIOB->ODR &= ~MASK(YELLOW_LED);
}

void driveFullSpeed() {
  GPIOB->ODR ^= MASK(RED_LED);

  T_CCR_L_FORWARD = fullSpeed;
  T_CCR_R_FORWARD = fullSpeed;
  s_delay(3);

  GPIOB->ODR &= ~MASK(RED_LED);
}

void slowDown() {
  GPIOB->ODR ^= MASK(YELLOW_LED);

  int curSpeed = fullSpeed;

  while (curSpeed < noSpeed) {
    curSpeed += 100;
    T_CCR_L_FORWARD = curSpeed;
    T_CCR_R_FORWARD = curSpeed;
    ms_delay(500);
  }

  T_CCR_L_FORWARD = motorCCR;
  T_CCR_R_FORWARD = motorCCR;

  GPIOB->ODR &= ~MASK(YELLOW_LED);
}

int main(void) {
  // The following lines only for testing
 // testMotorsDigital();
 // return 0;

  setupAll();
 // warmMotors();
  ms_delay(1000);

  driveHalfSpeed();
  driveFullSpeed();
  slowDown();

  // Main loop: switching LED
  while (true) {
    GPIOB->ODR ^= MASK(RED_LED);
    ms_delay(1000U);
  }

  return 0;
}
