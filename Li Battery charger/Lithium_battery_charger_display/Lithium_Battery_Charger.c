#include <msp430.h>
#include <inttypes.h>

#define CMD         0
#define DATA        1

#define AIN BIT3

#define LCD_P1OUT     P1OUT
#define LCD_P1DIR     P1DIR
#define LCD_P2OUT     P2OUT
#define LCD_P2DIR     P2DIR
#define D4          BIT4
#define D5          BIT5
#define D6          BIT6
#define D7          BIT7
#define RS          BIT0
#define EN          BIT1

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

    BCSCTL2 |= SELM_1 + DIVS_2;                   // MCLK = DCO && SMCLK = DCO + SMCLK/4
    DCOCTL |= BIT6 | BIT5;                 //DCO = 3
    BCSCTL1 |= BIT3|BIT1;           //Range select =
}
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

void register_settings_for_ADC10()
{
    ADC10AE0 |= BIT3 + BIT2 + BIT1 + BIT0;             // P1.0 ADC option select
    ADC10CTL1 = INCH_4 + ADC10DIV_0 + CONSEQ_3 + SHS_0; // ADC Channel -> 1 (P1.0)
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + MSC; // Ref -> Vcc, 64 CLK S&H , ADC - ON
    ADC10DTC1 = 4;
}
//*************************************************************
  //ADC pin 3 shows voltage
  //ADC pin 2 shows current
  //ADC pin 1 show temperature
//*************************************************************
unsigned int adc[8];
float logR, R, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float R_t = 10400.0; //Known Resistance of Thermistor

/*@brief entry point for the code*/
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //! Stop Watchdog (Not recommended for code in production and devices working in field)

    lcd_init();                                     // Initializing LCD
    register_settings_for_DCO();
    register_settings_for_ADC10();

    while (1)
    {
        ADC10CTL0 &= ~ENC;

        while (ADC10CTL1 & ADC10BUSY)
            ;           // Wait for conversion to end
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

        ADC10SA = (unsigned int) adc;

        float adc_value = 0;
        float adc_value_2 = 0;
        float adc_value_1 = 0;

        adc_value = (adc[1]) * (3.30) / (1023.00); // mapping 10-bit conversion result of ADC to corresponding voltage

        int int_part = adc_value;        // Integer part of calculated ADC value
        int decimal_part = (adc_value - (float) int_part) * 1000.0; // Decimal part of calculated ADC value

        adc_value_2 = ((adc[2]) * (3.30) / (1023.00))/47; // mapping 10-bit conversion result of ADC to corresponding voltage

        int int_part_2 = adc_value_2;    // Integer part of calculated ADC value
        int decimal_part_2 = (adc_value_2 - (float) int_part_2) * 1000.0; // Decimal part of calculated ADC value

        adc_value_1 = (adc[3]) * (3.30) / (1023.00); // mapping 10-bit conversion result of ADC to corresponding voltage

        R = R_t * (1023.0 / (float) adc_value_1 - 1.0); //Using Voltage divider for finding the Resistor Value
        logR = log(R);
        T = (1.0 / (c1 + c2 * logR + c3 * logR * logR * logR)); //Steinhart-Hart Equation
        T = T - 273.15;

        int int_temp = T;        // Integer part of calculated ADC value
        int decimal_temp = (T - (float) int_temp) * 10.0; // Decimal part of calculated ADC value

        lcd_write(0x01, CMD);                   // Clear screen
        delay(20);

        lcd_setCursor(0, 1);
        lcd_print("Volt");

        lcd_setCursor(1, 0);
        lcd_printNumber(int_part);       // Displaying integer part of ADC value
        lcd_print(".");
        lcd_printNumber(decimal_part);   // Displaying decimal part of ADC value
        //    lcd_setCursor(0, 12);
        //    lcd_print("V");

        lcd_setCursor(0, 7);
        lcd_print("CURR");

        lcd_setCursor(1, 6);
        lcd_printNumber(int_part_2);     // Displaying integer part of ADC value
        lcd_print(".");
        lcd_printNumber(decimal_part_2); // Displaying decimal part of ADC value

        lcd_setCursor(0, 12);
        lcd_print("Temp");

        lcd_setCursor(1, 12);
        lcd_printNumber (int_temp);    // Displaying integer part of ADC value
        lcd_print(".");
        lcd_printNumber (10); // Displaying decimal part of ADC value

        delay(6000);
        __delay_cycles(8000000);
    }
}
