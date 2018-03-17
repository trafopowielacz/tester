/*
 * main.c
 *      Author: superhiro2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include "Onewire/ds18x20.h"
#include "LCD/lcd4478.h"
#include "Onewire/crc8.h"
#include "dht/dht.h"

#include <avr/wdt.h>		//za³¹cznie pliku watch dog'a

// Makra upraszczaj¹ce dostêp do portów
//* *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


#define OBROTY	1 //0 - sekundy, 1 - minuty zmienna czasobrot

#define STOPIEN			223			//znak stopnia w ascii

#define STEROWANIE 		5			//liczba okreslaj¹ca kroki w PWM

//Definicja przycisków up,down,ok
#define KEY_PORT			C
//#define UP 					2
//#define DOWN 				3
#define OK	    			4
//#define EXIT	    		5
#define ENKODER_A			3

//#define KEY_UP 		!(PIN(KEY_PORT)	 & (1<<UP))
//#define KEY_DOWN 	!(PIN(KEY_PORT)  & (1<<DOWN))
#define KEY_OK 		!(PIN(KEY_PORT)	 & (1<<OK))
//#define KEY_EXIT 	!(PIN(KEY_PORT)	 & (1<<EXIT))
#define ENKODER_A_PIN !(PIN(KEY_PORT) & (1<ENKODER_A))

#define TR1			2			//pin triaka 1
#define TR1_PORT	B			//port triaka 1
#define TR1_OFF		PORT(TR1_PORT)&=~(1<<TR1)
#define TR1_ON		PORT(TR1_PORT)|=(1<<TR1)

#define TR2			3
#define TR2_PORT	B
#define TR2_OFF		PORT(TR2_PORT)&=~(1<<TR2)
#define TR2_ON		PORT(TR2_PORT)|=(1<<TR2)

#define MOS1		4			//pin mosfeta 1
#define MOS1_PORT	B			//port mosfeta 1
#define MOS1_OFF	PORT(MOS1_PORT)&=~(1<<MOS1)
#define MOS1_ON		PORT(MOS1_PORT)|=(1<<MOS1)

#define MOS2		5			//pin mosfeta 1
#define MOS2_PORT	B			//port mosfeta 1
#define MOS2_OFF	PORT(MOS2_PORT)&=~(1<<MOS2)
#define MOS2_ON		PORT(MOS2_PORT)|=(1<<MOS2)


#define UGN 5 //iloœc stopni regulacji

//Zmienne globalne

//obs³uga czasu
volatile uint8_t cnt_9m6=0, flag_9m6=0, cnt_100m=0, flag_100m=1, cnt_1s=0, flag_1s=1;
volatile uint8_t key_lock=0;						// klawisze
volatile uint8_t sekundy=0;
volatile uint8_t enkoder_kierunek=0;

static EEMEM uint16_t	temp_zad_ee;		//zmienne eepromu
static EEMEM uint8_t	PGN_ee, IGN_ee, DGN_ee;		//wzmocnienia cz³onów P I D
//PGN 25 IGN 0 DGN 0
static EEMEM uint8_t	IMD_ee;				//maksymalny uchyb dla cz³onu I (1*C)
static EEMEM uint8_t	czasobrot_ee, coileobrot_ee;


int main(void)
{
//	Definicje zmiennych------------------------------------------------------------------------------
	uint8_t subzero, cel, cel_fract_bits;  	//znak, czesci dzies i ulamkowe temp
	uint16_t temp_pomiar, temp_zadana;		//Temperatura pomiar i zadana 37,5*C
	uint8_t humidity;						//wilgotnosc
	int16_t uchyb=0, uchyb_poprz=0, uchyb_suma=0, pid_out=0;	//zmienne uchybów, wyjœcie pid
	uint8_t key_change=0;					//flaga zmiany ustawien
	uint8_t Hcnt=0, Hpower=0;					//licznik od pwm grzania, moc grzania
	uint8_t PGN, IGN, DGN, IMD;
	uint8_t	czasobrot, coileobrot;


	uint8_t	menu_level=0, menu_value=0, menu_subvalue=0;					//Zmienna okreslaj¹ca poziom menu

	uint8_t minuty=0, godziny=0, godzinaobrot=0, dzien=0;
	uint8_t flag_obrot=0;
	//const char strzala 			PROGMEM = ">";
	const char czas[] 			 = "Czas:           ";
	const char wzmocnieniep[] 	 = "Wzmocnienie P:  ";
	const char wzmocnieniei[]  	 = "Wzmocnienie I:  ";
	const char wzmocnienied[] 	 = "Wzmocnienie D:  ";
	const char granica[]  		 = "Granica calki:  ";
	const char wyjscie[]		 = "WYJSCIE         ";

	const char dzienn[] 		 = "Dzien:          ";
	const char coileobr[] 		 = "Obrot co:       ";
	const char obrile[]			 = "Ile obracaj:    ";
	const char obrteraz[] 		 = "Wykonaj obrot   ";
	//---------------------- ODCZYT EEPROM------------------------
	temp_zadana	=	eeprom_read_word(&temp_zad_ee);
	if(temp_zadana> 400 || temp_zadana < 200)
	{
		eeprom_write_word(&temp_zad_ee,375);
		eeprom_write_byte(&PGN_ee,25);
		eeprom_write_byte(&IGN_ee,0);
		eeprom_write_byte(&DGN_ee,0);
		eeprom_write_byte(&IMD_ee,10);
		eeprom_write_byte(&czasobrot_ee,10);
		eeprom_write_byte(&coileobrot_ee,1);

	}

	temp_zadana	=	eeprom_read_word(&temp_zad_ee);
	PGN			=	eeprom_read_byte(&PGN_ee);
	IGN			=	eeprom_read_byte(&IGN_ee);
	DGN			=	eeprom_read_byte(&DGN_ee);
	IMD			=	eeprom_read_byte(&IMD_ee);
	czasobrot  	= 	eeprom_read_byte(&czasobrot_ee);
	coileobrot 	= 	eeprom_read_byte(&coileobrot_ee);

	//-------------------------------------------------------------------

	//Piny przycisków jako we oraz podci¹gniêcie
	DDR(KEY_PORT)	&= ~(1<<OK)|(1<<ENKODER_A);
	PORT(KEY_PORT)	|= (1<<OK)|(1<<ENKODER_A);
	PORTD 			|= (1<<PD2);			//INT0

	//Pin sterowania jako wy i 0
	DDR(TR1_PORT)	|=	(1<<TR1);
	DDR(TR2_PORT)	|=	(1<<TR2);
	DDR(MOS1_PORT)	|=	(1<<MOS1);
	DDR(MOS2_PORT)	|=	(1<<MOS2);

	TR1_OFF;			//PORT(HEAT_PORT)&=~(1<<HEAT)
	TR2_OFF;
	MOS1_OFF;
	MOS2_OFF;

	//Przerwanie ustawienia
	TCCR1B |= (1<<WGM13)|(1<<WGM12); 	// CTC
	TCCR1B |= (1<<CS11)|(1<<CS10); 		// prescaler = 64
	//TCCR1B |= (1<<CS10); 				// prescaler = 8

	ICR1 = 1200;   //teoretycznie 1200 dla 104Hz
	TIMSK |= (1<<OCIE1B);

	//przerwanie INT0
	GICR |= (1<<INT0);
	MCUCR |= (1<<ISC00);



	lcd_init();					//inicjlizacja wyswietlacza

	lcd_str_P(PSTR("   INKUBATOR"));
	search_sensors();									//szuka czujników
	DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL );	//zlecenie konwersji temp

	_delay_ms(500);
	_delay_ms(500);
	DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits);		//Pierwszy pomiar temperatury

	lcd_cls();
	wdt_enable(WDTO_2S);
	sei();		//odblokowanie przerwan

	while(1)
	{

//------------------------------         OBS£UGA KLAWISZY          ----------------------
//---------------------------	DOWN	-------------------------------------------------
		if( enkoder_kierunek == 2 )
		{
			key_lock=1;
			key_change++;
			cnt_1s=0;
			if(menu_level==0 && temp_zadana>350)	temp_zadana--;		//zadana -0,1*C

			if(menu_level==1 && menu_value <5)	menu_value++;			//przesuñ poziom menu w dó³
			if(menu_level==2 && menu_value==0 && menu_subvalue<4)	menu_subvalue++;
			if(menu_level==2 && menu_value==1 && PGN > 0)	PGN--;
			if(menu_level==2 && menu_value==2 && IGN > 0)	IGN--;
			if(menu_level==2 && menu_value==3 && DGN > 0)	DGN--;
			if(menu_level==2 && menu_value==4 && IMD > 0)	IMD--;
			if(menu_level==3 && menu_subvalue == 0 && dzien > 0) dzien--;
			if(menu_level==3 && menu_subvalue == 1 && coileobrot > 1) coileobrot--;
			if(menu_level==3 && menu_subvalue == 2 && czasobrot > 0) czasobrot--;

			enkoder_kierunek = 0;
		}

//---------------------------	UP	-------------------------------------------------
		if( enkoder_kierunek == 1 )
		{
			key_lock=1;
			key_change++;
			cnt_1s=0;
			if(menu_level==0 && temp_zadana<400)	temp_zadana++;		//zadana +0,1*C

			if(menu_level==1 && menu_value >0)	menu_value--;		//menu w góre
			if(menu_level==2 && menu_value==0 && menu_subvalue>0) menu_subvalue--;
			if(menu_level==2 && menu_value==1 && PGN < 40)	PGN++;
			if(menu_level==2 && menu_value==2 && IGN < 20)	IGN++;
			if(menu_level==2 && menu_value==3 && DGN < 20)	DGN++;
			if(menu_level==2 && menu_value==4 && IMD < 20)	IMD++;
			if(menu_level==3 && menu_subvalue == 0 && dzien < 25) dzien++;
			if(menu_level==3 && menu_subvalue == 1 && coileobrot < 12) coileobrot++;
			if(menu_level==3 && menu_subvalue == 2 && czasobrot < 60) czasobrot++;

			enkoder_kierunek = 0;
		}
//---------------------------	OK	-------------------------------------------------
		if( !key_lock && KEY_OK )
		{
			key_lock=1;
			key_change++;
			cnt_1s=0;
			switch(menu_level)
			{
			case 0:
			{
				menu_level = 1;
				menu_value = 0;
				menu_subvalue = 0;
			}
			break;
			case 1:
			{
				if(menu_value != 5)
				{
				menu_level = 2;		//wejscie do podmenu
				menu_subvalue = 0;
				}
				if(menu_value == 5)	menu_level = 0;		//exit
			}
			break;
			case 2:
			{
				if(menu_value != 0)	menu_level = 1;		//zatwerdzenie podmenu
				if(menu_value == 0 && menu_subvalue < 3) menu_level = 3;	//we pod podmenu
				if(menu_value == 0 && menu_subvalue == 3)
				{
					flag_obrot = czasobrot;
					sekundy=55;
				}
				if(menu_value == 0 && menu_subvalue == 4) menu_level = 1;	//exit
			}
			break;
			case 3:
			{
				menu_level = 2;							//zatwierdzenie pod podmenu
			}
			break;
			}
			//if(menu_level == 1 && menu_value != 5)	menu_level = 2;		//wejscie do podmenu
			//if(menu_level == 1 && menu_value == 5)	menu_level = 0;		//exit
			//if(menu_level == 2 && menu_value != 0)	menu_level = 1;		//zatwerdzenie podmenu
			//if(menu_level == 2 && menu_value == 0 && menu_subvalue < 3) menu_level = 3;	//we pod podmenu
			//if(menu_level == 2 && menu_value == 0 && menu_subvalue == 3) flag_obrot = czasobrot;
			//if(menu_level == 2 && menu_value == 0 && menu_subvalue == 4) menu_level = 1;	//exit
			//if(menu_level == 3)	menu_level = 2;							//zatwierdzenie pod podmenu
		}

//		if( !key_lock && KEY_EXIT )
//		{
//			key_lock=1;
//			key_change++;
//			cnt_1s=0;
//
//			if(menu_level>1) menu_level--;
//			else menu_level = 0;
//
//		}
//------------------------------------------------------------------------------------------


//-------------------------------------CZAS CZAS CZAS----------------------------
		if(sekundy>59)
		{
			if(++minuty>59)
			{
				if(++godzinaobrot == coileobrot)
				{
					flag_obrot = czasobrot;
					godzinaobrot = 0;
				}
				if(++godziny>24)
				{
					dzien++;
					godziny=0;
				}
				minuty = 0;
			}
#if (OBROTY==1)
			if(flag_obrot>0 && dzien < 19)
			{
				MOS1_ON;
				flag_obrot--;
			}
			else MOS1_OFF;
#endif

			sekundy=0;
		}


//=================== Sterowanie grza³k¹ ===================================================
		if(flag_9m6)
		{
			if(++Hcnt == STEROWANIE)	Hcnt = 0;
			if(Hcnt <= Hpower)	TR1_ON;
			else 				TR1_OFF;
			flag_9m6 = 0;


			if(menu_level==0)
			{
				lcd_locate(1,0);

				if(PORT(TR1_PORT) & (1<<TR1))	lcd_str_P(PSTR("P1 "));
				else lcd_str_P(PSTR("   "));


				if(PORT(TR2_PORT) & (1<<TR2))	lcd_str_P(PSTR("P2"));
				else lcd_str_P(PSTR("  "));

				lcd_locate(1,12);
				if(PORT(MOS1_PORT) & (1<<MOS1))	lcd_str_P(PSTR("M1 "));
				else lcd_str_P(PSTR("   "));


				if(PORT(MOS2_PORT) & (1<<MOS2))	lcd_str_P(PSTR("M2"));
				else lcd_str_P(PSTR("   "));
			}
		}

//------------------------------------------------------------------------------------------
		if(flag_100m)
		{

				if(menu_level == 0)
				{
					lcd_locate(0,0);
					lcd_int(cel);
					lcd_str_P(PSTR(","));
					lcd_int(cel_fract_bits);
					lcd_char(STOPIEN);
					lcd_str_P(PSTR("C     "));
					lcd_locate(0,10);
					lcd_int(temp_zadana/10);
					lcd_str_P(PSTR(","));
					lcd_int(temp_zadana%10);
					lcd_char(STOPIEN);
					lcd_str_P(PSTR("C "));
					lcd_locate(1,7);
					lcd_int(humidity);
					lcd_str_P(PSTR("%W"));


					//1234567890123456
					//DD,U*C    DD,U*C
					//P1 P2 XX%W M1 M2
				}

				if(menu_level == 1)
				{
					lcd_locate(0,0);
					lcd_str_P(PSTR(">               "));
					lcd_locate( 0 ,1);

					if(menu_value == 0)
					{
						lcd_str(czas);
						lcd_locate(1 ,0);
						lcd_str(wzmocnieniep);
					}
					if(menu_value == 1)
					{
						lcd_str(wzmocnieniep);
						lcd_locate(1 ,0);
						lcd_str(wzmocnieniei);
					}
					if(menu_value==2)
					{
						lcd_str(wzmocnieniei);
						lcd_locate(1 ,0);
						lcd_str(wzmocnienied);
					}
					if(menu_value==3)
					{
						lcd_str(wzmocnienied);
						lcd_locate(1 ,0);
						lcd_str(granica);
					}
					if(menu_value==4)
					{
						lcd_str(granica);
						lcd_locate(1 ,0);
						lcd_str(wyjscie);
					}
					if(menu_value==5)
					{
						lcd_str(wyjscie);
						lcd_locate(1 ,0);
						lcd_str_P(PSTR("                "));
					}
				}
				if(menu_level == 2)
				{
					if(menu_value == 0)
					{
						lcd_locate(0,0);
						lcd_str_P(PSTR(">               "));
						lcd_locate(0,1);
						if(menu_subvalue==0)
						{
							lcd_str(dzienn);
							lcd_locate(1 ,0);
							lcd_str(coileobr);
						}
						if(menu_subvalue==1)
						{
							lcd_str(coileobr);
							lcd_locate(1 ,0);
							lcd_str(obrile);
						}
						if(menu_subvalue==2)
						{
							lcd_str(obrile);
							lcd_locate(1 ,0);
							lcd_str(obrteraz);
						}
						if(menu_subvalue==3)
						{
							lcd_str(obrteraz);
							lcd_locate(1 ,0);
							lcd_str(wyjscie);
						}
						if(menu_subvalue==4)
						{
							lcd_str(wyjscie);
							lcd_locate(1 ,0);
							lcd_str_P(PSTR("                "));
						}

					}
					if(menu_value == 1)
					{
						lcd_locate(1,0);
						lcd_int(PGN);
						lcd_str_P(PSTR("                "));
					}
					if(menu_value == 2)
					{
						lcd_locate(1,0);
						lcd_int(IGN);
						lcd_str_P(PSTR("                "));
					}
					if(menu_value == 3)
					{
						lcd_locate(1,0);
						lcd_int(DGN);
						lcd_str_P(PSTR("                "));
					}
					if(menu_value == 4)
					{
						lcd_locate(1,0);
						lcd_int(IMD/10);
						lcd_char(',');
						lcd_int(IMD%10);
						lcd_char(STOPIEN);
						lcd_str_P(PSTR("C                "));
					}
				}
				if(menu_level == 3)
				{
					if(menu_subvalue == 0)
					{
						lcd_locate(1,0);
						lcd_int(dzien);
						lcd_str_P(PSTR(" Dzien                "));
					}
					if(menu_subvalue == 1)
					{
						lcd_locate(1,0);
						lcd_int(coileobrot);
						lcd_str_P(PSTR(" h                "));
					}
					if(menu_subvalue == 2)
					{
						lcd_locate(1,0);
						lcd_int(czasobrot);
						lcd_str_P(PSTR(" min              "));
					}
				}

				//1234567890123456
				//DD,U*C    DD,U*C
				//T. Wew   *Zadana
				//P1 P2  MOS1 MOS2


				//1234567890123456
				//Histereza:
				//D,U*C


			flag_100m=0;
		}
//------------------------------------------------------------------------------------------------

//=============================== POMIAR I PID ================================
//-------------------------------------------------------------------------------------------------

		if(flag_1s)
		{



			dht_humidity(&humidity);

			if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits));
			else
			{
				search_sensors();
				DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits);
			}

			temp_pomiar  = (uint16_t)cel*10;			//zamiana temperatury z postaci dwóch 8 bitowych liczb
			temp_pomiar += cel_fract_bits;			//na 1 16bitow¹ np. 37,5*C to 375


//=====================================================================================
			uchyb = temp_zadana - temp_pomiar;
			//Ca³ka
			if(abs(uchyb)<=IMD)
			{

				uchyb_suma += (uchyb_poprz + uchyb) / 2;
				if(uchyb_suma > 150) uchyb_suma = 150;
				if(uchyb_suma < -150) uchyb_suma = -150;
			}
			else
				uchyb_suma = 0;


			//P
			pid_out = PGN * uchyb;

			//D
			pid_out += (DGN*(uchyb-uchyb_poprz));

			//I
			pid_out += IGN * uchyb_suma / 10;

			pid_out *= STEROWANIE;
			pid_out /= 100;
			if(pid_out<0) 			pid_out = 0;
			if(pid_out>STEROWANIE) 	pid_out = STEROWANIE;
			Hpower = pid_out;

			uchyb_poprz = uchyb;
			DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL );		//zlecenie konwersji temp


#if (OBROTY==0)
			if(flag_obrot>0 && dzien < 19)
				{
				MOS1_ON;
				flag_obrot--;
				}
			else MOS1_OFF;
#endif

			wdt_reset();											//RESET WATCH DOG'a
			flag_1s=0;
		}

//--------------------------------------------------------------------------------------------------

		if(key_change)
		{
			if(cnt_1s>20)
			{

				eeprom_write_word(&temp_zad_ee, temp_zadana);
				eeprom_write_byte(&PGN_ee, PGN);
				eeprom_write_byte(&IGN_ee, IGN);
				eeprom_write_byte(&DGN_ee, DGN);
				eeprom_write_byte(&IMD_ee, IMD);

				cnt_1s=0;
				key_change=0;
			}
		}
		else	cnt_1s=0;

	}

}


// cia³o procedury obs³ugi przerwania Compare Match Timera1
ISR( TIMER1_COMPB_vect)
{

	flag_9m6 = 1;
	if(++cnt_9m6>9)
	{
		flag_100m++;
		if(++cnt_100m>9)
		{
			flag_1s++;							//flaga co 1s
			cnt_1s++;
			sekundy++;
			cnt_100m = 0;
		}
		cnt_9m6 = 0;
	}
	if(key_lock && flag_100m) 				//jesli przyciski odblokowanie to nie blokuj
	{
		if(++key_lock>5) key_lock=0;		//odblokowanie przycisków po conajmniej 100ms
	}
}

//przerwanie zewnêtrzne int0

ISR(INT0_vect)
{
	static uint8_t dir;

	if(!(PIND & (1<<PD2)))
	{
		dir = ENKODER_A_PIN;
	}
	else
	{
		if(dir != ENKODER_A_PIN)
		{
			dir+=1;
			enkoder_kierunek = dir;

		}
	}

}





