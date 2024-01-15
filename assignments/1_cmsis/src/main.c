#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

#define R_BACKWARD 8
#define R_FORWARD 9
#define L_FORWARD 10
#define L_BACKWARD 11
#define RED_LED 1
#define MASK(x) (1L << (x))

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

  // Setup LED pin mode (B1)
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  
  // Setup Motor pin modes (A8-A11) all to Alt.Functions (mode "10")
  GPIOA->MODER |= GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1; 

  // Setup alternating functions in motor pins (to AF1 due to TIM1)
  // Using AFRH (8-11)
  // AF1 is "0001"
  GPIOA->AFR[1] |= (0x1U) | (0x1U << 4) | (0x1U << 8) | (0x1U << 12);

  setupTimer();
}

int convertToCcrValue(float speed) {
  if (speed < 0.0f) {
    speed = -speed;
  }

  float converted = speed * (float)motorCCR;
  return motorCCR - (int)converted;
}

void setMotorSpeed(float leftMotor, float rightMotor) {
  TIM1->CCR1 = 2500;
  TIM1->CCR2 = 2500;

  if (leftMotor >= 0.0f) {
    TIM1->CCR4 = 2500; // backward no speed
    TIM1->CCR3 = convertToCcrValue(leftMotor);
  }
}

int main(void) {
  setupAll();
  ms_delay(1000);

  int curSpeed = 0.3f;
  while (curSpeed < 0.9f) {
    setMotorSpeed(curSpeed, 0);
    ms_delay(1000);
    curSpeed += 0.1f;
  }

  // Main loop: switching LED
  while (true) {
    GPIOB->ODR ^= MASK(RED_LED);
    ms_delay(300U);
  }

  return 0;
}
