/*
 * Analog_read.h
 *
 *  Created on: 05-Feb-2022
 *      Author: hp
 */

#include <msp430.h>

#ifndef ANALOG_READ_H_
#define ANALOG_READ_H_

unsigned int adc[8];


void register_settings_for_ADC10();
void start_sampling();
float calc_temp();
int calc_shade();
#endif /* ANALOG_READ_H_ */
