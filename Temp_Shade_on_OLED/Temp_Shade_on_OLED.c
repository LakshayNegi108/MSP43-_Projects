/**
 * main.c
 *
 * Example usage of SSD1306 library created for MSP-EXP430G2 TI LaunchPad
 * Complied using TI v18.12.2.LTS
 *
 * SSD1306 library based on Adafuit's wonderful Arduino library with modifications
 *  and less features to fit the small amount of RAM available on the MSP430G2553 *
 *
 */

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "SSD1306.h"
#include "i2c.h"
#include "Analog_read.h"

#define DELAY_3sec       __delay_cycles(3000000)
#define DELAY_2sec       __delay_cycles(2000000)
#define DELAY_1sec       __delay_cycles(1000000)
#define DELAY_Halfsec    __delay_cycles(50000)

#define LED_1       BIT0
#define LED_2       BIT1
#define LED_3       BIT2
#define SW_UP       BIT3                   // Switch -> P1.4
#define SW_DOWN     BIT4
#define SW_SEL      BIT5

uint8_t option = 1;
uint8_t up_sel = 0;
uint8_t down_sel = 0;

void SetClockat_1Mhz()
{
    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;
}

void print_tapman_chaya()
{
    uint8_t upper_pos = 20;
    uint8_t lower_pos = 20;
    ssd1306_printText_2_wide(upper_pos, 0, "T");
    upper_pos = upper_pos + 8;
    ssd1306_printText_matra(upper_pos, 0, "A");
    upper_pos = upper_pos + 4;
    ssd1306_printText_2_wide(upper_pos, 0, "P");
    upper_pos = upper_pos + 8;
    ssd1306_printText_2_wide(upper_pos, 0, "M");
    upper_pos = upper_pos + 8;
    ssd1306_printText_matra(upper_pos, 0, "A");
    upper_pos = upper_pos + 4;
    ssd1306_printText_2_wide(upper_pos, 0, "N");
    upper_pos = upper_pos + 30;
    ssd1306_printText_2_wide(upper_pos, 0, "C");
    upper_pos = upper_pos + 8;
    ssd1306_printText_matra(upper_pos, 0, "A");
    upper_pos = upper_pos + 4;
    ssd1306_printText_2_wide(upper_pos, 0, "Y");
    upper_pos = upper_pos + 8;
    ssd1306_printText_matra(upper_pos, 0, "A");

    ssd1306_printText_2_wide(lower_pos, 1, "t");
    lower_pos = lower_pos + 8;
    ssd1306_printText_matra(lower_pos, 1, "a");
    lower_pos = lower_pos + 4;
    ssd1306_printText_2_wide(lower_pos, 1, "p");
    lower_pos = lower_pos + 8;
    ssd1306_printText_2_wide(lower_pos, 1, "m");
    lower_pos = lower_pos + 8;
    ssd1306_printText_matra(lower_pos, 1, "a");
    lower_pos = lower_pos + 4;
    ssd1306_printText_2_wide(lower_pos, 1, "n");
    lower_pos = lower_pos + 30;
    ssd1306_printText_2_wide(lower_pos, 1, "c");
    lower_pos = lower_pos + 8;
    ssd1306_printText_matra(lower_pos, 1, "a");
    lower_pos = lower_pos + 4;
    ssd1306_printText_2_wide(lower_pos, 1, "y");
    lower_pos = lower_pos + 8;
    ssd1306_printText_matra(lower_pos, 1, "a");
}

void print_delay_long()
{
    //   invertDisplay(1);
    print_tapman_chaya();
    start_sampling();
//        ssd1306_clearDisplay();
    //       ssd1306_printUI32(30, 2, adc[2], HCENTERUL_OFF);
    float temp = calc_temp();
    int int_part = temp;        // Integer part of calculated ADC value
    int decimal_part = (temp - (float) int_part) * 10.0; // Decimal part of calculated ADC value

    int shade = calc_shade();

    ssd1306_printUI32(26, 2, int_part, HCENTERUL_OFF);
    ssd1306_printText(37, 2, ".");
    ssd1306_printUI32(43, 2, decimal_part, HCENTERUL_OFF);
    ssd1306_printText(49, 2, "`C");
    ssd1306_printUI32(85, 2, shade, HCENTERUL_OFF);
    ssd1306_printText(0, 2, "   ");
    ssd1306_printText(61, 2, "    ");
    ssd1306_printText(97, 2, "    ");
 //    WDTCTL = WDT_ADLY_1000;
    //     ssd1306_printText(0, 3, "<><><><><><><><><><>");

    //    startscrollright(3, 3);
    //     ssd1306_printText(0, 6, "<><><><><><><><><><>");
    //    startscrollright(3, 6);
}

void print_delay_short()
{
    ssd1306_printText(20, 4, "Select the LED");
    ssd1306_printText(62, 5, "<");
    ssd1306_printUI32(68, 5, option, HCENTERUL_OFF);
    ssd1306_printText(74, 5, ">");
    if (up_sel == 1)
    {
        ssd1306_printText(56, 5, "<");
        up_sel = 0;
    }
    else if (down_sel == 1)
    {
        ssd1306_printText(80, 5, ">");
        down_sel = 0;
    }
    else
    {
        ssd1306_printText(56, 5, " ");
        ssd1306_printText(80, 5, " ");
    }
   //    WDTCTL = WDT_ADLY_1000;
}
void sel_led(uint8_t option)
{
    switch (option)
    {
    case 0:
        P2OUT ^= BIT0;
        break;
    case 1:
        P2OUT ^= BIT1;
        break;
    case 2:
        P2OUT ^= BIT2;
        break;
    }
}
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // stop watchdog timer

    P2DIR |= LED_1 | LED_2 | LED_3;
    P2OUT &= ~(LED_1 | LED_2 | LED_3);
    P2DIR &= ~(SW_UP + SW_DOWN + SW_SEL);                 // Set SW pin -> Input
    P2REN |= (SW_UP + SW_DOWN + SW_SEL);           // Enable Resistor for SW pin
    P2OUT |= (SW_UP + SW_DOWN + SW_SEL);            // Select Pull Up for SW pin

    P2IES &= ~(SW_UP + SW_DOWN + SW_SEL);     // Select Interrupt on Rising Edge
    P2IE |= (SW_UP + SW_DOWN + SW_SEL);            // Enable Interrupt on SW pin

    __bis_SR_register(GIE);             // Enable CPU Interrupt

    register_settings_for_ADC10();
    i2c_init();                               // initialize I2C to use with OLED
    ssd1306_init();                             // Initialize SSD1306 OLED
    ssd1306_clearDisplay();                     // Clear OLED display

    while (1)
    {
        //  WDTCTL = WDTPW | WDTHOLD;
        WDTCTL = WDT_ADLY_1000;

        print_delay_long();
        print_delay_short();

        WDTCTL = WDT_ADLY_1000;
        WDTCTL = WDTPW | WDTHOLD;
        DELAY_Halfsec;
        WDTCTL = WDT_ADLY_1000;
    }

}

#pragma vector=PORT2_VECTOR
__interrupt void Port_1(void)
{
    if (P2IFG & SW_UP)                      // If SW is Pressed
    {
        __delay_cycles(20000);
        if (option == 0)
        {
            option = 2;
        }
        else
            option = option - 1;
        up_sel = 1;
        P2IFG &= ~SW_UP;                   // Clear SW interrupt flag
    }
    if (P2IFG & SW_DOWN)                      // If SW is Pressed
    {
        __delay_cycles(20000);
        if (option == 2)
        {
            option = 0;
        }
        else
            option = option + 1;
        down_sel = 1;
        P2IFG &= ~SW_DOWN;                   // Clear SW interrupt flag
    }
    if (P2IFG & SW_SEL)                      // If SW is Pressed
    {
        __delay_cycles(20000);
        sel_led(option);
        P2IFG &= ~SW_SEL;
    }
}
