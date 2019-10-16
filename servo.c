/*
* servo.c
*
* Created: 2019-07-10 오후 3:04:54
*  Author: kccistc
*/

#include "servo.h"

void servo_Init(){
	// PE 3번핀에 Servo모터 제어핀 연결
	//SERVO_DDR |= (1 << SERVO_SIG);
	// 서보모터 헌팅현상을 막기 위해 서보모터 동작 전/후 ON/OFF
	
	// 20ms 주기를 만들기 위해 분주비 64, TOP 5000의 고속 PWM 설정
	TCCR1A |= (1 << WGM11) | (0 << WGM10);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);	// 모드 14, 고속 PWM
	
	// TOP : ICR1, 비교일치값: OCR1A 레지스터
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0);	// 비반전모드
	TCCR1B |= (1 << CS11) | (1 << CS10);		// 분주율 64
	
	ICR1 = 5000 - 1;	// 20ms (20ms / 1/(F_CPU/분주율) = 5000)
}

void servo_Run(uint8_t degree){
	
	uint16_t degValue = 0 ;
	
	degValue = (degree / 180.0) * 475 + 125;
	// SERVO 모터마다 값이 다름.
	//OCR3A = 125 - 1; // 0도
	//OCR3A = 375 - 1; // 90도
	//OCR3A = 625 - 1; // 180도
	
	OCR1A = degValue - 1;
}