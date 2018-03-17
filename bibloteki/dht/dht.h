/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - DHT-11 Library, by Charalampos Andrianakis on 18/12/11
*/


#ifndef DHT_H_
#define DHT_H_

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//setup port
#define DHT_DDR DDRB
#define DHT_PORT PORTB
#define DHT_PIN PINB
#define DHT_INPUTPIN PB1

//sensor type
#define DHT_TYPE DHT_DHT11

#define DHT_DHT11 1
#define DHT_DHT22 2
//enable decimal precision (float)
#if DHT_TYPE == DHT_DHT11
#define DHT_FLOAT 0
#elif DHT_TYPE == DHT_DHT22
#define DHT_FLOAT 1
#endif

//timeout retries
#define DHT_TIMEOUT 200

//functions
#if DHT_FLOAT == 1
int8_t dht_measure(int16_t *temperature, int8_t *sign, int16_t *humidity);
int8_t dht_humidity(int16_t *humidity);
#elif DHT_FLOAT == 0
int8_t dht_measure(int8_t *temperature, uint8_t *humidity);
int8_t dht_humidity(uint8_t *humidity);
#endif

#endif
