#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>


    int i = 0;
    int status = 0;
    int col = 0;
 
    char key = 'N';        // starts the program at NA until a key gets pressed

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    
    P1DIR   |= BIT0;

    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;


   /* while(true)
    {
        P1OUT^= BIT0;
        for(i=0;    i<0xFFFF; i++);
        {

        }
    }*/
    get_key();
    main();
}    /*
    check_keypad(status){
        if(status == 0){
            if(key = code_int_1){
                status = 1
            }
        }
    } 
    */

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
    P3DIR |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set keypad row pins as outputs

    P1DIR &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); // clears outputs to start


    // Set column pins as input with pull-down resistor
    P1DIR &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7); // Set P1.6 and P1.7 as input
    P1REN |=  (BIT4   |   BIT5   |   BIT6   |   BIT7);  // Enable pull-up/down resistors
    P1OUT &= ~(BIT4   |   BIT5   |   BIT6   |   BIT7);  // Set as pull-down


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


