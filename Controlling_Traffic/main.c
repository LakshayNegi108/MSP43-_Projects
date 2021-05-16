#include <msp430.h> 


#define East_Button BIT0
#define West_Button BIT2

#define eastRed BIT0
#define eastYellow BIT1
#define eastGreen BIT2
#define westRed BIT3
#define westYellow BIT4
#define westGreen BIT5


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	P1DIR &= ~East_Button;      //Set East Button -> Input
	P1DIR &= ~West_Button;      //Set West Button -> Input

	P2DIR |= eastRed;           //Set red LED on east -> Output
	P2DIR |= eastYellow;        //Set Yellow LED on east -> Output
	P2DIR |= eastGreen;         //Set Green LED on east -> Output
	P2DIR |= westRed;           //Set red LED on west -> Output
	P2DIR |= westYellow;        //Set Yellow LED on west -> Output
	P2DIR |= westGreen;         //Set Green LED on west -> Output

	P2OUT ^= westGreen;
	P2OUT ^= eastRed;

	int trafficWest = 1;
	const unsigned int flowtime = 10000000;
	const unsigned int changedelay = 500000;
	int a,b,i;

	while(1){
         if(!(P1IN & West_Button)){
             __delay_cycles(20000);  // Wait 20ms to debounce
              while(!(P1IN & West_Button));    // Wait till SW Released
             __delay_cycles(20000);
             if(trafficWest != 1){
                 trafficWest = 1;
              //   __delay_cycles(flowtime);
                 for(i=0;i<flowtime;i++);
                 for(i=0;i<flowtime;i++);
                 P2OUT &= ~eastGreen;

                 P2OUT |= eastYellow;
                 __delay_cycles(changedelay);
                 P2OUT &= ~eastYellow;

                 P2OUT |= eastRed;
                 __delay_cycles(changedelay);
                 for(a = 0;a<5;a++){
                     P2OUT &= ~westYellow;
                     __delay_cycles(changedelay);
                     P2OUT |= westYellow;
                     __delay_cycles(changedelay);
                 }
                 P2OUT &= ~westYellow;
                 P2OUT &= ~westRed;
                 P2OUT |= westGreen;
             }
         }
         if(!(P1IN & East_Button)){
             __delay_cycles(20000);  // Wait 20ms to debounce
              while(!(P1IN & East_Button));    // Wait till SW Released
              __delay_cycles(20000);
             if(trafficWest == 1){
                 trafficWest = 0;
             //    __delay_cycles(flowtime);
                 for(i=0;i<flowtime;i++);
                 for(i=0;i<flowtime;i++);

                 P2OUT &= ~westGreen;
                 P2OUT |= westYellow;
                 __delay_cycles(changedelay);

                 P2OUT &= ~westYellow;
                 P2OUT |= westRed;
                 __delay_cycles(changedelay);

                 for(b=0;b<5;b++){
                     P2OUT &= ~eastYellow;
                     __delay_cycles(changedelay);
                     P2OUT |= eastYellow;
                     __delay_cycles(changedelay);
                 }
                 P2OUT &= ~eastYellow;
                 P2OUT &= ~eastRed;
                 P2OUT |= eastGreen;
             }
         }
	}


	return 0;
}
