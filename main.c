/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */


//#define F_CPU 8000000L  //nie pisz tego jak korzystasz z eclipse bo mogą byc błędy


#include <avr/io.h>
#include <util/delay.h>

#define LED1 PB0
#define SW PB1




void main(void)
{
    DDRB  |= (1<<LED1);


    DDRB &= ~(1<<SW);
    PORTB |= (1<<SW);

   while (1)
   {
        if(  !(PINB & (1<<SW) )) PORTB |= (1<<LED1) ;
        else ;//PORTB &= ~(1<<LED1);
    }
}
