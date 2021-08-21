#include <msp430.h>

#define ONOFF ((P2IFG & 0x40) == 0x40)
#define RSPEED ((P2IFG & 0x04) == 0x04)
#define NWASH ((P2IFG & 0x08) == 0x08)
#define PWASH ((P2IFG & 0x10) == 0x10)
#define FSPIN ((P2IFG & 0x20) == 0x20)
#define RedLedToggle (P2OUT ^= 0x01)
#define YellowLedToggle (P2OUT ^= 0x02)
#define NormalWash 1
#define PreWash 2
#define FinalSpin 3

int Program=0;
int RotationSpeed = 0;
int open = 0;

void delay_ms(int);
void Wash();

void main(void)
{
    WDTCTL = WDTPW|WDTHOLD;

    P2SEL = 0x00;
    P1DIR = 0xFF;
    P2DIR = 0x83;
    P2REN = 0x7C;
    P1OUT = 0x00;
    P2OUT = 0x7C;
    P2IE = 0x7C;
    P2IES = 0x7C;
    P2IFG = 0x00;

    __bis_SR_register(GIE);

    while(1)
    {
        if(Program!=0) Wash();
    }
}

void delay_ms(int a)
{
    while(a != 0)
    {
        _delay_cycles(1000);
        a--;
    }
}

void Wash()
{
    int speed, turn, fast, slow, Rturn, Lturn, pos;
    volatile unsigned int seqr[8] = {0x08,0x0C,0x04,0x06,\
                                     0x02,0x03,0x01,0x09
                                    };
    volatile unsigned int seql[8] = {0x09,0x01,0x03,0x02,\
                                     0x06,0x04,0x0C,0x08
                                    };

    if (Program == 1)
    {
        slow = 20, fast = 10, Rturn = 100, Lturn = 100;
    }
    if (Program == 2)
    {
        slow = 20, fast = 10, Rturn = 30, Lturn = 30;
    }
    if (Program == 3)
    {
        slow = 10, fast = 5, Rturn = 50, Lturn = 0;
    }

    if(RotationSpeed == 0)
        speed = fast;
    else
        speed = slow;

    for (turn=0; turn<Rturn; turn++)
    {
        if(open == 1)
        {
            pos = 0;
            while(pos<8)
            {
                P1OUT = seqr[pos];
                pos++;
                delay_ms(speed);
            }
            P1OUT = 0x00;
        }
        else break;
    }
    for (turn=0; turn<Lturn; turn++)
    {
        if(open == 1)
        {
            pos = 0;
            while(pos<8)
            {
                P1OUT = seql[pos];
                pos++;
                delay_ms(speed);
            }
            P1OUT = 0x00;
        }
        else break;
    }
    Program = 0;
}

#pragma vector =PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    if(ONOFF)
    {
        open ^= 1;
        RedLedToggle;
        P2IFG &= ~0x40;
    }
    if ((RSPEED) && (open == 1))
    {
        RotationSpeed ^= 1;
        YellowLedToggle;
        P2IFG &= ~0x04;
    }
    if ((NWASH) && (open == 1))
    {
        Program = NormalWash;
        P2IFG &= ~0x08;
    }
    if ((PWASH) && (open == 1))
    {
        Program = PreWash;
        P2IFG &= ~0x10;
    }
    if ((FSPIN) && (open == 1))
    {
        Program = FinalSpin;
        P2IFG &= ~0x20;
    }
    P2IFG=0x00;
}
