/*
 * servo.h
 *
 * Created: 2019-07-10 오후 3:05:09
 *  Author: kccistc
 */ 


#ifndef SERVO_H_
#define SERVO_H_

#define F_CPU 16000000UL

#define SERVO_DDR	DDRB
#define SERVO_SIG	DDRB5

#include <avr/io.h>

void servo_Init();
void servo_Run(uint8_t degree);

#endif /* SERVO_H_ */