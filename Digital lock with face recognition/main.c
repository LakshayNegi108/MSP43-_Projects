#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define B1 (P2IN & input1)
#define B2 (P2IN & input2)
#define B3 (P2IN & input3)
#define B4 (P2IN & input4)

#define input1 BIT0
#define input2 BIT1
#define input3 BIT2
#define input4 BIT3

#define wakeup BIT4
#define enter BIT5

#define red_led BIT6
#define green_led BIT5


int control = 0;
int default_password[4]={0,1,0,0};
int your_password[4];
static int enter_your_password=0;
volatile unsigned long i;

static int face_flag = 0;

void register_settings_for_UART()
{
    P1SEL = BIT1 + BIT2;              // Select UART RX/TX function on P1.1,P1.2
    P1SEL2 = BIT1 + BIT2;

    UCA0CTL1 |= UCSSEL_1;               // UART Clock -> ACLK
    UCA0BR0 = 3;                        // Baud Rate Setting for 32kHz 9600
    UCA0BR1 = 0;                        // Baud Rate Setting for 32kHz 9600
    UCA0MCTL = UCBRF_0 + UCBRS_3;       // Modulation Setting for 32kHz 9600
    UCA0CTL1 &= ~UCSWRST;               // Initialize UART Module
    IE2 |= UCA0RXIE;                    // Enable RX interrupt
}


void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;       //! Stop Watchdog

    P2DIR &= ~(wakeup|enter);       //SW as input
    P2REN |= wakeup|enter;          //Internal resistor enable
    P2OUT |= (wakeup|enter);        //on Pull-up mode

    P2DIR &= ~(0x0F);           //dip SW input
    P2REN |= 0x0F;              //Internal resistor enable
    P2OUT &= ~(0x0F);           //Pulldown mode for dip


    P2IES &= ~(wakeup|enter);         //Interrupt edge select
    P2IE |= wakeup|enter;            //Enable interrupts for SWs


    P1DIR |= BIT7|BIT6|BIT5;
    P1OUT |= 0xE0;              //Red LED -> ON
    for(i = 0; i<10000; i++);   //delay
    P1OUT &= ~0xE0;             //Red LED -> OFF
    for(i = 0; i<5000; i++);

    register_settings_for_UART();
    while(1){
        __bis_SR_register(LPM0_bits +GIE);      //CPU is disabled,Supply current(at 1 MHz) : 56uA , Interrupt enable
    }
}

#pragma vector=USCIAB0RX_VECTOR         // UART RX Interrupt Vector
__interrupt void USCI0RX_ISR(void)
{
    P1OUT ^= BIT7;
    __delay_cycles(30000);
    face_flag = 1;
    IFG2 &= ~UCA0RXIFG;                  //clear interrupt bit
}

#pragma vector = PORT2_VECTOR
__interrupt void Port2(void){
if(face_flag == 1){
if(!(P2IN & wakeup)){

       __delay_cycles(20000);
       while(!(P2IN & wakeup));
        __delay_cycles(20000);

        P1OUT ^= red_led;
        enter_your_password = 1;

        __bic_SR_register_on_exit(LPM0_bits + GIE);
        P2IFG &= ~wakeup;
      }
if(enter_your_password==1){
    if(B1){
        your_password[0]=1;
    }
    else
        your_password[0]=0;
    if(B2){
            your_password[1]=1;
        }
    else
            your_password[1]=0;
    if(B3){
            your_password[2]=1;
        }
    else
            your_password[2]=0;
    if(B4){
            your_password[3]=1;
        }
     else
            your_password[3]=0;
        if(!(P2IN & enter)){
                    __delay_cycles(2000);
                 while(!(P2IN & enter));
                      __delay_cycles(2000);

                 if(your_password[0] == default_password[0] && your_password[1] == default_password[1] && your_password[2] == default_password[2] && your_password[3] == default_password[3]){
                      enter_your_password=0;
                      P1OUT |= green_led;
                 }
                 else{
                    P1OUT &= ~red_led;
                    __delay_cycles(510000);
                    P1OUT |= red_led;
                    __delay_cycles(510000);
                    P1OUT &= ~red_led;
                    __delay_cycles(510000);
                    P1OUT |= red_led;
                   }
       __bic_SR_register_on_exit(LPM0_bits + GIE);
       P2IFG &= ~enter;
        }
    }
  }
}
