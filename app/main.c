#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>

int unlocked = 0;
int unlocking = 1;
int locked = 2;
int status = -1;
int pattern = -1;
int pattern1 = 0;                       
int pattern2 = 0;                           
int pattern3 = 0;
int pattern3_step = 0;
int current_pattern = -1;   
int next_pattern = -1;                        
float base_transition_scalar = 1.0;             

#define LED_BAR P1OUT

void init(void) {
    WDTCTL = WDTPW | WDTHOLD;                    // Stop watchdog timer           
    PM5CTL0 &= ~LOCKLPM5;                        // Disable High Z mode

    // Set P2.0 (red), P2.1 (green), P2.2 (blue) as outputs for RGB led
    P2DIR |= (BIT0 | BIT1 | BIT2);  
    P2OUT &= ~(BIT0 | BIT1 | BIT2); 

    // Set P1.0-P1.7 as outputs for led bar
    P1DIR |= 0b11111111; 
    P1OUT |= 0b00000000;  

    // Configure Timer B0
    TB0CTL |= (TBSSEL__ACLK | MC__UP | TBCLR); // Use ACLK, up mode, clear
    TB0CCR0 = (int)(base_transition_scalar * 32768);

    // Configure Timer B3
    TB3CTL |= (TBSSEL__SMCLK | MC__UP | TBCLR);  // Use SMCLK, up mode, clear
    TB3CCR0 = 16320;                              // Set PWM period (adjust for desired frequency)

    // Enable and clear interrupts for each color channel
    TB0CCTL0 |= CCIE;                            // Interrupt for pattern transistions
    TB0CCTL0 &= ~CCIFG;
    TB3CCTL0 |= CCIE;                            // Interrupt for base period
    TB3CCTL0 &= ~CCIFG; 
    TB3CCTL1 |= CCIE;                            // Interrupt for Red
    TB3CCTL1 &= ~CCIFG; 
    TB3CCTL2 |= CCIE;                            // Interrupt for Green
    TB3CCTL2 &= ~CCIFG;
    TB3CCTL3 |= CCIE;                            // Interrupt for Blue
    TB3CCTL3 &= ~CCIFG;

    __enable_interrupt();                        // enable interrupts
}

int main(void)
{
    init();

    while(1) {
        status = unlocking;
        update_rgb_led(status);
    }
}

void update_leds(int status, int pattern) {
    update_rgb_led(status);
    update_led_bar(status, pattern);
}

void update_rgb_led(int status) {

    if (status == unlocked) {                
        set_rgb_led_pwm(254,1,1);     // red
    } else if (status == unlocking) {         
        set_rgb_led_pwm(254,25,1);    // orange
    } else if (status == locked) {        
        set_rgb_led_pwm(1,1,254);    // blue
    } else {
        set_rgb_led_pwm(0,0,0);         // shut off rgb led
    }
}

void set_rgb_led_pwm(int red, int green, int blue) {
    TB3CCR1 = red*64;   // Red brightness
    TB3CCR2 = green*64; // Green brightness
    TB3CCR3 = blue*64;  // Blue brightness
}

void update_led_bar(int status, int pattern) {

    if (status == unlocked) {                   
        next_pattern = pattern;

        if (next_pattern == current_pattern) {
            if (pattern == 1) {
                pattern1 = 0b10101010;
            } else if (pattern == 2) {
                pattern2 = 0b00000000;
            } else if (pattern == 3) {
                pattern3 = 0b00011000;
                pattern3_step = 0;
            }
        }

        if (pattern == 0) {                 // static    
            LED_BAR = 0b10101010;
        } else if (pattern == 1) {          // toggle      
            base_transition_scalar = 1.0;   
            LED_BAR = pattern1;
        } else if (pattern == 2) {          // up counter            
            base_transition_scalar = 0.5;
            LED_BAR = pattern2;
        } else if (pattern == 3) {          // in and out    
            base_transition_scalar = 0.5;
            LED_BAR = pattern3;
        }
        current_pattern = next_pattern;
    }

    TB0CCR0 = (int)(base_transition_scalar * 32768);
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Pattern_Transition_ISR(void) {
    if (status == unlocked) {
        if (pattern == 1) {
            if (pattern1 == 0b01010101) {
                pattern1 = 0b10101010;        
            } else {
                pattern1 = 0b01010101;
            }
        } else if (pattern == 2) {
            if (pattern2 != 0b11111111) {
                pattern2 = pattern2 + 0b00000001;
            } else {
                pattern2 = 0b00000000;
            }
        } else if (pattern == 3) {
            if (pattern3_step == 0) {
                pattern3 = 0b00100100;
                pattern3_step = 1;
            } else if (pattern3_step == 1) {
                pattern3 = 0b01000010;
                pattern3_step = 2;
            } else if (pattern3_step == 2) {
                pattern3 = 0b10000001;
                pattern3_step = 3;
            } else if (pattern3_step == 3) {
                pattern3 = 0b01000010;
                pattern3_step = 4;
            } else if (pattern3_step == 4) {
                pattern3 = 0b00100100;
                pattern3_step = 5;
            } else if (pattern3_step == 5) {
                pattern3 = 0b00011000;
                pattern3_step = 0;
            }
        }
        update_led_bar(status, pattern);
    }
    TB0CCTL0 &= ~ CCIFG;            // Clear interrupt flag
}

#pragma vector = TIMER3_B0_VECTOR
__interrupt void RGB_Period_ISR(void) {
    P2OUT |= (BIT0 | BIT1 | BIT2);  // Turn ON all LEDs at start of period
    TB3CCTL0 &= ~CCIFG;             // Clear interrupt flag
}

#pragma vector = TIMER3_B1_VECTOR
__interrupt void RGB_Duty_ISR(void) {
    switch (TB3IV) {
        case 0x02:  // TB3CCR1 - Red
            P2OUT &= ~BIT0; // Turn OFF Red
            TB3CCTL1 &= ~CCIFG; 
            break;
        case 0x04:  // TB3CCR2 - Green
            P2OUT &= ~BIT1; // Turn OFF Green
            TB3CCTL2 &= ~CCIFG;
            break;
        case 0x06:  // TB3CCR3 - Blue
            P2OUT &= ~BIT2; // Turn OFF Blue
            TB3CCTL3 &= ~CCIFG;
            break;        
    }
}   