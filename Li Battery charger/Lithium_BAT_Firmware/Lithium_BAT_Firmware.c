#include <msp430.h>
#include <inttypes.h>

#define CMD         0
#define DATA        1

#define LCD_P1OUT     P1OUT
#define LCD_P1DIR     P1DIR
#define LCD_P2OUT     P2OUT
#define LCD_P2DIR     P2DIR
#define D4          BIT4
#define D5          BIT5
#define D6          BIT6
#define D7          BIT7
#define RS          BIT1
#define EN          BIT0

#define TIMER_OUTPUT BIT2

unsigned int adc[8];                 //Store ADC values

unsigned int const_volt = 0;        // Determine ConstVoltage/ConstCurrent stage
unsigned int timedelay = 0;            // Variable to Start Watchdog timer delay
unsigned int timercounts = 0;               // Track time delay
unsigned int delayflag = 0;

/**
 *@brief Delay function for producing delay in 0.1 ms increments
 *@param t milliseconds to be delayed
 *@return void
 **/
void delay(uint16_t t)
{
    uint16_t i;
    for (i = t; i > 0; i--)
        __delay_cycles(1000);
}

/**
 *@brief Function to pulse EN pin after data is written
 *@return void
 **/
void pulseEN(void)
{
    LCD_P2OUT |= EN;                          // Giving a falling edge at EN pin
    delay(1);
    LCD_P2OUT &= ~EN;
    delay(1);
}

/**
 *@brief Function to write data/command to LCD
 *@param value Value to be written to LED
 *@param mode Mode -> Command or Data
 *@return void
 **/
void lcd_write(uint8_t value, uint8_t mode)
{
    if (mode == CMD)
        LCD_P2OUT &= ~RS;             // Set RS -> LOW for Command mode
    else
        LCD_P2OUT |= RS;              // Set RS -> HIGH for Data mode

    uint8_t high_nibble = (value & 0xF0);
    uint8_t low_nibble = (value << 4) & 0xF0;

    LCD_P1OUT = ((LCD_P1OUT & 0x0F) | high_nibble);   // Write high nibble first
    pulseEN();
    delay(1);

    LCD_P1OUT = ((LCD_P1OUT & 0x0F) | low_nibble);      // Write low nibble next
    pulseEN();
    delay(1);
}

/**
 *@brief Function to print a string on LCD
 *@param *s pointer to the character to be written.
 *@return void
 **/
void lcd_print(char *s)
{
    while (*s)
    {
        lcd_write(*s, DATA);
        s++;
    }
}

/**
 *@brief Function to move cursor to desired position on LCD
 *@param row Row Cursor of the LCD
 *@param col Column Cursor of the LCD
 *@return void
 **/
void lcd_setCursor(uint8_t row, uint8_t col)
{
    const uint8_t row_offsets[] = { 0x00, 0x40 };
    lcd_write(0x80 | (col + row_offsets[row]), CMD);
    delay(1);
}

void lcd_printNumber(unsigned int num)
{
    char buf[4];                        // Creating a array of size 3
    char *str = &buf[3];             // Initializing pointer to end of the array

    *str = '\0';                        // storing null pointer at end of string

    do
    {
        unsigned long m = num;          // Storing number in variable m
        num /= 10;                      // Dividing number by 10
        char c = (m - 10 * num) + '0'; // Finding least place value and adding it to get character value of digit
        *--str = c; // Decrementing pointer value and storing character at that character
    }
    while (num);

    lcd_print(str);
}

/**
 *@brief Initialize LCD
 **/
void lcd_init()
{
    LCD_P1DIR |= (D4 + D5 + D6 + D7);
    LCD_P1OUT &= ~(D4 + D5 + D6 + D7);
    LCD_P2DIR |= (RS + EN);
    LCD_P2OUT &= ~(RS + EN);

    delay(150);                     // Wait for power up ( 15ms )
    lcd_write(0x33, CMD);           // Initialization Sequence 1
    delay(50);                      // Wait ( 4.1 ms )
    lcd_write(0x32, CMD);           // Initialization Sequence 2
    delay(1);                       // Wait ( 100 us )

    // All subsequent commands take 40 us to execute, except clear & cursor return (1.64 ms)

    lcd_write(0x28, CMD);           // 4 bit mode, 2 line
    delay(1);

    lcd_write(0x0C, CMD);           // Display ON, Cursor OFF, Blink OFF
    delay(1);

    lcd_write(0x01, CMD);           // Clear screen
    delay(20);

    lcd_write(0x06, CMD);           // Auto Increment Cursor
    delay(1);

    lcd_setCursor(0, 0);             // Goto Row 1 Column 1
}

void register_settings_for_TIMER1()
{
    P2DIR |= TIMER_OUTPUT;
    P2SEL |= TIMER_OUTPUT;

    TA1CCR0 = 255;
    TA1CCTL1 = OUTMOD_7;
    TA1CCR1 = 80;
    TA1CCTL0 = CCIE;
    TA1CTL = TASSEL_2 + MC_1;
}

unsigned int i;
void register_settings_for_DCO()
{
    // BCSCTL3 |= LFXT1S_2;                 // LFXT1 = VLO
    do
    {
        IFG1 &= ~OFIFG;                  // Clear oscillator fault flag
        for (i = 50000; i; i--)
            ;         // Delay
    }
    while (IFG1 & OFIFG);            // Test osc fault flag

    BCSCTL2 |= SELM_1 + DIVS_2;           // MCLK = DCO && SMCLK = DCO + SMCLK/4
    DCOCTL |= BIT6 | BIT5;                 //DCO = 3
    BCSCTL1 |= BIT3 | BIT1;           //Range select = 10 Freq = 3.4MHz
}

void register_settings_for_ADC10()
{
    ADC10CTL0 &= ~ENC;
    ADC10AE0 |= BIT3 + BIT2 + BIT1 + BIT0; // P1.0,1.1,1.2,1.3 ADC option select
    ADC10CTL1 = INCH_4 + CONSEQ_3 + SHS_1;     // ADC Channel -> 4,A4
                                               //Conversion Seq. -> Repeat-sequence-of-channels
                                               //Sample-and-hold source -> Timer_A.OUT1
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + ADC10ON + REFON + ADC10IE + MSC; // Ref -> Vcc, 64 CLK S&H , ADC - ON
    ADC10DTC1 = 4;

    while (ADC10CTL1 & ADC10BUSY)
        ;           // Wait for conversion to end
    ADC10DTC0 |= ADC10CT;
    ADC10SA = (unsigned int) adc;
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

}

void LCD_VIT_PRINT(char c)
{
    //**************************************************************//
    //*                                                            *//
    //*  adc[1] store value from ADC P1.3 for showing voltage      *//
    //*  adc[2] store value from ADC P1.2 for showing current      *//
    //*  adc[3] store value from ADC P1.1 for showing temperature  *//
    //*                                                            *//
    //**************************************************************//
    float adc_value = 0;
    int int_part, decimal_part = 0;

    switch (c)
    {
    case 'V':
    {
        adc_value = (adc[1]) * (3.30) / (1023.00); // mapping 10-bit conversion result of ADC to corresponding voltage

        int_part = adc_value;        // Integer part of calculated ADC value
        decimal_part = (adc_value - (float) int_part) * 1000.0; // Decimal part of calculated ADC value

        lcd_setCursor(0, 1);
        lcd_print("Volt");

        lcd_setCursor(1, 0);
        lcd_printNumber(int_part);       // Displaying integer part of ADC value
        lcd_print(".");
        lcd_printNumber(decimal_part);   // Displaying decimal part of ADC value
    }
    case 'I':
    {
        adc_value = 2 * ((adc[2]) * (3.30) / (1023.00)); // mapping 10-bit conversion result of ADC to corresponding voltage

        int_part = adc_value;        // Integer part of calculated ADC value
        decimal_part = (adc_value - (float) int_part) * 1000.0; // Decimal part of calculated ADC value

        lcd_setCursor(0, 7);
        lcd_print("CURR");

        lcd_setCursor(1, 6);
        lcd_printNumber(int_part);    // Displaying integer part of ADC value
        lcd_print(".");
        lcd_printNumber(decimal_part); // Displaying decimal part of ADC value
    }
    case 'T':
    {
        lcd_setCursor(0, 12);
        lcd_print("Temp");

        lcd_setCursor(1, 12);
        if (adc[3] > 450 && adc[3] < 700)
        {
            lcd_print("NORM");
        }
        else if (adc[3] > 700)
        {
            lcd_print("HIGH");
        }
        else
        {
            lcd_print("--");
        }
    }
    }
}

/*@brief entry point for the code*/
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //! Stop Watchdog (Not recommended for code in production and devices working in field)

    register_settings_for_DCO();
    lcd_init();                                     // Initializing LCD
    register_settings_for_ADC10();
    register_settings_for_TIMER1();
    __bis_SR_register(GIE);
    while (1)
    {
        if(timedelay == 1)
        {
          delayflag = 1;
          WDTCTL = WDT_ADLY_1000;                   // WDT 1s, ACLK, interval timer
          IE1 |= WDTIE;                             // Enable WDT interrupt
          _BIS_SR(GIE);                       // Enter LPM3 w/interrupt
        }
        unsigned int a = 0;
        a = a + 1;
        if (a == 34000000)
        {
            lcd_write(0x01, CMD);                   // Clear screen
            delay(20);
            LCD_VIT_PRINT('V');
            LCD_VIT_PRINT('I');
            LCD_VIT_PRINT('T');
            a = 0;
        }
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void adc10_ISR(void)
{
    if (adc[3] > 700)                     // Batt_Temperature too high?
    {
        TACTL = 0;
        P1SEL = 0;
     //   P1OUT = 0x01;
        ADC10CTL0 = 0;
        timercounts = 0;
        timedelay = 0;
        delayflag = 0;
        WDTCTL = WDTPW + WDTHOLD;              // Stop WDT
    }
    if (const_volt == 0)
    {
        if (adc[1] < 227 | adc[1] > 975)   // Vout < 1V or >4.3V, Alarm
        //  P1OUT |= 0x02;
            lcd_setCursor(0, 0);
        lcd_print("Battery dead or");
        lcd_setCursor(1, 0);
        lcd_print("wrong polarity");
    }
    if ((adc[1] < 818) && const_volt == 0)       // Vout < 3.6V, slow Charge
    {
        if (adc[2] > 300)              // adc[2] measures Rsense voltage
            TA1CCR1--;                                 // Adjust PWM
        else
        {
            TA1CCR1++;
            TA1CCR1 &= 0xFF;                           // 8-bit resolution
        }
    }
    else if ((adc[1] > 932) | const_volt == 1) // Vout >4.1V, Const Voltage
    {
        const_volt = 1;
        TA1CCR1--;
    }
    if (const_volt == 1)
    {
        if ((adc[2] < 1) && (delayflag == 0)) // Current < 0.1C, Stop Charging after
        {                                       // 30 minute delay
            timedelay = 1;
            //  LPM0_EXIT;
        }
        if (adc[1] < 954)                  // 4.1 < Vout < 4.2
            TA1CCR1++;
    }
    else if ((adc[1] >= 818) && const_volt == 0) // Vout > 3.6 V, Fast Charge
    {
        if (adc[2] > 500)
            TA1CCR1--;
        else
        {
            TA1CCR1++;
            TA1CCR1 &= 0xFF;
        }
    }
       _NOP();
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A1(void)
{

    // TA1CCR1 = TA1CCR1;

}

// Watchdog Timer interrupt service routine
// Enter every 8 secs to create 15 min delay before Stop
// charging
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    //  P1OUT ^= 0x01;
    if (timercounts == 110)
    {
        TACTL = 0;                             // before stop charging
        P1SEL = 0;
        P1OUT = 0;
        ADC10CTL0 = 0;
        timercounts = 0;
        timedelay = 0;
        delayflag = 0;
        WDTCTL = WDTPW + WDTHOLD;              // Stop WDT
    }
    timercounts++;
}

