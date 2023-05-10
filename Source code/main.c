/*
 * GccApplication4.c
 *
 * Created: 4/19/2023 7:56:36 PM
 * Author : Dinh Viet Hoa
 */ 

#define F_CPU 16000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>	
#include "USART_H_file.h"
#include "SIMTCPClient.h"
#include "ADC_FILE_H.h"
#include "SENSOR_FILE_H.h"

#define CLIENT_ID "nhathongminh"
#define USERNAME "dinhviethoa"
#define PASSWORD "dinhviethoa2706"
#define HOST "demo.thingsboard.io"
#define PORT "1883"
#define TOPIC "v1/devices/me/telemetry"
#define WARN_TEM "nhiet do cao hon muc canh bao"
#define WARN_SMOKE "nong do khoi cao hon muc canh bao"
#define WARN_FIRE "nguy co xay ra chay cao"
#define PHONE_NUMBER "\"0988902563\""
#define WARN_TEM_SERVER "{\"house_status\":overheated}"
#define WARN_GAS_SERVER "{\"house_status\":high gas}"
#define WARN_FIRE_SERVER "{\"house_status\":fire harzard}"
#define NORMAL_STATUS "{\"house_status\":normal}"


char mang[50];
char mess[100];
enum{nhiet_do,do_am,nong_do_khoi};

void number_to_string(uint16_t data_adc)
{
	if(data_adc >= 100)
	{
		mang[0]=data_adc/100+48;
		data_adc%=100;
		mang[1]=data_adc/10+48;
		mang[2]=data_adc%10+48;
	}
	else
	{
		mang[0]=data_adc/10+48;
		mang[1]=data_adc%10+48;
	}
}

//  creat mess JSON form 

void getmess(uint8_t t)
{
	if(t == 0)
	{
		strcat(mess, "{\"temperature\":");
		strcat(mess, mang);
	}
	else 
	if(t == 1)
		{
			strcat(mess,",\"humidity\":");
			strcat(mess, mang);	
		}
		else
		{
			strcat(mess, ",\"smoke\":");
			strcat(mess, mang);
			strcat(mess, "}");
		}
}

int main()
{
	uint16_t sensor;
	bool fire_hazard = false;
	bool overheated = false;
	bool high_gas = false;
	
	adc_init();
	USART_Init();
	
	sei();
	
	while(!(SIM_Start()));
	SIM_reset();
	MQTT_Start();
	TCPClient_Start_MQTT(USERNAME, PASSWORD, CLIENT_ID, HOST, PORT);
	TCPClient_Send_Topic(TOPIC);
	TCPClient_Send_Mess(NORMAL_STATUS);
	TCPClient_Send_Pub();
	MQTT_Stop();
	
	while(1)
	{
		/*		read data from sensors and if house is a fire hazard, SIM will send warn mess to ower's phone	*/
		memset(mang,0,50);
		memset(mess, 0, 100);
		sensor = get_temperature();
		number_to_string(sensor);
		if ((sensor >= 60) && (overheated == false))
		{
			overheated = true;
			SIM_send_SMS(PHONE_NUMBER, WARN_TEM);
			while(!(SIM_Start()));
			SIM_reset();
			MQTT_Start();
			TCPClient_Start_MQTT(USERNAME, PASSWORD, CLIENT_ID, HOST, PORT);
			TCPClient_Send_Topic(TOPIC);
			TCPClient_Send_Mess(WARN_TEM_SERVER);
			TCPClient_Send_Pub();
			MQTT_Stop();
		}
		getmess(nhiet_do);
		
		memset(mang,0,50);
		sensor = get_humidity();
		number_to_string(sensor);
		getmess(do_am);
		memset(mang,0,50);
		
		sensor = get_gas();
		number_to_string(sensor);
		if ((sensor >= 40) && (high_gas == false))
		{
			high_gas == true; 
			SIM_send_SMS(PHONE_NUMBER, WARN_SMOKE);
			while(!(SIM_Start()));
			SIM_reset();
			MQTT_Start();
			TCPClient_Start_MQTT(USERNAME, PASSWORD, CLIENT_ID, HOST, PORT);
			TCPClient_Send_Topic(TOPIC);
			TCPClient_Send_Mess(WARN_GAS_SERVER);
			TCPClient_Send_Pub();
			MQTT_Stop();
		}
		if ((overheated == true) && (high_gas == true) && (fire_hazard == false))
			{
				fire_hazard = true;
				SIM_send_SMS(PHONE_NUMBER, WARN_FIRE);
				while(!(SIM_Start()));
				SIM_reset();
				MQTT_Start();
				TCPClient_Start_MQTT(USERNAME, PASSWORD, CLIENT_ID, HOST, PORT);
				TCPClient_Send_Topic(TOPIC);
				TCPClient_Send_Mess(WARN_FIRE_SERVER);
				TCPClient_Send_Pub();
				MQTT_Stop();
			}
		getmess(nong_do_khoi);
		
		/*		send data sensor on thingsboard		*/
		while(!(SIM_Start()));
		SIM_reset();
		MQTT_Start();
		TCPClient_Start_MQTT(USERNAME, PASSWORD, CLIENT_ID, HOST, PORT);
		TCPClient_Send_Topic(TOPIC);
		TCPClient_Send_Mess(mess);
		TCPClient_Send_Pub();
		MQTT_Stop();
	}
}

