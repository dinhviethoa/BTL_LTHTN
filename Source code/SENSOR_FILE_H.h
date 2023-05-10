/*
 * SENSOR_FILE_H.h
 *
 * Created: 5/10/2023 10:35:58 PM
 *  Author: Dinh Viet Hoa
 */ 


#ifndef SENSOR_FILE_H_H_
#define SENSOR_FILE_H_H_


#define F_CPU 16000000ul
#include <avr/io.h>
#include "ADC_FILE_H.h"


uint16_t get_temperature();
uint16_t get_gas();
uint16_t get_humidity();

#endif /* SENSOR_FILE_H_H_ */