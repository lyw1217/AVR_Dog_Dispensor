/*
 * buzzer.h
 *
 * Created: 2019-07-02 오후 5:16:18
 *  Author: LYW
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define C4	262
#define D4	294
#define E4	330
#define F4	349
#define G4	392
#define A4	440
#define B4	494
#define C5	523 // Hz

void buzzer_Init();
void setBuzzer(int note);
void stopBuzzer();
void playBuzzer();
void powerOnBuzzer();
void buttonBuzzer();

#endif /* BUZZER_H_ */