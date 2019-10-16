/*
 * buzzer.c
 *
 * Created: 2019-07-02 오후 5:16:08
 *  Author: LYW
 */ 

#include "buzzer.h"

void buzzer_Init(){
	DDRE |= (1 << PORTE3);
	
	TCCR3B |= (0<<CS32) | (1<<CS31) | (0<<CS30); // 분주비 8
	// CTC Mode
	TCCR3B |= (0<<WGM33) | (1<<WGM32);
	TCCR3B |= (0<<WGM31) | (0<<WGM31);
	// Toggle 출력
	TCCR3A |= (0<<COM3A1) | (1<<COM3A0);
	OCR3A = 1000;
}

/*
/ 주파수 값 입력 설정
/ ex) 1kHz 출력 : setSound(1000);
*/

void setBuzzer(int note){
	int ocr_value = 1000000 / note; // == ( F_CPU / 2 / PRESCALER / note )
	OCR3A = ocr_value;
}

void stopBuzzer(){
	TCCR3A &= ~((1<<COM3A1) | (1<<COM3A0));
}

void playBuzzer(){
	TCCR3A |= (0<<COM3A1) | (1<<COM3A0);
}

void powerOnBuzzer(){
	playBuzzer();
	setBuzzer(C4);
	_delay_ms(300);
	setBuzzer(E4);
	_delay_ms(300);
	setBuzzer(G4);
	_delay_ms(300);
	stopBuzzer();
}

void buttonBuzzer(){
	playBuzzer();
	setBuzzer(C5);
	_delay_ms(50);
	setBuzzer(F4);
	_delay_ms(50);
	stopBuzzer();
}