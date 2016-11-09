/*
 * main.c
 *
 *  Created on: 29 paü 2016
 *      Author: Asia
 */
#include<avr/io.h>
#include "HD44780.h"

/* Set MISO as output (to send byte to Master). Enable SPI.*/

void initSPI()
{
    DDRB = ( 1 << PB4 );
    SPCR = ( 1 << SPE );
}

/* Wait for the end of transmission (flag SPIF in status register).
 *  Return result of transmission.*/

char getTemperatureBySPI()
{
    while ( ! bit_is_set( SPSR, SPIF ) );
    return SPDR;
}

/* Display temperature on LCD, with the LCD library.
 * Clear LCD, change result of temperature to char array and display on LCD */

void displayOnLCD(uint8_t temperature){
	LCD_Clear();
	char temperatureArray[] = "   ";
    char degree = (char)223;

	itoa(temperature,temperatureArray,10);
	temperatureArray[2]=degree;
	LCD_GoTo(7,0);
	LCD_WriteText(temperatureArray);
	LCD_GoTo(10,0);
	LCD_WriteText("C");
}

int main()
{
	LCD_Initalize();
	initSPI();

    while(1)
    {
    	uint8_t temperature = getTemperatureBySPI();
        displayOnLCD(temperature);
    }
}

