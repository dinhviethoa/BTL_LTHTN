/*
 * ADC_FILE_H.h
 *
 * Created: 5/10/2023 10:26:42 PM
 *  Author: Dinh Viet Hoa
 */ 


#ifndef ADC_FILE_H_H_
#define ADC_FILE_H_H_

#define F_CPU 16000000ul
#include <avr/io.h>


void adc_init();
uint16_t adc_read(uint8_t channel);



#endif /* ADC_FILE_H_H_ */