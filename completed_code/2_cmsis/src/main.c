/**
 * Student name: Aldas Lenksas
 * Student number: 5714192
 *
 */

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

// motors: A8-A11
#define L_FORWARD 10
#define L_BACKWARD 11
#define R_BACKWARD 8
#define R_FORWARD 9

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
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;

  // Both LED mode output ("01")
  GPIOB->MODER |= GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0;

  // Trig mode output ("01")
  GPIOA->MODER &= ~GPIO_MODER_MODE15;
  GPIOA->MODER |= GPIO_MODER_MODE15_0;

  //GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0; // "01" pull-up

  // Echo mode input ("00")
  GPIOB->MODER  &= ~GPIO_MODER_MODE9;

  // motors (A8-A11) mode output ("01")
  GPIOA->MODER &= ~GPIO_MODER_MODE8;
  GPIOA->MODER &= ~GPIO_MODER_MODE9;
  GPIOA->MODER &= ~GPIO_MODER_MODE10;
  GPIOA->MODER &= ~GPIO_MODER_MODE11;

  GPIOA->MODER |= GPIO_MODER_MODE8_0;
  GPIOA->MODER |= GPIO_MODER_MODE9_0;
  GPIOA->MODER |= GPIO_MODER_MODE10_0;
  GPIOA->MODER |= GPIO_MODER_MODE11_0;


 setupFrontUltrasoundInterrupts();

  // Setup timers, TIM2 for 10us delay, TIM3 for interrupts for Echo
  //TIM2->PSC = 16 - 1; // max value
  // 32 seconds = 500 * 1000 * 1000
  // then, 1 us = 15.625 TIM2 ticks
  // then, 235 ticks should be enough for 10 us
  TIM2->ARR = 10000;

  TIM3->PSC = 1023; // 2^10 - 1
  TIM3->ARR = 655355;

  // update (clear) counter and prescaler reg.
  TIM2->EGR |= TIM_EGR_UG;
  TIM3->EGR |= TIM_EGR_UG;
}

void ten_microseconds_delay() {
//  TIM2->CNT = 0;
  TIM2->EGR |= TIM_EGR_UG;

  TIM2->CR1 |= TIM_CR1_CEN;

  // a bit more than 10 us
  while ((TIM2->CNT) < 300) {}

  TIM2->CR1 &= ~TIM_CR1_CEN;
}

int timerTicks;
int interruptState = 0; // 0 - don't do anything, 1 - start timer, 2 - stop timer

float readFrontSensor() {
  interruptState = 1; // starting timer state
  //float distance_to_return = 0;
  timerTicks = 0;

  // Send 10 us Trig pulse
  GPIOA->ODR |= MASK(FRONT_TRIG);
  //  ms_delay(1);
 ten_microseconds_delay();
  GPIOA->ODR &= ~MASK(FRONT_TRIG);

//  while ((GPIOB->IDR) & (MASK(9)) == 0)

  ms_delay(10); // just to be sure that the interrupts happened

  // close interrupt and timer
  interruptState = 0; // don't care state
  //TIM3->CR1 &= ~TIM_CR1_CEN; // stop timer

  if (timerTicks == 0) return 0; // no interrupt happened
  // 6000 ticks = ~20 cm
  // 1 cm = 6000/20 ticks= 300 ticks
  float distanceCm = (float)timerTicks;
  return distanceCm;
}

void EXTI9_5_IRQHandler() {
  if (!(EXTI->PR & EXTI_PR_PR9)) {
    return;
  }
  EXTI->PR |= EXTI_PR_PR9;

  if (interruptState == 0) return;

  // If timer enabled
  if (interruptState == 2) {
    timerTicks = TIM3->CNT;
    TIM3->CR1 &= ~TIM_CR1_CEN;
    interruptState = 0; // don't care
  } else { // interrupt state == 1
    TIM3->EGR |= TIM_EGR_UG;
    // TIM3->CNT = 0;
    TIM3->CR1 |= TIM_CR1_CEN;
    interruptState = 2; // next action is stop timer
  }
}

void motorsForward() {
  GPIOA->ODR &= ~MASK(L_BACKWARD);
  GPIOA->ODR &= ~MASK(R_BACKWARD);
  GPIOA->ODR |= MASK(L_FORWARD);
  GPIOA->ODR |= MASK(R_FORWARD);

  // Turn on yellow led
  GPIOB->ODR &= ~MASK(RED_LED);
  GPIOB->ODR |= MASK(YELLOW_LED);
}

void motorsBackward() {
  GPIOA->ODR &= ~MASK(L_FORWARD);
  GPIOA->ODR &= ~MASK(R_FORWARD);
  GPIOA->ODR |= MASK(L_BACKWARD);
  GPIOA->ODR |= MASK(R_BACKWARD);

  // Turn on both leds
  GPIOB->ODR |= MASK(RED_LED);
  GPIOB->ODR |= MASK(YELLOW_LED);
}

void motorsStill() {
  // no move
  GPIOA->ODR &= ~MASK(L_BACKWARD);
  GPIOA->ODR &= ~MASK(R_BACKWARD);
  GPIOA->ODR &= ~MASK(L_FORWARD);
  GPIOA->ODR &= ~MASK(R_FORWARD);

  // Turn on red led
  GPIOB->ODR |= MASK(RED_LED);
  GPIOB->ODR &= ~MASK(YELLOW_LED);
}

int main(void) {
  setup();

  // Give a kick of full speed to motors for one second.
  motorsForward();
  ms_delay(1000);
  motorsStill();
  ms_delay(1000);

  // In Arduino, I implemented this with Finite State Machine
  // But as it is not necessary to have a nice FSM, here I will
  // just use if statements.

  while (true) {
    float distanceCm = readFrontSensor();
    if (timerTicks == 0 || distanceCm > 20.0f) {
      // Can still drive forward.
      motorsForward();
    } else if (distanceCm < 10.0f) {
      // Drive back.
      motorsBackward();
    } else {
      // Stay still.
      motorsStill();
    }

    ms_delay(100);
  }

  return 0;
}

// below main loop is for testing the ultrasonic front sensor
/*
int main(void) {
  setup();

  // GPIOA->ODR |= MASK(L_FORWARD);
  // GPIOA->ODR |= MASK(R_FORWARD);

  // Main loop
  while (true) {
    float distanceCm = readFrontSensor();
    if (timerTicks != 0 && distanceCm < 20) {
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
*/
