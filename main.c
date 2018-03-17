/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */


#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>

#define LED1 PB0




void main(void)
{
    DDRB  |= (1<<LED1);
    PORTB |= (1<<LED1);
   while (1)
   {
        PORTB ^=(1<<LED1);
        _delay_ms(100);
        _delay_ms(100);
    }
}
