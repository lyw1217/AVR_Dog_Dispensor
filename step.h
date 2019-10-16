/*
 * step.h
 *
 * Created: 2019-08-26 오후 5:14:24
 *  Author: LYW
 */ 


#ifndef STEP_H_
#define STEP_H_

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#define DDR_STEP	DDRC
#define PORT_STEP	PORTC
#define STEP_A0		PORTC0
#define STEP_A1		PORTC1
#define STEP_B0		PORTC2
#define STEP_B1		PORTC3

void step_Init();
void step_Forward(uint32_t _degree);
void step_Backward(uint32_t _degree);

#endif /* STEP_H_ */