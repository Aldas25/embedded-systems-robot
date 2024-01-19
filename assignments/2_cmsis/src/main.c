#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

// LEDs B1, B2
#define YELLOW_LED 2
#define RED_LED 1

// US: Front trig A15, echo B9
#define FRONT_TRIG 15
#define FRONT_ECHO 9
#define MASK(x) (1UL << (x))
#define FRONT_ECHO_VALUE (GPIOB->IDR & MASK(FRONT_ECHO))

void setupFrontUltrasoundInterrupts() {
  // Set external interrupts for the Echo pin (B9)
  EXTI->IMR |= EXTI_IMR_MR9;
  // rising and falling edge enabled
  EXTI->RTSR |= EXTI_RTSR_TR9;
  EXTI->FTSR |= EXTI_FTSR_TR9;

  // we are not expecting any requests right now (just clear this to be sure)
 // EXTI->PR |= EXTI_PR_PR9;

  // Set the external interrupt value in SYSCFG
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI9_PB;

  // Setup interrupt functions
  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void setup() {
  // Turn on power by turning RCC in GPIOB, GPIOA
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // Both LED mode output ("01")
  GPIOB->MODER |= GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0;

  // Trig mode output ("01")
  GPIOA->MODER &= ~GPIO_MODER_MODE15;
  GPIOA->MODER |= GPIO_MODER_MODE15_0;
  
  GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0; // "01" pull-up

  // Echo mode input ("00")
  GPIOB->MODER  &= ~GPIO_MODER_MODE9;


 setupFrontUltrasoundInterrupts();

  // Setup timers
  //TIM2->PSC = 16 - 1; // max value 
  // 32 seconds = 500 * 1000 * 1000
  // then, 1 us = 15.625 TIM2 ticks
  // then, 235 ticks should be enough for 10 us
  TIM2->ARR = 10000;
}

void ten_microseconds_delay() {
  TIM2->CNT = 0;
  TIM2->CR1 |= TIM_CR1_CEN;

  // a bit more than 10 us
  while ((TIM2->CNT) < 200) {}

  TIM2->CR1 &= ~TIM_CR1_CEN;
}

int timerTicks;

float readFrontSensor() {
  float distance_to_return = 0;
  timerTicks = 0;

  // Send 10 us Trig pulse
  GPIOA->ODR |= MASK(FRONT_TRIG);
  ms_delay(1);
  //ten_microseconds_delay(); 
  // ms_delay(1000);
  GPIOA->ODR &= ~MASK(FRONT_TRIG);

//  while ((GPIOB->IDR) & (MASK(9)) == 0)

  ms_delay(10); // just to be sure that the interrupts happened

  if (timerTicks == 0) return 0; // no interrupt happened
  float duration_us = (float) timerTicks / 15.625f; 
  distance_to_return = (float) duration_us * 0.017f; 
  return distance_to_return;
}

void EXTI9_5_IRQHandler() {
  if (!(EXTI->PR & EXTI_PR_PR9)) {
    return;
  } 
//return;
  EXTI->PR |= EXTI_PR_PR9;

  // If timer enabled
  if ((TIM2->CR1) & (TIM_CR1_CEN)) {
    TIM2->CR1 &= ~TIM_CR1_CEN;
    timerTicks = TIM2->CNT;
  } else {
    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
  }
}

int main(void) {
  setup();
  ms_delay(1000);

  GPIOB->ODR |= MASK(1);

  for (int i = 0; i < 100 * 1000 * 10; i++) {
    ten_microseconds_delay();
  }

  GPIOB->ODR &= ~MASK(1);

  return 0;

  // Main loop
  while (true) {
    float distanceCm = readFrontSensor();
    if (distanceCm != 0 && distanceCm < 5) {
      GPIOB->ODR |= MASK(YELLOW_LED);
      GPIOB->ODR &= ~MASK(RED_LED);
    } else {
      GPIOB->ODR &= ~MASK(YELLOW_LED);
      GPIOB->ODR |= MASK(RED_LED);
    }
    ms_delay(100U);
  }

  return 0;
}
