#include <msp430.h>

#define BUTTON1 ((P2IN & 0x02) == 0x00)
#define BUTTON2 ((P2IN & 0x04) == 0x00)
#define BUTTON3 ((P2IN & 0x08) == 0x00)
#define BUTTON4 ((P2IN & 0x10) == 0x00)
#define B1 (P1IN & 0x10)
#define B2 (P1IN & 0x20)
#define B3 (P1IN & 0x40)
#define B4 (P1IN & 0x80)
#define YellowLedOn (P1OUT |= 0x01)
#define GreenLedOn  (P1OUT |= 0x02)
#define RedLedOn    (P1OUT |= 0x04)
#define AllLedsOff  (P1OUT &= ~0x07)

void main(void)
{
 WDTCTL = WDTPW|WDTHOLD;

 int Control = 0;
 int NewPassword[4];
 int YourPassword[4];
 int EnterNewPassword = 0;
 int EnterYourPassword = 0;

 P2SEL = 0x00;
 P1DIR = 0x0F;
 P2DIR = 0xE1;
 P1REN = 0xF0;
 P2REN = 0x1E;
 P1OUT = 0xF0;
 P2OUT = 0x1E;

 while(1)
 {
 if (Control == 0)
 {
 if (BUTTON1)
 {
 EnterNewPassword = 1;
 }
 if (EnterNewPassword == 1)
 {
 AllLedsOff;
 NewPassword[0] = B1;
 NewPassword[1] = B2;
 NewPassword[2] = B3;
 NewPassword[3] = B4;
 if (BUTTON2)
 {
 EnterNewPassword = 0;
 YellowLedOn;
 Control = 1;
 }
 }
 }
 if (Control == 1)
 {
 if (BUTTON3)
 {
 EnterYourPassword = 1;
 }
 if (EnterYourPassword == 1)
 {
 YourPassword[0] = B1;
 YourPassword[1] = B2;
 YourPassword[2] = B3;
 YourPassword[3] = B4;
 if (BUTTON4)
 {
 EnterYourPassword = 0;
 if ((YourPassword[0] == NewPassword[0]) && 
 (YourPassword[1] == NewPassword[1]) && 
 (YourPassword[2] == NewPassword[2]) && 
 (YourPassword[3] == NewPassword[3]))
 {
 AllLedsOff;
 GreenLedOn;
 Control = 0;
 }
 else
 {
 AllLedsOff;
 RedLedOn;
 }}}}}
}
