#include <msp430.h>

#define LED1   BIT6                        // Green LED -> P1.6
#define LED2   BIT2
#define Check BIT1
/**
 * @brief
 * These settings are w.r.t enabling TIMER0 on Lunchbox
 **/
void register_settings_for_TIMER0()
{
    P1DIR |= LED1;                         // Green LED -> Output
    P1SEL |= LED1;                         // Green LED -> Select Timer Output

    CCR0 = 800;                             // Set Timer0 PWM Period
    CCTL1 = OUTMOD_7;                       // Set TA0.1 Waveform Mode - Clear on Compare, Set on Overflow
    CCR1 = 5;                               // Set TA0.1 PWM duty cycle
    CCTL0 = CCIE;                           // CCR0 Enable Interrupt
    TACTL = TASSEL_0 + MC_1;                // Timer Clock -> SMCLK, Mode -> Up Count
}

void register_settings_for_TIMER1()
{
    P2DIR |= LED2;                         // Green LED -> Output
    P2SEL |= LED2;                         // Green LED -> Select Timer Output

    TA1CCR0 = 255;                             // Set Timer0 PWM Period
    TA1CCTL1 = OUTMOD_7;                       // Set TA0.1 Waveform Mode - Clear on Compare, Set on Overflow
    TA1CCR1 = 200;                               // Set TA0.1 PWM duty cycle
    TA1CCTL0 = CCIE;                           // CCR0 Enable Interrupt
    TA1CTL = TASSEL_0 + MC_1;                // Timer Clock -> SMCLK, Mode -> Up Count
}
/**
 * @brief
 * Entry point for the code
 **/
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    P1DIR |= Check;
    P1OUT &= ~Check;

    register_settings_for_TIMER0();
    register_settings_for_TIMER1();

    __bis_SR_register(GIE);                 // Enable CPU Interrupt

    while(1)
    {
    }

}

/**
 * @brief
 * Entry point for TIMER0_interrupt vector
 **/

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
//    P1OUT ^= Check;
//    __delay_cycles(110000);
    CCR1 = CCR1 + 1;                        // Increment CCR1
    if(CCR1 == 256)
    {
        CCR1 = 0;
    }


}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A1(void)
{
//    P1OUT ^= Check;
//    __delay_cycles(110000);
    TA1CCR1 = TA1CCR1 + 1;                        // Increment CCR1
    if(TA1CCR1 == 256)
    {
        TA1CCR1 = 0;
    }

}
