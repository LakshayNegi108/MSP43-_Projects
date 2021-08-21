#include <msp430.h>
#include <inttypes.h>

#define CMD         0
#define DATA        1

#define LCD_OUT     P1OUT
#define LCD_DIR     P1DIR
#define D4          BIT4
#define D5          BIT5
#define D6          BIT6
#define D7          BIT7
#define RS          BIT2
#define EN          BIT3

#define B1 (P2IN & input1)
#define B2 (P2IN & input2)
#define B3 (P2IN & input3)
#define B4 (P2IN & input4)

#define red_led BIT0
#define green_led BIT1

#define input1 BIT0
#define input2 BIT1
#define input3 BIT2
#define input4 BIT3

#define output_lock BIT4

#define wakeup BIT5
#define reset BIT6
#define enter BIT7

void delay(uint16_t t)
{
    uint16_t i;
    for(i=t; i > 0; i--)
        __delay_cycles(100);
}


void pulseEN(void)
{
    LCD_OUT |= EN;
    delay(1);
    LCD_OUT &= ~EN;
    delay(1);
}


void lcd_write(uint8_t value, uint8_t mode)
{
    if(mode == CMD)
        LCD_OUT &= ~RS;
    else
        LCD_OUT |= RS;

    LCD_OUT = ((LCD_OUT & 0x0F) | (value & 0xF0));              // Write high nibble first
    pulseEN();
    delay(1);

    LCD_OUT = ((LCD_OUT & 0x0F) | ((value << 4) & 0xF0));       // Write low nibble next
    pulseEN();
    delay(1);
}

void lcd_print(char *s)
{
    while(*s)
    {
        lcd_write(*s, DATA);
        s++;
    }
}

void lcd_setCursor(uint8_t row, uint8_t col)
{
    const uint8_t row_offsets[] = { 0x00, 0x40};
    lcd_write(0x80 | (col + row_offsets[row]), CMD);
    delay(1);
}


void lcd_init()
{
    LCD_DIR |= (D4+D5+D6+D7+RS+EN);
    LCD_OUT &= ~(D4+D5+D6+D7+RS+EN);

    delay(150);                     // Wait for power up ( 15ms )
    lcd_write(0x33, CMD);           // Initialization Sequence 1
    delay(50);                      // Wait ( 4.1 ms )
    lcd_write(0x32, CMD);           // Initialization Sequence 2
    delay(1);                       // Wait ( 100 us )



    lcd_write(0x28, CMD);           // 4 bit mode, 2 line
    delay(1);

    lcd_write(0x0C, CMD);           // Display ON, Cursor OFF, Blink OFF
    delay(1);

    lcd_write(0x01, CMD);           // Clear screen
    delay(20);

    lcd_write(0x06, CMD);           // Auto Increment Cursor
    delay(1);

    lcd_setCursor(0,0);             // Goto Row 1 Column 1
}

lcd_clear(){
        lcd_setCursor(0,1);
        lcd_print("                ");    // Print

        lcd_setCursor(1,1);
        lcd_print("                ");
}
lcd_password(){
           lcd_setCursor(0,1);
           lcd_print("Enter your");    // Print

           lcd_setCursor(1,1);
           lcd_print("Password");
}


lcd_reset(){
           lcd_setCursor(0,1);
           lcd_print("Enter new");    // Print

           lcd_setCursor(1,1);
           lcd_print("Password");
}

lcd_default(){
    lcd_setCursor(0,1);
    lcd_print("Password notset");    // Print

    lcd_setCursor(1,1);
    lcd_print("Default:0000");
}

lcd_correct(){
    lcd_setCursor(0,4);
    lcd_print("Password");    // Print

    lcd_setCursor(1,4);
    lcd_print("Accepted");
}
lcd_incorrect(){
    lcd_setCursor(0,1);
    lcd_print("Password");    // Print

    lcd_setCursor(1,2);
    lcd_print("is incorrect");
}

check_password(){

}

int control = 0;
int reset_control = 0;
int default_password[4]={0,0,0,0};
int your_password[4];
int new_password[4];
static int enter_your_password=0;
static int enter_new_password=0;

unsigned int flag = 0;

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;       //! Stop Watchdog (Not recommended for code in production and devices working in field)



    lcd_init();                     // Initialising LCD

    P1DIR |= red_led|green_led;
    P1OUT ^= red_led|green_led;
    __delay_cycles(22000);
    P1OUT ^= red_led|green_led;

    P2DIR &= ~0xEF;  //11101111 as input
    P2DIR |= output_lock;
    P2REN |= 0xEF;   //enable internal resister
    P2OUT &= ~0x0F;   // pull down mode for 00001111
    P2OUT |= wakeup|reset|enter;      //pullup mode, receving 1 always

    P2IES &= ~(wakeup|reset|enter);
    P2IE |= wakeup|reset|enter;

    volatile unsigned int reset_flag = 0;


    while(1){
        __bis_SR_register(LPM0_bits +GIE);
    }
}

#pragma vector = PORT2_VECTOR
__interrupt void Port2(void){

if(control == 0){
    if(!(P2IN & wakeup)){

      //  __delay_cycles(2000);
        while(!(P2IN & wakeup));
     //   __delay_cycles(2000);

        lcd_clear();
        lcd_default();
            enter_your_password = 1;
            control = 1;

        __bic_SR_register_on_exit(LPM0_bits + GIE);
        P2IFG &= ~wakeup;
    }
 }

if(control == 1){
    if(!(P2IN & wakeup)){

      //  __delay_cycles(2000);
        while(!(P2IN & wakeup));
     //   __delay_cycles(2000);
        reset_control = 0;

        lcd_clear();
        lcd_password();

            enter_your_password = 1;

        __bic_SR_register_on_exit(LPM0_bits + GIE);
        P2IFG &= ~wakeup;
    }
 }

if(enter_your_password==1){
    your_password[0]=B1;
    your_password[1]=B2;
    your_password[2]=B3;
    your_password[3]=B4;
    if(!(P2IN & enter)){
              //  __delay_cycles(2000);
             while(!(P2IN & enter));
              //    __delay_cycles(2000);

              if(your_password[0] == default_password[0] && your_password[1] == default_password[1] && your_password[2] == default_password[2] && your_password[3] == default_password[3]){
                  lcd_clear();
                  lcd_correct();
                  enter_your_password=0;
                  reset_control = 2;
                  flag = 1;
             }
             else{
                lcd_clear();
                lcd_incorrect();
                enter_your_password=0;
                if(flag == 0){
                control = 0;
                }
               }
    __bic_SR_register_on_exit(LPM0_bits + GIE);
    P2IFG &= ~enter;
    }
}
if(reset_control == 2){
       if(!(P2IN & reset)){
           // __delay_cycles(2000);
           while(!(P2IN & reset));
           //  __delay_cycles(2000);

           lcd_clear();
           lcd_reset();

           enter_new_password=1;
           __bic_SR_register_on_exit(LPM0_bits + GIE);
           P2IFG &= ~reset;
       }
}
if(enter_new_password==1){
    new_password[0]=B1;
    new_password[1]=B2;
    new_password[2]=B3;
    new_password[3]=B4;
    if(!(P2IN & enter)){
              //  __delay_cycles(2000);
             while(!(P2IN & enter));
              //    __delay_cycles(2000);

             default_password[0]=new_password[0];
             default_password[1]=new_password[1];
             default_password[2]=new_password[2];
             default_password[3]=new_password[3];

             lcd_clear();
             lcd_correct();
             reset_control=0;

             enter_new_password=0;
    __bic_SR_register_on_exit(LPM0_bits + GIE);
    P2IFG &= ~enter;
    }
  }
}

