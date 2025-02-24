#include "intrinsics.h"
#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>

int i = 0;              // heartbeet LED integer
int j = 1;

int status = 0;         // status of the keypad
int unlocked = 2;       
int unlocking = 1;
int locked = 0;

int col = 0;            // variable that marks what columb of the keypad was pressed
char code_char_1 = '5';
char code_char_2 = '2';
char code_char_3 = '9';
char code_char_4 = '3';
int key_num = 0;        // this variable tracks ammt of correct nombers pressed in password
int period = 4;         // this will correspond to 1 second changing the integer period variable by 1 should change the period by .25 s
int key_pad_flag = 0;
int int_en = 0;         //stops intterupt from flagging after inputs go high
int pressed = 0;
int unlocking_flag = 0;
int five_sec = 5;

char pattern = '8';     // default patern
char current_pattern = 'C';   
char next_pattern = 'K';    
int pattern1 = 0;                       
int pattern2 = 0;                           
int pattern3 = 0;
int pattern4 = 0;
int pattern5 = 0;
int pattern6 = 0;
int pattern7 = 0;
int pattern3_step = 0;
int pattern6_step = 0;
float base_transition_scalar = 1.0;        

#define LED_BAR P3OUT

char key = 'N';         // starts the program at NA until a key gets pressed

void init_rgb_led(void) {
    WDTCTL = WDTPW | WDTHOLD;                    // Stop watchdog timer           
    PM5CTL0 &= ~LOCKLPM5;                        // Disable High Z mode

    // Set P2.0 (red), P2.1 (green), P2.2 (blue) as outputs for RGB led
    P6DIR |= (BIT0 | BIT1 | BIT2);  
    P6OUT &= ~(BIT0 | BIT1 | BIT2); 

    // Configure Timer B3
    TB3CTL |= (TBSSEL__SMCLK | MC__UP | TBCLR);  // Use SMCLK, up mode, clear
    TB3CCR0 = 16320;                              // Set PWM period (adjust for desired frequency)

    // Enable and clear interrupts for each color channel
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
    
void init_led_bar(void) {
    WDTCTL = WDTPW | WDTHOLD;                    // Stop watchdog timer           
    PM5CTL0 &= ~LOCKLPM5;                        // Disable High Z mode

    // Set P3.0-P3.7 as outputs for led bar
    P3DIR |= 0b11111111; 
    P3OUT |= 0b00000000;  

    // Configure Timer B0
    TB0CTL |= (TBSSEL__ACLK | MC__UP | TBCLR); // Use ACLK, up mode, clear
    TB0CCR0 = (int)(base_transition_scalar * 32768);

    // Enable and clear interrupts for each color channel
    TB0CCTL0 |= CCIE;                            // Interrupt for pattern transistions
    TB0CCTL0 &= ~CCIFG;

    __enable_interrupt();                        // enable interrupts
}

void init_keypad(void) {
    WDTCTL = WDTPW | WDTHOLD;                    // Stop watchdog timer           
    PM5CTL0 &= ~LOCKLPM5;                        // Disable High Z mode

    //--set up ports
    P1DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set keypad row pins as outputs
    P1OUT |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // sets output high to start

    // Set column pins as input with pull-down resistor
    P2DIR &= ~(BIT0   |   BIT1   |   BIT2   |   BIT3); // Set P1.4 - p1.7 as input
    P2REN |=  (BIT0   |   BIT1   |   BIT2   |   BIT3); // Enable pull-up/down resistors
    P2OUT &= ~(BIT0   |   BIT1   |   BIT2   |   BIT3); // Set as pull-down
}

int main(void)
{
    init_rgb_led();
    init_led_bar();
    init_keypad();
    update_rgb_led(status, pattern);        // start up RGB led 

    while(1){
        update_rgb_led(status, pattern);
        pressed = (P2IN & 0b00001111);
        if (pressed > 0 && int_en == 0){
            key_pad_flag = 1;
            int_en = 1;
        }
        if (pressed == 0){
            int_en = 0;
        }
        if(key_pad_flag == 1){
            check_keypad();
            update_leds(status, pattern);
            P1DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7); 
            key_pad_flag = 0;                                   // stops the ISR from prematurly setting keypad flag
        }
    }
    return 0;
}    

void get_column()
{
    int col_1 = (P2IN & BIT0) ? 1 : 0;
    int col_2 = (P2IN & BIT1) ? 1 : 0;
    int col_3 = (P2IN & BIT2) ? 1 : 0;
    int col_4 = (P2IN & BIT3) ? 1 : 0;

    if (col_1 == 1) {
        col = 1;
    } 
    
    else if (col_2 == 1) {
        col = 2;
    }

    else if (col_3 == 1) {
        col = 3;
    } 

    else if (col_4 == 1) {
        col = 4;
    } 
    
    else {
        col = 0;  // No key pressed
    }
}

void get_key()
{
    P1OUT &= ~(BIT5   |   BIT6   |   BIT7);  // clears outputs to start other than BIT4
    P1OUT |= BIT4; // Activate first row
    get_column();  

    switch(col){
    case 1:
        key = '1';
        break;
        
    case 2:
        key = '2';
        break;
    
    case 3:
        key = '3';
        break;

    case 4:
        key = 'A';
        break;

    case 0:
        break;
    }
    P1OUT &= ~BIT4;      

    P1OUT |= BIT5; // Activate second row
    get_column();  
        
    switch(col){
    case 1:
        key = '4';
        break;
        
    case 2:
        key = '5';
        break;
    
    case 3:
        key = '6';
        break;

    case 4:
        key = 'B';
        break;

    case 0:
        break;
    }
    P1OUT &= ~BIT5; 


    P1OUT |= BIT6; // Activate third row
    get_column();  

    switch(col){
    case 1:
        key = '7';
        break;
        
    case 2:
        key = '8';
        break;
    
    case 3:
        key = '9';
        break;

    case 4:
        key = 'C';
        break;

    case 0:
        break;
    }
    P1OUT &= ~BIT6; 


    P1OUT |= BIT7; // Activate forth row
    get_column();  

    switch(col){
    case 1:
        key = '*';
        break;
        
    case 2:
        key = '0';
        break;
    
    case 3:
        key = '#';
        break;

    case 4:
        key = 'D';
        break;

    case 0:
        break;
    }
    P1OUT &= ~BIT7; 


}

void check_keypad(){
        get_key();
        if(status == unlocked){
            if(key =='0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7'){
                pattern = key;
            }
            if(key == 'A'){
                if(period > 1){
                    period = period - 1;
                }
                else{
                    period = 1;
                }
            }
            if(key == 'B'){
                period = period + 1;
            }
            if(key == 'D'){
                status = locked; 
                pattern = 'N';
                period = 4;
                five_sec = 5;
                unlocking_flag = 0;
            }
        }
        if(status == unlocking){                // currently being unclocked
            switch(key_num){
            case 1:                     // one correct key has been pressed
                if(key == code_char_2){
                    status = unlocking;
                    key_num = 2;
                }
                else{
                    status = locked;
                    key_num = 0;
                    unlocking_flag = 0;
                }
            break;

            case 2:                     // two correct keys have been pressed
                if(key == code_char_3){
                    status = unlocking;
                    key_num = 3;
                }
                else{
                    status = locked;
                    key_num = 0;
                    unlocking_flag = 0;
                }
            break;

            case 3:                     // Three correct keys have been pressed
                if(key == code_char_4){
                    status = unlocked;         // keypad is unlocked
                    key_num = 4;
                    unlocking_flag = 0;
                }
                else{
                    status = locked;
                    key_num = 0;
                    unlocking_flag = 0;
                }
            break;
            }

        }
        if(status == locked){                // indicates tahat the keypad is locked
            if(key == code_char_1){      // correct first key of the code was pressed
                status = unlocking;             // keypad in unlocking mode
                key_num = 1;            // indicates one correct key has been pressed
            }
            else{
                status = locked;             // re locks the keypad if the incorrect key was pressed
                key_num = 0;
                pattern = 'N';
            }
        }
        
        P1OUT |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // sets output high to start
        key_pad_flag == 0;
    } 

void update_leds(int status, char pattern) {
    update_rgb_led(status, pattern);
    update_led_bar(status, pattern);
}

void update_rgb_led(int status, char pattern) {

    if (status == unlocked) {                
        if (pattern == '0') {
            set_rgb_led_pwm(1,254,1);   // green
        } else if (pattern == '1') {
            set_rgb_led_pwm(130,1,254);  // purple
        } else if (pattern == '2') {
            set_rgb_led_pwm(254,10,130);  // pink
        } else if (pattern == '3') {
            set_rgb_led_pwm(230,90,254);  // white
        } else if (pattern == '4') {
            set_rgb_led_pwm(110,75,1);     // yellow    
        } else if (pattern == '5') {
            set_rgb_led_pwm(50,4,254);              // light blue
        } else if (pattern == '6') {
            set_rgb_led_pwm(254,20,35);              // orange-pink
        } else if (pattern == '7') {
            set_rgb_led_pwm(35,200,150);              // aqua
        } else {
            set_rgb_led_pwm(1,1,254);     // blue
        }
    } else if (status == unlocking) {         
        set_rgb_led_pwm(254,20,1);    // orange
    } else if (status == locked) {        
        set_rgb_led_pwm(254,1,1);    // red
    } else {
        set_rgb_led_pwm(0,0,0);         // shut off rgb led
    }
}

void set_rgb_led_pwm(int red, int green, int blue) {
    TB3CCR1 = red*64;   // Red brightness
    TB3CCR2 = green*64; // Green brightness
    TB3CCR3 = blue*64;  // Blue brightness
}

void update_led_bar(int status, char pattern) {

    if (status == unlocked) {                   
        next_pattern = pattern;

        if (next_pattern == current_pattern) {
            if (pattern == '1') {
                pattern1 = 0b01010101;
            } else if (pattern == '2') {
                pattern2 = 0b11111111;
            } else if (pattern == '3') {
                pattern3_step = 0;
            } else if (pattern == '4') {
                pattern4 = 0b00000000;
            } else if (pattern == '5') {
                pattern5 = 0b10000000;
            } else if (pattern == '6') {
                pattern6_step = 0;
            } else if (pattern == '7') {
                pattern7 = 0b11111111;
            }
        }
        current_pattern = next_pattern;
    }
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Pattern_Transition_ISR(void) {
    if (status == unlocking){                       // ensures that the system is unlocked in under 5 seconds
        if(key_num == 1 && unlocking_flag == 0){
            TB0CCR0 = 32768;                        // stops the unlocking time being less after period was spead up
            base_transition_scalar = 1.0;
            unlocking_flag = 1;
            five_sec = 5;
        }
        if( five_sec > 0){
            five_sec = five_sec -1;
        }
        else{
            status = locked;
            unlocking_flag = 0;

        }


    }
    if (status == unlocked) {
        if (pattern == '0') {
            LED_BAR = 0b10101010;
        } else if (pattern == '1') {
            base_transition_scalar = 1.0;
            if (pattern1 == 0b01010101) { 
                pattern1 = 0b10101010;       
            } else {
                pattern1 = 0b01010101;
            }
            LED_BAR = pattern1;
        } else if (pattern == '2') {
            base_transition_scalar = 0.5;
            if (pattern2 != 0b11111111) {
                pattern2 = pattern2 + 1;
            } else {
                pattern2 = 0b00000000;
            }
            LED_BAR = pattern2;
        } else if (pattern == '3') {
            base_transition_scalar = 0.5;
            if (pattern3_step == 0) {
                pattern3 = 0b00011000;
                pattern3_step = 1;
            } else if (pattern3_step == 1) {
                pattern3 = 0b00100100;
                pattern3_step = 2;
            } else if (pattern3_step == 2) {
                pattern3 = 0b01000010;
                pattern3_step = 3;
            } else if (pattern3_step == 3) {
                pattern3 = 0b10000001;
                pattern3_step = 4;
            } else if (pattern3_step == 4) {
                pattern3 = 0b01000010;
                pattern3_step = 5;
            } else if (pattern3_step == 5) {
                pattern3 = 0b00100100;
                pattern3_step = 0;
            }
            LED_BAR = pattern3;
        } else if (pattern == '4') {
            base_transition_scalar = 0.25;
            if (pattern4 == 0b00000000) {
                pattern4 = 0b11111111;
            } else {
                pattern4 = pattern4 - 1;
            }
            LED_BAR = pattern4;
        } else if (pattern == '5') {
            base_transition_scalar = 1.5;
            if (pattern5 == 0 || pattern5 == 0b10000000) {
                pattern5 = 0b00000001;
            } else {
                pattern5 = pattern5 * 2;       
            } 
            LED_BAR = pattern5;
        } else if (pattern == '6') {
            base_transition_scalar = 0.5;
            if (pattern6_step == 0) {
                pattern6 = 0b01111111;
                pattern6_step = 1;
            } else if (pattern6_step == 1) {
                pattern6 = 0b10111111;
                pattern6_step = 2;
            } else if (pattern6_step == 2) {
                pattern6 = 0b11011111;
                pattern6_step = 3;
            } else if (pattern6_step == 3) {
                pattern6 = 0b11101111;
                pattern6_step = 4;
            } else if (pattern6_step == 4) {
                pattern6 = 0b11110111;
                pattern6_step = 5;
            } else if (pattern6_step == 5) {
                pattern6 = 0b11111011;
                pattern6_step = 6;
            } else if (pattern6_step == 6) {
                pattern6 = 0b11111101;
                pattern6_step = 7;
            } else if (pattern6_step == 7) {
                pattern6 = 0b11111110;
                pattern6_step = 0;
            }
            LED_BAR = pattern6;
        } else if (pattern == '7') {
            base_transition_scalar = 1.0;
            if (pattern7 != 0b11111111) {
                pattern7 = pattern7 * 2 + 1;       
            } else {
                pattern7 = 0b00000001;
            }
            LED_BAR = pattern7;
        }
        TB0CCR0 = (int)(base_transition_scalar * 8192 * period);
    } else {
        LED_BAR = 0b00000000;
    }
    TB0CCTL0 &= ~ CCIFG;            // Clear interrupt flag
}

#pragma vector = TIMER3_B0_VECTOR
__interrupt void RGB_Period_ISR(void) {
    P6OUT |= (BIT0 | BIT1 | BIT2);  // Turn ON all LEDs at start of period
    TB3CCTL0 &= ~CCIFG;             // Clear interrupt flag
}

#pragma vector = TIMER3_B1_VECTOR
__interrupt void RGB_Duty_ISR(void) {
    switch (TB3IV) {
        case 0x02:  // TB3CCR1 - Red
            P6OUT &= ~BIT0; // Turn OFF Red
            TB3CCTL1 &= ~CCIFG; 
            break;
        case 0x04:  // TB3CCR2 - Green
            P6OUT &= ~BIT1; // Turn OFF Green
            TB3CCTL2 &= ~CCIFG;
            break;
        case 0x06:  // TB3CCR3 - Blue
            P6OUT &= ~BIT2; // Turn OFF Blue
            TB3CCTL3 &= ~CCIFG;
            break;        
    }
}   
