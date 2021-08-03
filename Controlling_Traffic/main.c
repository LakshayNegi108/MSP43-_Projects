//Single Lane Traffic light control

#include <msp430.h>

#define SW_east  BIT3
#define SW_west  BIT4

#define red_east BIT0
#define yellow_east BIT1
#define green_east BIT2

#define red_west BIT3
#define yellow_west BIT4
#define green_west BIT5


void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= red_east|yellow_east|green_east|red_west|yellow_west|green_west;
    P2DIR &= ~SW_east|SW_west;
    volatile int traffic_west = 1;
   // const unsigned int flowtime =10000000;
   // const unsigned int changedelay = 1100000;
    unsigned int i;

    P1OUT |= red_east|green_west;
    P1OUT &= ~(red_west|yellow_west|yellow_east|green_east);

    while(1)
    {

        if(P2IN & SW_east){

                    if(traffic_west == 0){
                        traffic_west = 1;
                        __delay_cycles(10000000);
                        P1OUT &= ~green_east;
                        P1OUT |= yellow_east;
                        __delay_cycles(4100000);
                        P1OUT &= ~yellow_east;
                        P1OUT |= red_east;
                        __delay_cycles(4100000);
                        for(i=5;i>0;i--){
                            P1OUT |= yellow_west;
                            __delay_cycles(500000);
                            P1OUT &= ~yellow_west;
                            __delay_cycles(500000);
                        }
                        P1OUT &= ~(red_west|yellow_west);
                        P1OUT |= green_west;
                    }
                }
        if(P2IN & SW_west){

                  if(traffic_west == 1){
                      traffic_west = 0;
                      __delay_cycles(10000000);
                      P1OUT &= ~green_west;
                      P1OUT |= yellow_west;
                      __delay_cycles(4100000);
                      P1OUT &= ~yellow_west;
                      P1OUT |= red_west;
                      __delay_cycles(4100000);
                      for(i=5;i>0;i--){
                          P1OUT |= yellow_east;
                          __delay_cycles(500000);
                          P1OUT &= ~yellow_east;
                          __delay_cycles(500000);
                      }
                      P1OUT &= ~(red_east|yellow_east);
                      P1OUT |= green_east;
                  }
              }
    }
}
