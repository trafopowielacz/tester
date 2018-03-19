/*
 * main.c
 *
 *  Created on: 17 mar 2018
 *      Author: dominik
 */


//---------------------------------------------------------------------------------------------
//jest jakis problem z polskimi znakami na gicie lepiej nie uzywac
//---------------------------------------------------------------------------------------------

//#define F_CPU 8000000L  //nie pisz tego jak korzystasz z eclipse bo mogą byc błędy

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "LCD/lcd4478.h"
//#include "OLED/ssd1306xled.h"



// Makra upraszczajace dostep do portow
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


//Definicja przyciskow
#define KEY_PORT	 	B
#define KEY_PIN			1

#define KEY 		!(PIN(KEY_PORT)	 & (1<<KEY_PIN))


//Definicja wyjsc
#define LED	0
#define LED_PORT	B
#define LED_OFF			PORT(LED_PORT)&=~(1<<LED)
#define LED_ON			PORT(LED_PORT)|=(1<<LED)
#define LED_TOG			PORT(LED_PORT)^=(1<<LED)



void main(void)
{



	uint8_t key_lock=0;

	//przycisk
	DDR(KEY_PORT)	&= ~(1<<KEY_PIN);
	PORT(KEY_PORT)	|= (1<<KEY_PIN);

	//dioda
	DDR(LED_PORT)	|=	(1<<LED);
	LED_ON;


	lcd_init();
	_delay_ms(250);
	lcd_locate(0,0);
	lcd_str("hello world");
	lcd_locate(1,0);
	lcd_str("test");
	_delay_ms(1000);

	while(1)
	{
		if(KEY)	LED_TOG;
		_delay_ms(500);
	}
}

