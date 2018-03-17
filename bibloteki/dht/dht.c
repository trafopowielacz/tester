/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "dht.h"

/*
 * get data from sensor
 */
#if DHT_FLOAT == 1
int8_t dht_measure(int16_t *temperature, int8_t *sign, uint16_t *humidity) {
#elif DHT_FLOAT == 0
int8_t dht_measure(int8_t *temperature, uint8_t *humidity) {
#endif
	uint8_t bits[5];
	uint8_t i,j = 0;
	uint8_t sreg;

	sreg=SREG;
	cli();

	memset(bits, 0, sizeof(bits));

	//reset port
	DHT_DDR |= (1<<DHT_INPUTPIN); //output
	DHT_PORT |= (1<<DHT_INPUTPIN); //high
	_delay_ms(10);

	cli();
	//send request
	DHT_PORT &= ~(1<<DHT_INPUTPIN); //low
	#if DHT_TYPE == DHT_DHT11
	_delay_ms(18);
	#elif DHT_TYPE == DHT_DHT22
	_delay_us(500);
	#endif
	DHT_PORT |= (1<<DHT_INPUTPIN); //high
	DHT_DDR &= ~(1<<DHT_INPUTPIN); //input
	_delay_us(40);

	//check start condition 1
	if((DHT_PIN & (1<<DHT_INPUTPIN))) {
		return -1;
	}
	_delay_us(80);
	//check start condition 2
	if(!(DHT_PIN & (1<<DHT_INPUTPIN))) {
		return -1;
	}
	_delay_us(80);

	//read the data
	uint16_t timeoutcounter = 0;
	for (j=0; j<5; j++) { //read 5 byte
		uint8_t result=0;
		for(i=0; i<8; i++) {//read every bit
			timeoutcounter = 0;
			while(!(DHT_PIN & (1<<DHT_INPUTPIN))) { //wait for an high input (non blocking)
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
			_delay_us(30);
			if(DHT_PIN & (1<<DHT_INPUTPIN)) //if input is high after 30 us, get result
				result |= (1<<(7-i));
			timeoutcounter = 0;
			while(DHT_PIN & (1<<DHT_INPUTPIN)) { //wait until input get low (non blocking)
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
		}
		bits[j] = result;
	}
	SREG=sreg; // sei()
	//reset port
//	DHT_DDR |= (1<<DHT_INPUTPIN); //output
	DHT_PORT |= (1<<DHT_INPUTPIN); //high
//	_delay_ms(30);

	//check checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		//return temperature and humidity
		#if DHT_TYPE == DHT_DHT11
		*temperature = bits[2];
		*humidity = bits[0];
		#elif DHT_TYPE == DHT_DHT22
		int16_t humidity = bits[0]<<8 | bits[1];
		int16_t temperature = bits[2]<<8 | bits[3];
		if(temperature & 0x8000)
		{
			*temperature &= 0x7FFF;
			*sign = 1;
		}
		*humidity = humidity;
		#endif
		return 0;
	}

	return -1;
}

/*
 * get humidity
 */
#if DHT_FLOAT == 1
int8_t dht_humidity(uint16_t *humidity)
{
	int8_t temperature = 0;
	int8_t sign = 0;
	return dht_measure(&temperature, &sign, humidity);
#elif DHT_FLOAT == 0
int8_t dht_humidity(uint8_t *humidity)
{
	int8_t temperature = 0;
	return dht_measure(&temperature, humidity);
#endif
}


