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

#define L_SPEED 400
#define R_SPEED 300

// wheel sensors: right B13, left B12
#define R_WHEEL_SENSOR 13
#define L_WHEEL_SENSOR 12

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

void setupWheelSensors() {
  // wheel sensor mode is input ("00")
  GPIOB->MODER &= ~GPIO_MODER_MODE12;
  // GPIOB->MODER &= ~GPIO_MODER_MODE13;

  // --- Interrupts ---
  // Setup interrupts for bins B12 and B13
  // Set external interrupts for the Echo pin (B12 and B13)
  EXTI->IMR |= EXTI_IMR_MR12;
  // EXTI->IMR |= EXTI_IMR_MR12 | EXTI_IMR_MR13;
  // rising and falling edge enabled
  // EXTI->RTSR |= EXTI_RTSR_TR12 | EXTI_RTSR_TR13;
  EXTI->RTSR |= EXTI_RTSR_TR12;
  // EXTI->FTSR |= EXTI_FTSR_TR12 | EXTI_FTSR_TR13;
  EXTI->FTSR |= EXTI_FTSR_TR12;

  // we are not expecting any requests right now (just clear this to be sure)
 // EXTI->PR |= EXTI_PR_PR9;

  // Set the external interrupt value in SYSCFG
  // SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PB | SYSCFG_EXTICR4_EXTI13_PB;
  SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PB;

  // Setup interrupt functions
  NVIC_EnableIRQ(EXTI15_10_IRQn);

  // --- Timer ---
  TIM2->PSC = 15; // 2^10 - 1
  TIM2->ARR = 655355;
  TIM2->EGR |= TIM_EGR_UG;
  
  // TIM3->PSC = 15; // 2^10 - 1
  // TIM3->ARR = 655355;
  // TIM3->EGR |= TIM_EGR_UG;
}

void setup() {
  // RCC power to GPIOA, GPIOB, and to ADC and TIM1
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  // RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // LED mode output ("01")
  GPIOB->MODER &= ~GPIO_MODER_MODE1;
  GPIOB->MODER &= ~GPIO_MODER_MODE2;
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  GPIOB->MODER |= GPIO_MODER_MODE2_0;

  setupIR();
  setupMotors();
  setupTim1();
  setupWheelSensors();
}

int leftWheelSensorChanges = 0;
int rightWheelSensorChanges = 0;
int mode12 = 0;
int mode13 = 0;

void EXTI15_10_IRQHandler() {
  if (EXTI->PR & EXTI_PR_PR12) {
    EXTI->PR |= EXTI_PR_PR12;
    
    if (mode12 == 0) {
      // rising edge
      // start timer
      TIM2->EGR |= TIM_EGR_UG;
      TIM2->CR1 |= TIM_CR1_CEN;
      mode12 = 1;
    } else {
      // falling edge
      // stop timer
      int timerTicks = TIM2->CNT;
      TIM2->CR1 &= ~TIM_CR1_CEN;  

      if (timerTicks > 10) {
        leftWheelSensorChanges++;
      }

      mode12 = 0;
    }
  }

  // if (EXTI->PR & EXTI_PR_PR13) {
  //   EXTI->PR |= EXTI_PR_PR13;
    
  //   if (mode13 == 0) {
  //     // rising edge
  //     // start timer
  //     TIM3->EGR |= TIM_EGR_UG;
  //     TIM3->CR1 |= TIM_CR1_CEN;
  //     mode13 = 1;
  //   } else {
  //     // falling edge
  //     // stop timer
  //     int timerTicks = TIM3->CNT;
  //     TIM3->CR1 &= ~TIM_CR1_CEN;  

  //     if (timerTicks > 10) {
  //       rightWheelSensorChanges++;
  //     }

  //     mode13 = 0;
  //   }
  // }

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



float getDrivenDistanceM() {
  // The actual constant will be measured by experimenting
  // (due to bad motors, bad sensors, and my bad coding skills :) )

  // d = 6.4 cm
  // C = pi*d = 20cm
  // 20 changes is 20 cm
  // so 1 change is 1 cm
  // so 100 changes is 1 m

  float totalChanges = (float) leftWheelSensorChanges + (float) rightWheelSensorChanges;
  return totalChanges / 100.0f;
}

void stopMotors() {
  T_CCR_L_BACKWARD = motorCCR;
  T_CCR_L_FORWARD = motorCCR;
  T_CCR_R_BACKWARD = motorCCR;
  T_CCR_R_FORWARD = motorCCR;
}

int main(void) {
  setup();
  //warmMotors();

  // Main loop
  while (true) {
    if (getDrivenDistanceM() >= 1.5f) {
      stopMotors();
      break;
    }

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
      stopMotors();
      ms_delay(100U);
    } else if (left) {
     NVIC_DisableIRQ(EXTI15_10_IRQn);

      // right forward, left backward
      T_CCR_L_FORWARD = motorCCR;
      T_CCR_R_BACKWARD = motorCCR;

      T_CCR_L_BACKWARD = L_SPEED; 
      T_CCR_R_FORWARD = R_SPEED;
      ms_delay(100U);
      NVIC_EnableIRQ(EXTI15_10_IRQn);
    } else if (right) {
     NVIC_DisableIRQ(EXTI15_10_IRQn);

      // left forward, right backward
      T_CCR_L_BACKWARD = motorCCR;
      T_CCR_R_FORWARD = motorCCR;

      T_CCR_L_FORWARD = L_SPEED;
      T_CCR_R_BACKWARD = R_SPEED; 
      ms_delay(100U);
      NVIC_EnableIRQ(EXTI15_10_IRQn);
    } else {
      // both forward, it's a win!
      T_CCR_L_BACKWARD = motorCCR;
      T_CCR_R_BACKWARD = motorCCR;

      T_CCR_L_FORWARD = L_SPEED;
      T_CCR_R_FORWARD = R_SPEED;
      ms_delay(100U);
    }


   // NVIC_EnableIRQ(EXTI15_10_IRQn);
  }

  // Mark the end. Do the light show. 
  GPIOB->ODR |= MASK(YELLOW_LED);
  GPIOB->ODR &= ~MASK(RED_LED);
  while (true) {
    GPIOB->ODR ^= MASK(YELLOW_LED) | MASK(RED_LED);
    ms_delay(500);
  }

  return 0;
}
