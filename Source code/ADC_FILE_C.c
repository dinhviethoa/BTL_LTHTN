/*
 * ADC_FILE_C.c
 *
 * Created: 5/10/2023 10:26:24 PM
 *  Author: Dinh Viet Hoa
 */ 
#include "ADC_FILE_H.h"

void adc_init()
{
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t channel)
{
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}