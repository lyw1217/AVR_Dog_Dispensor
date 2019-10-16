/*
* DS1302.h
*
* Created: 2019-07-11 오후 2:09:03
*  Author: LYW
*/


#ifndef DS1302_H_
#define DS1302_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DS1302_CLK_DDR		DDRA
#define DS1302_CLK_PORT		PORTA
#define DS1302_DAT_DDR		DDRA
#define DS1302_DAT_PORT		PORTA
#define DS1302_DAT_PIN		PINA
#define DS1302_RST_DDR		DDRA
#define DS1302_RST_PORT		PORTA
#define DS1302_CLK			0
#define DS1302_DAT			1
#define DS1302_RST			2

#define ADDRESS_SECONDS		0x80
#define ADDRESS_MINUTES		0x82
#define ADDRESS_HOUR		0x84
#define ADDRESS_DATE		0x86
#define ADDRESS_MONTH		0x88
#define ADDRESS_DAYOFWEEK	0x8a
#define ADDRESS_YEAR		0x8c

typedef struct _ds1302{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t date;
	uint8_t month;
	uint8_t dayofweek; // sum = 1, mon = 2 ... sat = 7
	uint8_t year;
	uint8_t ampm; // pm = 0 , am = 1
	uint8_t hourMode; // 24h = 0, 12h = 1

}DS1302;

extern struct _ds1302 stTime;
extern char *dayofweek[7];

void DS1302_Init();
void DS1302_Selected();
void DS1302_Deselected();
void DS1302_Clock();
void DS1302_DataBitSet();
void DS1302_DataBitReset();
uint8_t DS1302_Read_DataPin();
void DS1302_Change_ReadMode();
void DS1302_Change_WriteMode();
uint8_t decimal_to_bcd(uint8_t decimal);
uint8_t bcd_to_decimal(uint8_t bcd);
void DS1302_TxData(uint8_t _txData);
uint8_t DS1302_RxData();
void DS1302_WriteData(uint8_t address, uint8_t data);
uint8_t DS1302_ReadData(uint8_t address);
void DS1302_GetTime(DS1302 *timeData);
void DS1302_GetDate(DS1302 *dateData);
void DS1302_SetTimeDates(DS1302 timeDate);

#endif /* DS1302_H_ */