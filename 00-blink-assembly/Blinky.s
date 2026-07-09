@-----------------------------------------------------------------------------
@ Name:    Blinky.s
@ Purpose: LED Flasher (Cortex-M4 assembly, GNU/AT&T syntax)
@
@ Onboard LED (PC13, active low) blinks at a fixed rate using a busy-wait
@ delay loop. Blinking pauses while the USER key (PA0, active low) is held.
@ Runs on the default 16 MHz HSI reset clock - no PLL/clock configuration.
@-----------------------------------------------------------------------------

    .syntax unified
    .thumb

    .equ RCC_BASE,           0x40023800
    .equ RCC_AHB1ENR,        RCC_BASE + 0x30

    .equ GPIOA_BASE,         0x40020000
    .equ GPIOA_MODER,        GPIOA_BASE + 0x00
    .equ GPIOA_PUPDR,        GPIOA_BASE + 0x0C
    .equ GPIOA_IDR,          GPIOA_BASE + 0x10

    .equ GPIOC_BASE,         0x40020800
    .equ GPIOC_MODER,        GPIOC_BASE + 0x00
    .equ GPIOC_BSRR,         GPIOC_BASE + 0x18

    .equ RCC_AHB1ENR_GPIOAEN, (1 << 0)
    .equ RCC_AHB1ENR_GPIOCEN, (1 << 2)

    .equ LED_PIN,            13   @ PC13, active low
    .equ BTN_PIN,             0   @ PA0, User key, active low

    @ Busy-wait iteration count for ~500 ms at the default 16 MHz HSI clock.
    @ Approximate (not cycle-calibrated) - adjust if the blink rate is off.
    .equ DELAY_COUNT,        400000

    .section .text
    .thumb_func
    .global main
main:
    @ Enable GPIOA and GPIOC clocks
    ldr   r0, =RCC_AHB1ENR
    ldr   r1, [r0]
    movs  r2, #(RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN)
    orrs  r1, r1, r2
    str   r1, [r0]

    @ PC13 as general purpose output (MODER = 01)
    ldr   r0, =GPIOC_MODER
    ldr   r1, [r0]
    movs  r2, #3
    lsls  r2, r2, #(LED_PIN * 2)
    bics  r1, r1, r2
    movs  r2, #1
    lsls  r2, r2, #(LED_PIN * 2)
    orrs  r1, r1, r2
    str   r1, [r0]

    @ PA0 as input (MODER = 00) with pull-up (PUPDR = 01)
    ldr   r0, =GPIOA_MODER
    ldr   r1, [r0]
    movs  r2, #3
    bics  r1, r1, r2
    str   r1, [r0]

    ldr   r0, =GPIOA_PUPDR
    ldr   r1, [r0]
    movs  r2, #3
    bics  r1, r1, r2
    movs  r2, #1
    orrs  r1, r1, r2
    str   r1, [r0]

main_loop:
    @ LED on: active low -> reset the pin via BSRR[LED_PIN + 16]
    ldr   r0, =GPIOC_BSRR
    movs  r1, #1
    lsls  r1, r1, #(LED_PIN + 16)
    str   r1, [r0]

    bl    delay

wait_release_1:
    bl    button_pressed
    cmp   r0, #0
    bne   wait_release_1

    @ LED off: set the pin via BSRR[LED_PIN]
    ldr   r0, =GPIOC_BSRR
    movs  r1, #1
    lsls  r1, r1, #LED_PIN
    str   r1, [r0]

    bl    delay

wait_release_2:
    bl    button_pressed
    cmp   r0, #0
    bne   wait_release_2

    b     main_loop

@-----------------------------------------------------------------------------
@ button_pressed: returns 1 in r0 if PA0 reads low (button held), else 0
@-----------------------------------------------------------------------------
    .thumb_func
button_pressed:
    ldr   r0, =GPIOA_IDR
    ldr   r0, [r0]
    ubfx  r0, r0, #BTN_PIN, #1
    eors  r0, r0, #1
    bx    lr

@-----------------------------------------------------------------------------
@ delay: busy-wait loop, ~500 ms at the default 16 MHz HSI clock
@-----------------------------------------------------------------------------
    .thumb_func
delay:
    ldr   r0, =DELAY_COUNT
delay_loop:
    subs  r0, r0, #1
    bne   delay_loop
    bx    lr

    .end
