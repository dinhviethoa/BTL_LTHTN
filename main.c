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

enum{nhiet_do,do_am,nong_do_khoi};
uint16_t data_adc, sensor; 
char mang[50];
char mess[100];

const uint32_t impedance_ohms_table[15][13] =
{
	//Temperature
	//00       05         10        15        20       25       30       35       40       45       50       55       60        Relative Humidity   /  RH Step
	
	//T Step
	// 0         1         2         3         4        5        6        7        8        9       10       11       12

	17500000, 15000000, 12500000, 10000000, 6700000, 5000000, 3900000, 3000000, 2400000, 1750000, 1450000, 1150000, 970000,   //20%              /   0
	13000000, 10000000,  7000000,  5000000, 3400000, 2600000, 1900000, 1500000, 1100000,  880000,  700000,  560000, 450000,   //25%              /   1
	6400000,  4600000,  3200000,  2300000, 1750000, 1300000,  970000,  740000,  570000,  420000,  340000,  270000, 215000,   //30%              /   2
	2900000,  2100000,  1500000,  1100000,  850000,  630000,  460000,  380000,  280000,  210000,  170000,  150000, 130000,   //35%              /   3
	1400000,  1000000,   750000,   540000,  420000,  310000,  235000,  190000,  140000,  110000,   88000,   70000,  57000,   //40%              /   4
	700000,   500000,   380000,   280000,  210000,  160000,  125000,  100000,   78000,   64000,   50000,   41000,  34000,   //45%              /   5
	370000,   260000,   200000,   150000,  115000,   87000,   69000,   56000,   45000,   38000,   31000,   25000,  21000,   //50%              /   6
	190000,   140000,   110000,    84000,   64000,   49000,   39000,   33000,   27000,   24000,   19500,   17000,  14000,   //55%              /   7
	105000,    80000,    62000,    50000,   39000,   31000,   25000,   20000,   17500,   15000,   13000,   11000,   9400,   //60%              /   8
	62000,    48000,    37000,    30000,   24000,   19500,   16000,   13000,   11500,   10000,    8600,    7600,   6800,   //65%              /   9
	38000,    30000,    24000,    19000,   15500,   13000,   10500,    9000,    8000,    7000,    6000,    5400,   4800,   //70%              /   10
	23000,    18000,    15000,    12000,   10000,    8400,    7200,    6200,    5600,    4900,    4200,    3800,   3400,   //75%              /   11
	15500,    12000,    10000,     8000,    7000,    5700,    5000,    4300,    3900,    3400,    3000,    2700,   2500,   //80%              /   12
	10500,     8200,     6800,     5500,    4800,    4000,    3500,    3100,    2800,    2400,    2100,    1900,   1800,   //85%              /   13
	7100,     5300,     4700,     4000,    3300,    2800,    2500,    2200,    2000,    1800,    1550,    1400,   1300    //90%              /   14
};


uint16_t doc_adc(uint8_t channel)
{
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}
void get_temperature()
{
	data_adc = doc_adc(nhiet_do);
	data_adc=(uint16_t)((float)data_adc * (float)0.489);
	sensor = data_adc;
	if (data_adc >= 100)
	{
		mang[0]=(char)data_adc/100+48;
		data_adc%=100;
		mang[1]=(char)data_adc/10+48;
		mang[2]=(char)data_adc%10+48;
	}
	else
	{
		mang[0]=(char)data_adc/10+48;
		mang[1]=(char)data_adc%10+48;
	}
}
void get_gas()
{
	data_adc = doc_adc(nong_do_khoi);
	data_adc=(uint16_t)((float)data_adc * (float)100/(float)1024);
	sensor = data_adc;
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
uint8_t binary_search_impedance_zone(uint32_t value, uint8_t t_zone)
{
	uint8_t h = 14;
	uint8_t l = 0;
	uint8_t m = (h / 2);
	
	while(l <= h)
	{
		if(value < (impedance_ohms_table[m][t_zone]))
		{
			l = (m + 1);
		}
		
		else
		{
			h = (m - 1);
		}
		
		m = ((h + l) / 2);
		
	}
	
	return ((uint8_t)m);
}

void get_humidity()
{
	uint8_t humidity;
	uint8_t temp = sensor;
	uint32_t impedance = 0;
	temp /= 5;
	data_adc = doc_adc(do_am);
	impedance = 10000/((1023/(data_adc)-1));
	humidity = binary_search_impedance_zone(impedance,(uint8_t)temp)*5 + 20;
	if(humidity >= 100)
	{
		mang[0]=humidity/100+48;
		humidity%=100;
		mang[1]=humidity/10+48;
		mang[2]=humidity%10+48;
	}
	else
	{
		mang[0]=humidity/10+48;
		mang[1]=humidity%10+48;
	}
}

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
	bool fire_hazard = false;
	bool overheated = false;
	bool high_gas = false;
	
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS1)|(1<<ADPS0);
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
		memset(mess, 0, 100);
		get_temperature();
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
		get_humidity();
		getmess(do_am);
		memset(mang,0,50);
		get_gas();
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
		memset(mang,0,50);
		
		/*		send data on thingsboard		*/
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
