/*
 * adc.c
 *
 * Created: 2019-07-10 오후 12:42:04
 *  Author: LYW
 */ 

//FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);	// output을 UART0_transmit()이라는 함수를 불러서 사용하겠다
//FILE INPUT = FDEV_SETUP_STREAM(NULL, UART0_receive, _FDEV_SETUP_READ);
//stdout = &OUTPUT;	// 표준 출력을 어디에다 할 것인지 주소를 주는 것
//stdin = &INPUT;

#include "adc.h"

void ADC_init(unsigned char channel){
	ADMUX |= (1 << REFS0);		// AVCC를 기준전압으로 선택
	ADCSRA |= 0X07;				// 분주비 설정
	ADCSRA |= (1 << ADEN);		// ADC 활성화
	ADCSRA |= (1 << ADFR);		// 프리러닝 모드
	
	ADMUX = ((ADMUX & 0xE0) | channel);	// 채널 선택 -> ADMUX bit 1110 0000 중 111 부분은 유지를 하고 0~4번 bit만 채널을 선택하겠다.
	ADCSRA |= (1 << ADSC);		// 변환 시작, 프리러닝 모드니 계속 실행
}

int read_ADC(){
	while(!(ADCSRA & (1 << ADIF)));	// 변환 종료 대기
	
	return ADC;					// 10비트 값을 반화
}

void ADC_init_single(){
	ADMUX |= (1 << REFS0);		// AVCC를 기준전압으로 선택
	ADCSRA |= 0X07;				// 분주비 설정
	ADCSRA |= (1 << ADEN);		// ADC 활성화
	ADCSRA |= (0 << ADFR);		// 단일 변환 모드
}

int read_ADC_single(unsigned char channel){
	ADMUX = ((ADMUX & 0xE0) | channel);	// 채널 선택 -> ADMUX bit 1110 0000 중 111 부분은 유지를 하고 0~4번 bit만 채널을 선택하겠다.
	ADCSRA |= (1 << ADSC);
	
	while(!(ADCSRA & (1 << ADIF)));	// 변환 종료 대기

	return ADC;					// 10비트 값을 반화
}