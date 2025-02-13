#include "msp430fr2355.h"
#include <msp430.h>
#include <stdbool.h>

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    
    P1DIR   |= BIT0;

    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;

    int i = 0;
    
    while(true)
    {
        P1OUT^= BIT0;
        for(i=0;    i<0xFFFF; i++)
        {

        }
    }
}
