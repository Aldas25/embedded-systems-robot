#include "utils.hpp"

#include "stm32f4xx.h"
#include <stdbool.h>

// leds B1, B2
#define YELLOW_LED 2
#define RED_LED 1

// IR sensors A0, A1
#define IR_LEFT 1
#define IR_RIGHT 0

#define IR_RIGHT_THRESHOLD 1000
#define IR_LEFT_THRESHOLD 1200

// motors (A8 - A11)
#define R_BACKWARD 8
#define R_FORWARD 9
#define L_FORWARD 10
#define L_BACKWARD 11
#define T_CCR_L_BACKWARD TIM1->CCR3
#define T_CCR_L_FORWARD TIM1->CCR4
#define T_CCR_R_BACKWARD TIM1->CCR2
#define T_CCR_R_FORWARD TIM1->CCR1

#define L_SPEED 800
#define R_SPEED 800

#define MASK(x) (1L << (x))

int motorCCR = 2500;
int prescaler = 16;

void setupIR() {
  // IR mode analog ("11")
  GPIOA->MODER |= GPIO_MODER_MODE0;
  GPIOA->MODER |= GPIO_MODER_MODE1;

  // AD converter is on (ADON)
  ADC1->CR2 &= ~ADC_CR2_ADON; // turn off AD just in case
 
  // not Continuous mode (CONT)
  ADC1->CR2 &= ~ADC_CR2_CONT;

  // no Scan enable (needed for continuous mode)
  ADC1->CR1 &= ~ADC_CR1_SCAN;
  
  // Set sampling speed
  ADC1->SMPR2 |= ADC_SMPR2_SMP0_0; // set to 15 cycles

  // Set the channels in SQR (set the regular channels: CONT mode only works with them)
  // Set the number of conversions to 1 (L should be "0000")
  //ADC1->SQR1 &= ~ADC_SQR1_L;
  // Set first conversion to A0 ("00000")
  //ADC1->SQR3 &= ~ADC_SQR3_SQ1; // clear to "00000"

  // Finally, start the conversion of the regular channels (SWSTART)
  ADC1->CR2 |= ADC_CR2_ADON; // turn on AD
 // ADC1->CR2 |= ADC_CR2_SWSTART;
}

void setupMotors() {
  // Setup Motor pin modes (A8-A11) all to Alt.Functions (mode "10")
  GPIOA->MODER &= ~GPIO_MODER_MODE8;
  GPIOA->MODER &= ~GPIO_MODER_MODE9;
  GPIOA->MODER &= ~GPIO_MODER_MODE10;
  GPIOA->MODER &= ~GPIO_MODER_MODE11;
  GPIOA->MODER |= GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1; 

  // Setup alternating functions in motor pins (to AF1 due to TIM1)
  // Using AFRH (8-11)
  // AF1 is "0001"
  GPIOA->AFR[1] |= (0x1U) | (0x1U << 4) | (0x1U << 8) | (0x1U << 12);
}

void setupTim1() {
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

void setup() {
  // RCC power to GPIOA, GPIOB, and to ADC and TIM1
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

  // LED mode output ("01")
  GPIOB->MODER &= ~GPIO_MODER_MODE1;
  GPIOB->MODER &= ~GPIO_MODER_MODE2;
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  GPIOB->MODER |= GPIO_MODER_MODE2_0;

  setupIR();
  setupMotors();
  setupTim1();
}

uint16_t readIR(bool right) {
  // Set "00000" (A0) channel to use
  ADC1->SQR3 &= ~ADC_SQR3_SQ1; // clear to "00000"
  if (!right) {
    ADC1->SQR3 |= ADC_SQR3_SQ1_0; // set to "00001"
  }

  // Start conversion
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // Wait until EOC is on (End of conversion)
  while (!((ADC1->SR) & (ADC_SR_EOC))) {}

  // The last converted value is stored in Data Register
  // it is 32-bit, but only the last 16 bits are used.
  return ADC1->DR;
}

bool readIRLeft() {
  uint16_t result = readIR(false);
  return result > IR_LEFT_THRESHOLD;
}

bool readIRRight() {
  uint16_t result = readIR(true);
  return result > IR_RIGHT_THRESHOLD;
}

void warmMotors() {
  GPIOB->ODR |= MASK(YELLOW_LED);

  T_CCR_L_FORWARD = 10;
  T_CCR_R_FORWARD = 10;

  ms_delay(1000);

  T_CCR_L_FORWARD = motorCCR; 
  T_CCR_R_FORWARD = motorCCR;

  GPIOB->ODR &= ~MASK(YELLOW_LED);
  ms_delay(2000);
}

int main(void) {
  setup();
  warmMotors();

  // Main loop
  while (true) {
    bool left = readIRLeft();
    bool right = readIRRight();

    if (left) {
      GPIOB->ODR |= MASK(YELLOW_LED);
    } else {
      GPIOB->ODR &= ~MASK(YELLOW_LED);
    }

    if (right) {
      GPIOB->ODR |= MASK(RED_LED);
    } else {
      GPIOB->ODR &= ~MASK(RED_LED);
    }

    if (left && right) {
      // f*ck, stop
      T_CCR_L_BACKWARD = motorCCR;
      T_CCR_L_FORWARD = motorCCR;
      T_CCR_R_BACKWARD = motorCCR;
      T_CCR_R_FORWARD = motorCCR;
    } else if (left) {
      // right forward, left backward
      T_CCR_L_FORWARD = motorCCR;
      T_CCR_R_BACKWARD = motorCCR;

      T_CCR_L_BACKWARD = L_SPEED;
      T_CCR_R_FORWARD = R_SPEED;
    } else if (right) {
      // left forward, right backward
      T_CCR_L_BACKWARD = motorCCR;
      T_CCR_R_FORWARD = motorCCR;

      T_CCR_L_FORWARD = L_SPEED;
      T_CCR_R_BACKWARD = R_SPEED;
    } else {
      // both forward, it's a win!
      T_CCR_L_BACKWARD = motorCCR;
      T_CCR_R_BACKWARD = motorCCR;

      T_CCR_L_FORWARD = L_SPEED;
      T_CCR_R_FORWARD = R_SPEED;
    }

    ms_delay(100U);
  }

  return 0;
}
