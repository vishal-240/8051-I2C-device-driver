#define	F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <util/delay.h> 
#include "uart.h"
unsigned int ADC_read()
{
	
	ADCSRA|=(1<<ADSC);		 // start conversion
	while(!(ADCSRA & (1<<ADIF)));   // wait for ADIF conversion complete return
	ADCSRA|=(1<<ADIF);   		// clear ADIF when conversion complete by writing 1
	return (ADCH);			//return calculated ADC value
}


int main(void)
{   int space=0x20;
	int len=0;
	char res[20];
	float value=0.0;
	unsigned char data[] = "The voltage is:";
	
	USART_Init();
	DDRD = 0xFF;        	 				 //make PORTC as output to connect 8 leds
	ADMUX=chnl;      					// Selecting internal reference voltage
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);     // Enable ADC also set Prescaler as 128
	
	int i = 0; // define an integer to save adc read value
	
	while (1)
	{
		
		i = ADC_read(0);
		value =i*0.0196;
		ftoa(value, res, 4);
		len=strlen(res);
		
		for (int j=0;j<15;j++)
		{
		USART_TransmitPolling(data[j]);
		}
		USART_TransmitPolling(space);
		for (int j=0;j<len+1;j++)
		{
			USART_TransmitPolling(res[j]);
		}
		USART_TransmitPolling('V');
		USART_TransmitPolling('\n');
		_delay_ms(1000);
	}
}