/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2015 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "stm32f4xx.h"                  // Device header


extern int stdout_init (void);

#define LED_PIN                   13U   /* PC13, active low            */
#define BTN_PIN                    0U   /* PA0  User key, active low   */

volatile uint32_t msTicks;                            /* counts 1ms timeTicks */
/*----------------------------------------------------------------------------
 * SysTick_Handler:
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;
}

/*----------------------------------------------------------------------------
 * Delay: delays a number of Systicks
 *----------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }
}

/*----------------------------------------------------------------------------
 * SystemCoreClockConfigure: configure SystemCoreClock using the 25 MHz HSE
                             crystal populated on the Black Pill board
 *----------------------------------------------------------------------------*/
void SystemCoreClockConfigure(void) {

  RCC->CR |= ((uint32_t)RCC_CR_HSION);                     /* Enable HSI */
  while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  /* Wait for HSI Ready */

  RCC->CFGR = RCC_CFGR_SW_HSI;                             /* HSI is system clock */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  /* Wait for HSI used as system clock */

  FLASH->ACR  = FLASH_ACR_PRFTEN;                          /* Enable Prefetch Buffer */
  FLASH->ACR |= FLASH_ACR_ICEN;                            /* Instruction cache enable */
  FLASH->ACR |= FLASH_ACR_DCEN;                            /* Data cache enable */
  FLASH->ACR |= FLASH_ACR_LATENCY_5WS;                     /* Flash 5 wait state */

  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                         /* HCLK = SYSCLK */
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;                        /* APB1 = HCLK/4 */
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;                        /* APB2 = HCLK/2 */

  RCC->CR |= RCC_CR_HSEON;                                 /* Enable HSE (25 MHz crystal) */
  while ((RCC->CR & RCC_CR_HSERDY) == 0);                  /* Wait for HSE Ready */

  RCC->CR &= ~RCC_CR_PLLON;                                /* Disable PLL */

  /* PLL configuration:  VCO = HSE/M * N,  Sysclk = VCO/P */
  RCC->PLLCFGR = ( 25ul                   |                /* PLL_M =  25 */
                 (192ul <<  6)            |                /* PLL_N = 192 */
                 (  1ul << 16)            |                /* PLL_P =   4 */
                 (RCC_PLLCFGR_PLLSRC_HSE) |                /* PLL_SRC = HSE */
                 (  4ul << 24)             );              /* PLL_Q =   4 */

  RCC->CR |= RCC_CR_PLLON;                                 /* Enable PLL */
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           /* Wait till PLL is ready */

  RCC->CFGR &= ~RCC_CFGR_SW;                               /* Select PLL as system clock source */
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  /* Wait till PLL is system clock src */
}

/*----------------------------------------------------------------------------
 * LED_Initialize / LED_On / LED_Off: onboard LED on PC13 (active low)
 *----------------------------------------------------------------------------*/
static void LED_Initialize (void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;                     /* Enable GPIOC clock */

  GPIOC->MODER &= ~(3ul << (LED_PIN * 2));
  GPIOC->MODER |=  (1ul << (LED_PIN * 2));                 /* PC13 as output */
  GPIOC->BSRR   =  (1ul <<  LED_PIN);                      /* LED off (idle high) */
}

static void LED_On (void)  { GPIOC->BSRR = (1ul << (LED_PIN + 16)); }
static void LED_Off (void) { GPIOC->BSRR = (1ul <<  LED_PIN); }

/*----------------------------------------------------------------------------
 * Button_Initialize / Button_Pressed: user key on PA0 (active low)
 *----------------------------------------------------------------------------*/
static void Button_Initialize (void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;                     /* Enable GPIOA clock */

  GPIOA->MODER &= ~(3ul << (BTN_PIN * 2));                 /* PA0 as input */
  GPIOA->PUPDR &= ~(3ul << (BTN_PIN * 2));
  GPIOA->PUPDR |=  (1ul << (BTN_PIN * 2));                 /* pull-up */
}

static uint32_t Button_Pressed (void) {
  return ((GPIOA->IDR & (1ul << BTN_PIN)) == 0U);
}

/*----------------------------------------------------------------------------
 * main: blink LED and check button state
 *----------------------------------------------------------------------------*/
int main (void) {

  SystemCoreClockConfigure();                              /* configure HSE/PLL as System Clock */
  SystemCoreClockUpdate();

  LED_Initialize();
  Button_Initialize();
  stdout_init();                                           /* Initializ Serial interface */

  SysTick_Config(SystemCoreClock / 1000);                  /* SysTick 1 msec interrupts */

  for (;;) {
    LED_On();                                              /* Turn LED on */
    Delay(500);                                            /* Wait 500ms */
    while (Button_Pressed());                              /* Wait while holding USER key */
    LED_Off();                                             /* Turn LED off */
    Delay(500);                                            /* Wait 500ms */
    while (Button_Pressed());                              /* Wait while holding USER key */

    printf ("Hello World\n\r");
  }

}
