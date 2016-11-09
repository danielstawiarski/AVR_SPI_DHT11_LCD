/*
 * main.c
 *
 *  Created on: 29 paü 2016
 *      Author: Asia
 */
#include<avr/io.h>
#include<util/delay.h>

#define DHT_PORT_OUT PORTB
#define DHT_PORT_IN PINB
#define DHT_PIN PB1

#define SET_BIT(byte, bit) ((byte) |= (1UL << (bit)))

#define CLEAR_BIT(byte,bit) ((byte) &= ~(1UL << (bit)))

#define IS_SET(byte,bit) (((byte) & (1UL << (bit))) >> (bit))

/* This function initialize SPI. PB5,PB3,PB (MOSI,SCK,SS') as output (MISO is always input in Master))*/
/* SPE - enable SPI. MSTR - set master bit. SPR1,SPR0 (1,1) so SCK = fosc/128 */
void initSPI()
{
    DDRB = ( 1 << PB5 ) | ( 1 << PB3 ) | ( 1 << PB2 );
    SPCR = ( 1 << SPE ) | ( 1 << MSTR ) | ( 1 << SPR1 ) | ( 1 << SPR0 );
}


/* Send temperature with one byte. Write temperature to SPDR register and
 * wait for the end transsmision (when bit SPIF in SPSR register is set) */
void sendTemperaturBySPI(uint8_t temperatureByte)
{
    SPDR = temperatureByte;
    //while( ! bit_is_set( SPSR, SPIF ) );
    while( ! IS_SET( SPSR, SPIF ) );
}


uint8_t fetchData(uint8_t* arr)
{
    uint8_t data [5];
    uint8_t tempTCNTO0, check;
    int8_t i,j;

                         /* Start communication with sensor */
                        /* Use timer 0  to count time periods */


    /* DHT pin as output */
    SET_BIT(DDRB,DHT_PIN);

    /* Set timer's prescaler to clk/1024 */
    TCCR0 = 0x05;

    /* Clean 8 bit register, designed to count impulses */
    TCNT0 = 0;

    /* Low state for DHT pin */
    CLEAR_BIT(DHT_PORT_OUT,DHT_PIN);

    /* Wait  something about 20 ms */
    while(TCNT0 < 160);

    /* Change prescaler for timer to clk/8
     1us - 1 imuplse (8 MHZ) */
    TCCR0 = 0x02;

    /*Clear register which count impulses */
    TCNT0 = 0;

    /* Change state of DHT pin to high */
    SET_BIT(DHT_PORT_OUT,DHT_PIN);

    /* DHT pin as input (wait for response from sensor) */
    CLEAR_BIT(DDRB,DHT_PIN);

    /* Sensor should give response after 20-40 us.
     * If sensor don't clear input DHT PIN - return 0 after 60 us. */
    while(IS_SET(DHT_PORT_IN,DHT_PIN)){
    	if (TCNT0 >= 60)
    		return 0;
    }

    /* Now sensor should send presence pulse */

    TCNT0 = 0;

    /* First should appear low state for 80 us
     * if no - return 0 after 100 us. */
    while(!IS_SET(DHT_PORT_IN,DHT_PIN)){
    	if (TCNT0 >= 100)
    		return 0;
    }

    TCNT0 = 0;

    /* After then should appear hight state on DHT input for 80 us
     * if no - return 0 after 100 us. */
    while(IS_SET(DHT_PORT_IN,DHT_PIN)){
    	if (TCNT0 >= 100)
    		return 0;
    }

    /* If presence pulse didn't return 0 -  Data Transsmision should started.*/

    for (i = 0; i < 5; ++i)
    {
        for(j = 7; j >= 0; --j)
        {
            TCNT0 = 0;

            /* Before send data bit sensor set DHT PIN to low state for 50 us.
             * If this period is longer than 70 us - return 0 */
            while(!IS_SET(DHT_PORT_IN,DHT_PIN)){
            	if (TCNT0 >= 70)
            		return 0;
            }

            TCNT0 = 0;

            /* Data transfer. 26-28 us hight state is a low bit, around 70 us is high bit.
             * If more than 90 us - return 0 */
            while(IS_SET(DHT_PORT_IN,DHT_PIN)){
            	if (TCNT0 >= 90)
            		return 0;
            }

            /* Save the actually state of TCNT0 to tempTCNT0
             * (time still run, so TCNT0 still increase) */
            tempTCNTO0 = TCNT0;

            if (tempTCNTO0 >= 20 && tempTCNTO0 <= 35)
            {
            	CLEAR_BIT(data[i],j);
            }

            else if (tempTCNTO0 >= 60 && tempTCNTO0 <= 80)
            {
            	SET_BIT(data[i],j);
            }

            else return 0;
        }
    }

    /* Check correct of data. Summary first 4 bytes.
     * The last 8 bits of sum need to be equal to the last byte. */

    check = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

    if (check != data[4]) return 0;


    /* If everything fine save data to array "arr" and return 1 */
    for(i = 0; i < 4; ++i)
    { arr[i] = data[i]; }

    return 1;
}

int main()
{
    uint8_t data [4];  /* Array for data from DHT11*/
    _delay_ms(2000);  /* 2 s for DHT11 stabilization */
    initSPI();       /* Init SPI */

    /* Main loop which send data from sensor using SPI. */
    while(1)
    {
    	if(fetchData(data)){
    		/* Send thrid byte - integral byte of Temperature */
    		sendTemperaturBySPI(data[2]);
    	}
       _delay_ms(1000);

    }
}



