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

#define AIN BIT1

float logR, R, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float R_t = 10400.0; //Known Resistance of Thermistor

void register_settings_for_ADC10()
{
    ADC10AE0 |= AIN;                            // P1.0 ADC option select
    ADC10CTL1 = INCH_1;                         // ADC Channel -> 1 (P1.0)
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON; // Ref -> Vcc, 64 CLK S&H , ADC - ON
}

/*@brief entry point for the code*/
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // Stop watchdog timer

    //   BCSCTL1 |= (BIT0 + BIT1 + BIT2 + BIT3);     // Selecting RSELx as 15
    //  DCOCTL  |= (BIT6 + BIT5 + BIT4);            // Selecting DCOx as 7, DCO_freq = 15.6 MHz

    register_settings_for_ADC10();              // Register setting for ADC10

    initialise_SerialPrint_on_lunchbox(); // Function to initialize Serial on LunchBox using LunchboxCommon.h

    unsigned int i;
    while (1)
    {
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

        while (ADC10CTL1 & ADC10BUSY)
            ;           // Wait for conversion to end

        float adc_value = 0;

        adc_value = (ADC10MEM) * (3.30) / (1023.00); // mapping 10-bit conversion result of ADC to corresponding voltage

        int int_part = adc_value;        // Integer part of calculated ADC value
        int decimal_part = (adc_value - (float) int_part) * 1000.0; // Decimal part of calculated ADC value
        printf("Value: %d.%d \n\r", int_part, decimal_part);

        printf("ADCMEM: %d\n\r", ADC10MEM);

        R = R_t * (1023.0 / (float) ADC10MEM - 1.0); //Using Voltage divider for finding the Resistor Value
        logR = log(R);
        T = (1.0 / (c1 + c2 * logR + c3 * logR * logR * logR)); //Steinhart-Hart Equation
        T = T - 273.15;

        int int_temp = T;        // Integer part of calculated ADC value
        int decimal_temp = (T - (float)int_temp) * 1000.0; // Decimal part of calculated ADC value

        printf("Temperature: %d.%d C\n\r", int_temp, decimal_temp);

        __delay_cycles(1100000);

    }
}
