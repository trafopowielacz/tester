/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */



//#define F_CPU 8000000L  //nie pisz tego jak korzystasz z eclipse bo mogÄ… byc bÅ‚Ä™dy

#include <avr/io.h>
#include <util/delay.h>
#include "LCD/lcd44780.h"

// Makra upraszczaj¹ce dostêp do portów
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


#define LED1 PB0
#define SW PB1


void main(void)
{
	uint8_t key_lock=0;

    DDRB  |= (1<<LED1);


    DDRB &= ~(1<<SW);
    PORTB |= (1<<SW);

   while (1)
   {
        if(!(PINB&(1<<SW) )) PORTB ^= (1<<LED1) ;
        else ;
        _delay_ms(100);
        _delay_ms(100);

    }
}
