/*
 * button.h
 *
 * Created: 2019-07-02 오후 6:45:09
 *  Author: kccistc
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define DDR_BUTTON	DDRG
#define PIN_BUTTON	PING
#define button_1	PING3

void button_Init();
uint8_t button1_State();



#endif /* BUTTON_H_ */