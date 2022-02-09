/*
 * Analog_read.c
 *
 *  Created on: 05-Feb-2022
 *      Author: hp
 */
#include <msp430.h>
#include "Analog_read.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void register_settings_for_ADC10()
{
    ADC10AE0 |= BIT3 + BIT2 + BIT1 + BIT0;             // P1.0 ADC option select
    ADC10CTL1 = INCH_4 + ADC10DIV_0 + CONSEQ_3 + SHS_0; // ADC Channel -> 1 (P1.0)
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + MSC; // Ref -> Vcc, 64 CLK S&H , ADC - ON
    ADC10DTC1 = 4;
}

void start_sampling()
{
    ADC10CTL0 &= ~ENC;

    while (ADC10CTL1 & ADC10BUSY)
        ;           // Wait for conversion to end
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

    ADC10SA = (unsigned int) adc;
}

float calc_temp()
{
    float logR, R, T;
    float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    float R_t = 11400.0; //Known Resistance of Thermistor

 //   float adc_value = (adc[2]) * (3.30) / (1023.00); // mapping 10-bit conversion result of ADC to corresponding voltage

    R = R_t * (1023.0 / (float) adc[2] - 1.0); //Using Voltage divider for finding the Resistor Value
    logR = log(R);
    T = (1.0 / (c1 + c2 * logR + c3 * logR * logR * logR)); //Steinhart-Hart Equation
    T = T - 273.15;
//
//    char int_temp = T;        // Integer part of calculated ADC value
//    char decimal_temp = (T - (float) int_temp) * 1000.0; // Decimal part of calculated ADC value
//    char temp = int_part + "." + decimal_part;
    return T;
}

int calc_shade(){
    int S;
    S = map(adc[0], 0, 1023, 0, 99);
    return S;
}
