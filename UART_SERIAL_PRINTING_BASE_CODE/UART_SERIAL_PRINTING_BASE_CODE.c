#include <msp430.h>
#include <math.h>
#include <Library/LunchboxCommon.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>


/**
 * @brief
 * These settings are w.r.t. enabling ADC10 on LunchBox
 **/


/*@brief entry point for the code*/
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;                   // Stop watchdog timer


    BCSCTL1 |= (BIT0 + BIT1 + BIT2 + BIT3);     // Selecting RSELx as 15
    DCOCTL  |= (BIT6 + BIT5 + BIT4);            // Selecting DCOx as 7, DCO_freq = 15.6 MHz

 //   register_settings_for_ADC10();              // Register setting for ADC10

    initialise_SerialPrint_on_lunchbox();       // Function to initialize Serial on LunchBox using LunchboxCommon.h

    unsigned int i;
    while(1)
    {

        printf("Hello\n\r");

        for (i = 0; i < 20000; i++);
    }
}
