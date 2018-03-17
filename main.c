/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */


//---------------------------------------------------------------------------------------------
//jest jakis problem z polskimi znakami na gicie lepiej nie uzywac
//---------------------------------------------------------------------------------------------

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


//Definicja przycisków
#define KEY_PORT				B
#define KEY_PIN		 			1

#define KEY 		!(PIN(KEY_PORT)	 & (1<<KEY_PIN))


//Definicja wyjœc
#define LED			0
#define LED_PORT	B
#define LED_OFF		PORT(LED_PORT)&=~(1<<LED)
#define LED_ON		PORT(LED_PORT)|=(1<<LED)
#define LED_TOG		PORT(LED_PORT)^=(1<<LED)


#define LED1 PB0
#define SW PB1


void main(void)
{
	uint8_t key_lock=0;

	//przycisk
	DDR(KEY_PORT)	&= ~(1<<KEY_PIN);
	PORT(KEY_PORT)	|= (1<<KEY_PIN);

	//dioda
	DDR(LED_PORT)	|=	(1<<LED);
	LED_OFF;

	lcd_init(); //inicjalizuje wysw

	while(1)
	{
		if(!key_lock && KEY)
		{
			LED_TOG;
			key_lock = 1;
		}
		if(!KEY)	key_lock = 0;
		_delay_ms(100);


	}
}
