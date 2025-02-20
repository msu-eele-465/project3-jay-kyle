#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>

void init(void) {

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;            
    
    // Disable High Z mode
    PM5CTL0 &= ~LOCKLPM5;

    // Configure Timer B0
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR;    // Use SMCLK, up mode, clear
    TB3CCR0 = 4080;                             // Set PWM period (adjust for desired frequency)

    // Enable and clear interrupts for each color channel
    TB3CCTL0 |= CCIE;                            // Interrupt for base period
    TB3CCTL0 &= ~CCIFG; 
    TB3CCTL1 |= CCIE;                            // Interrupt for Red
    TB3CCTL1 &= ~CCIFG; 
    TB3CCTL2 |= CCIE;                            // Interrupt for Green
    TB3CCTL2 &= ~CCIFG;
    TB3CCTL3 |= CCIE;                            // Interrupt for Blue
    TB3CCTL3 &= ~CCIFG;
}

int main(void)
{
    init();

    while(1) {
        update_rgb_led();
    }

    return 0;
}

void set_rgb_led_pwm(int red, int green, int blue) {
    TB3CCR1 = red*16;   // Red brightness
    TB3CCR2 = green*16; // Green brightness
    TB3CCR3 = blue*16;  // Blue brightness
}

void update_rgb_led() {

    //set_rgb_led_pwm(196,62,29);
    set_rgb_led_pwm(196,146,29);

    //if (status == 0) {                  // unlocked
    //    set_rgb_led_pwm(196,62,29);     // red
    //}
    //else if (status == 1) {             // unlocking
    //    set_rgb_led_pwm(196,146,29);    // orange
    //}
    //else {                              // locked
    //    set_rgb_led_pwm(29,162,196);    // blue
    //}
}

#pragma vector = TIMER3_B0_VECTOR
__interrupt void RGB_Period_ISR(void) {
    P2OUT |= (BIT0 | BIT1 | BIT2);  // Turn ON all LEDs at start of period
    TB3CCTL0 &= ~CCIFG;             // Clear interrupt flag
}

#pragma vector = TIMER3_B1_VECTOR
__interrupt void RGB_Duty_ISR(void) {
    switch (TB3IV) {
        case 2:  // TB3CCR1 - Red
            P2OUT &= ~BIT0; // Turn OFF Red
            TB3CCTL1 &= ~CCIFG; 
            break;
        case 4:  // TB3CCR2 - Green
            P2OUT &= ~BIT1; // Turn OFF Green
            TB3CCTL2 &= ~CCIFG;
            break;
        case 6:  // TB3CCR3 - Blue
            P2OUT &= ~BIT2; // Turn OFF Blue
            TB3CCTL3 &= ~CCIFG;
            break;
    }
}