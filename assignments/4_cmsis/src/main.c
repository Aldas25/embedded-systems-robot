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

#define L_SPEED 600
#define R_SPEED 500

// US: Front trig A15, echo B9
// US: Side trig B3, echo B8
#define FRONT_TRIG 15
#define FRONT_ECHO 9
#define SIDE_TRIG 3
#define SIDE_ECHO 8

#define MASK(x) (1L << (x))

int motorCCR = 2500;
int prescaler = 16;

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

  // TIM2 and TIM3
  TIM2->ARR = 10000;
  
  TIM3->PSC = 1023; // 2^10 - 1
  TIM3->ARR = 655355;

  // update (clear) counter and prescaler reg.
  TIM2->EGR |= TIM_EGR_UG; 
  TIM3->EGR |= TIM_EGR_UG;
}

void setupSideUltrasoundInterrupts() {
  // Set external interrupts for the Echo pin (B8)
  EXTI->IMR |= EXTI_IMR_MR8;
  // rising and falling edge enabled
  EXTI->RTSR |= EXTI_RTSR_TR8;
  EXTI->FTSR |= EXTI_FTSR_TR8;

  // Set the external interrupt value in SYSCFG
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PB;

  // Setup interrupt functions - it is already set when setting front US sensor
  // NVIC_EnableIRQ(EXTI9_5_IRQn);

  // I won't set timers since I will reuse the same timers as for Front US sensor
  // (both sensors won't be used at the same time)
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

float readSideSensor() {
  interruptState = 1; // starting timer state
  //float distance_to_return = 0;
  timerTicks = 0;

  // Send 10 us Trig pulse
  GPIOB->ODR |= MASK(SIDE_TRIG);
  //  ms_delay(1);
 ten_microseconds_delay(); 
  GPIOB->ODR &= ~MASK(SIDE_TRIG);

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
  // Same interrupt handler for both pins (B8 and B9)
  // logic for both is the same, and both US sensors won't be used at the same time

  if (!(EXTI->PR & EXTI_PR_PR9) && !(EXTI->PR & EXTI_PR_PR8)) {
    return;
  } 
  EXTI->PR |= EXTI_PR_PR9;
  EXTI->PR |= EXTI_PR_PR8;

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

void setupUltrasounds() {
  // --- FRONT ---
  // Trig mode output ("01")
  GPIOA->MODER &= ~GPIO_MODER_MODE15;
  GPIOA->MODER |= GPIO_MODER_MODE15_0;
  
  // Echo mode input ("00")
  GPIOB->MODER  &= ~GPIO_MODER_MODE9;

  // --- SIDE ---
  // Trig mode output ("01")
  GPIOB->MODER &= ~GPIO_MODER_MODE3;
  GPIOB->MODER |= GPIO_MODER_MODE3_0;
  
  // Echo mode input ("00")
  GPIOB->MODER  &= ~GPIO_MODER_MODE8;
}

void setup() {
  // RCC power to GPIOA, GPIOB, and to ADC and TIM1
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;


  // LED mode output ("01")
  GPIOB->MODER &= ~GPIO_MODER_MODE1;
  GPIOB->MODER &= ~GPIO_MODER_MODE2;
  GPIOB->MODER |= GPIO_MODER_MODE1_0;
  GPIOB->MODER |= GPIO_MODER_MODE2_0;

  setupIR();
  setupMotors();
  setupTim1();
  setupUltrasounds();
  setupFrontUltrasoundInterrupts();
  setupSideUltrasoundInterrupts();
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

void stopMotors() {
  T_CCR_L_BACKWARD = motorCCR;
  T_CCR_L_FORWARD = motorCCR;
  T_CCR_R_BACKWARD = motorCCR;
  T_CCR_R_FORWARD = motorCCR;
}

void driveForward() {
  T_CCR_L_BACKWARD = motorCCR;
  T_CCR_R_BACKWARD = motorCCR;

  T_CCR_L_FORWARD = L_SPEED;
  T_CCR_R_FORWARD = R_SPEED;
}

void driveLeftForward() {
  T_CCR_L_BACKWARD = motorCCR;
  T_CCR_R_FORWARD = motorCCR;

  T_CCR_L_FORWARD = L_SPEED;
  T_CCR_R_BACKWARD = R_SPEED;
}

void driveRightForward() {
  T_CCR_L_FORWARD = motorCCR;
  T_CCR_R_BACKWARD = motorCCR;

  T_CCR_L_BACKWARD = L_SPEED;
  T_CCR_R_FORWARD = R_SPEED;
}

void goByLine() { 
  while (true) {
    float distanceCm = readFrontSensor();

    if (timerTicks != 0 && distanceCm < 20.0f) {
      stopMotors();
      return; // stop going by line
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
    } else if (left) {
      // right forward, left backward
      driveRightForward();
    } else if (right) {
      // left forward, right backward
      driveLeftForward();
    } else {
      // both forward, it's a win!
      driveForward();
    }

    ms_delay(100U);
  }
}

void turnAndDriveABit() {
  GPIOB->ODR |= MASK(RED_LED);

  driveLeftForward();
  ms_delay(750);
  driveForward();
  ms_delay(800);
  stopMotors();

  GPIOB->ODR &= ~MASK(RED_LED);
}

void goAroundTheObstacle() {
  while (true) {
    bool left = readIRLeft();
    bool right = readIRRight();

    if (left || right) {
      // stop this phase, now will need to continue the line
      stopMotors();
      return;
    }

    float sideSensorDistanceCm = readSideSensor();

    if (timerTicks != 0 && sideSensorDistanceCm < 25.0f) {
      // still drive forward (obstacle is too close on the side)
      driveForward();
      GPIOB->ODR &= ~MASK(YELLOW_LED);
    } else {
      // turn a bit
      driveRightForward();
      GPIOB->ODR |= MASK(YELLOW_LED);
      ms_delay(800);
      driveForward();
      ms_delay(800);
    }

    ms_delay(200);
  }
}

int main(void) {
  setup();
  warmMotors();

  // Could be in a while loop, but now it's also good enough :)
  goByLine(); // until the obstacle is in front
  turnAndDriveABit(); // just to move from the line
  goAroundTheObstacle(); // until the line is found again
  goByLine(); // continue the line

  // Mark end
  GPIOB->ODR |= MASK(YELLOW_LED);
  GPIOB->ODR |= MASK(RED_LED);
  while(true) {
    GPIOB->ODR ^= MASK(YELLOW_LED);
    GPIOB->ODR ^= MASK(RED_LED);
    ms_delay(500);
  }

  return 0;
}

/*
// Testing Side ultrasonic

while (true) {
    float value = readSideSensor();
    if (timerTicks == 0) {
      GPIOB->ODR |= MASK(RED_LED);
    } else {
      GPIOB->ODR &= ~MASK(RED_LED);
    }

    if (timerTicks != 0 && value < 20.0f) {
      GPIOB->ODR |= MASK(YELLOW_LED);
    } else {
      GPIOB->ODR &= ~MASK(YELLOW_LED);
    }

    ms_delay(300);
  }

*/