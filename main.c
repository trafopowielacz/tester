/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */



//#define F_CPU 8000000L  //nie pisz tego jak korzystasz z eclipse bo mogą byc błędy

#include <avr/io.h>
#include <util/delay.h>
#include "LCD/lcd44780.h"

// Makra upraszczaj�ce dost�p do port�w
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


//Definicja przycisk�w
#define KEY_PORT			C
#define KEY_PIN 			2


//#define KEY_UP 		!(PIN(KEY_PORT)	 & (1<<UP))

#define LED1 PB0
#define SW PB1


void main(void)
{
	lcd_init();

		lcd_str_P("hello world");

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
