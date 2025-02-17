#include "intrinsics.h"
#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>


    int i = 0;              // heartbeet LED integer
    int status = 0;         // unlock status of the keypad
    int col = 0;            // variable that marks what columb of the keypad was pressed
    char code_char_1 = '5';
    char code_char_2 = '2';
    char code_char_3 = '9';
    char code_char_4 = '3';
    int key_num = 0;        // this variable tracks ammt of correct nombers pressed in password
    char patern_sel = '0';  // default patern
    int period = 4;         // this will correspond to 1 second changing the integer period variable by 1 should change the period by .25 s
    int key_pad_flag = 0;

 
    char key = 'N';         // starts the program at NA until a key gets pressed

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    //--set up ports
    P3DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set keypad row pins as outputs
    P3OUT |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // sets output high to start

    // Set column pins as input with pull-down resistor
    P1DIR &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); // Set P1.4 - p1.7 as input
    P1REN |=  (BIT4   |   BIT5   |   BIT6   |   BIT7); // Enable pull-up/down resistors
    P1OUT &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); // Set as pull-down
    P1IES &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); //configur IQR sensitivity
    //P1IES |= BIT4;

    //--- Set Up port 1 IQR
    P1IFG &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); // clears interrupt flag
    P1IE  |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);
    __enable_interrupt();


    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;


    while(1){
        //P3DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set keypad row pins as outputs
        //P3OUT |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // re sets output high so it can do the next cycle

        if(key_pad_flag == 1){
            check_keypad();
            P3DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7); 
            key_pad_flag = 0;                                   // stops the ISR from prematurly setting keypad flag
        }
    }
    return 0;
    
}    

void get_column()
{
    int col_1 = (P1IN & BIT4) ? 1 : 0;
    int col_2 = (P1IN & BIT5) ? 1 : 0;
    int col_3 = (P1IN & BIT6) ? 1 : 0;
    int col_4 = (P1IN & BIT7) ? 1 : 0;

    if (col_1) {
        col = 1;
    } 
    
    else if (col_2) {
        col = 2;
    }

    else if (col_3) {
        col = 3;
    } 

    else if (col_4) {
        col = 4;
    } 
    
    else {
        col = 0;  // No key pressed
    }
}


void get_key()
{
    P3OUT &= ~(BIT5   |   BIT6   |   BIT7);  // clears outputs to start other than BIT4
    P3OUT |= BIT4; // Activate first row
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
    P3OUT &= ~BIT4;      

    P3OUT |= BIT5; // Activate second row
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
    P3OUT &= ~BIT5; 


    P3OUT |= BIT6; // Activate third row
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
    P3OUT &= ~BIT6; 


    P3OUT |= BIT7; // Activate forth row
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
    P3OUT &= ~BIT7; 


}

void check_keypad(){
        get_key();
        if(status == 2){
            if(key =='0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7'){
                patern_sel = key;
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
        }
        if(status == 1){                // currently being unclocked
            switch(key_num){
            case 1:                     // one correct key has been pressed
                if(key == code_char_2){
                    status = 1;
                    key_num = 2;
                }
                else{
                    status = 0;
                    key_num = 0;
                }
            break;

            case 2:                     // two correct keys have been pressed
                if(key == code_char_3){
                    status = 1;
                    key_num = 3;
                }
                else{
                    status = 0;
                    key_num = 0;
                }
            break;

            case 3:                     // Three correct keys have been pressed
                if(key == code_char_4){
                    status = 2;         // keypad is unlocked
                    key_num = 4;
                }
                else{
                    status = 0;
                    key_num = 0;
                }
            break;
            }

        }
        if(status == 0){                // indicates tahat the keypad is locked
            if(key == code_char_1){      // correct first key of the code was pressed
                status = 1;             // keypad in unlocking mode
                key_num = 1;            // indicates one correct key has been pressed
            }
            else{
                status = 0;             // re locks the keypad if the incorrect key was pressed
                key_num = 0;
            }
        }
        

        P3OUT |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set keypad row pins as outputs
        for(i=0; i<0x00FF; i++){}                           // stops a split second high from keypad 
                               
    } 

//------Interrupt Service Routines
#pragma  vector = PORT1_VECTOR
__interrupt  void ISR_PORT1_kye(void){
    key_pad_flag = 1;
    P1IFG &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7);      // clears interrupt flag, could pottentialy set up so 
                                                            // i could get rid of get column function

}
