/*
 * uart.c
 *
 * Created: 2019-07-03 오전 10:26:26
 *  Author: LYW
 */ 

#include "uart0.h"

volatile uint8_t rxString[64] = {0};
volatile uint8_t rxReadyFlag = 0;

void UART0_Init(){
	
	/* Enable receiver and transmitter */
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	// RXCIE0 => 수신 인터럽트 사용(USART0_RX_vect)
	UCSR0A |= (1 << U2X1); // 2배속 모드 설정
	// UCSR0A |= _BV(U2X1); 2배속 모드 설정 매크로 사용
	// UCSR0C 대부분 기본 설정 사용 비동기, 8bit 데이터, no parity, 1비트 정지
	
	UBRR0H = 0;
	UBRR0L = 207;	// 9600 baud -> p.220 참고
}

void UART0_transmit(char data)
{
	while ( !(UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}

unsigned char UART0_receive(void)
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

void UART0_printf_string(char *str)
{
	for (int i=0; str[i]; i++)
	UART0_transmit(str[i]);
}

uint8_t isRxD(){
	return (UCSR0A & (1<<7));
}

uint8_t isRxString(){ // 들어온 String이 있는가?
	return rxReadyFlag;
}

uint8_t* getRxString(){ // 리턴자료형 뒤에 *를 붙이면 반환되는 값이 주소임을 알려줌
	rxReadyFlag = 0;
	return rxString;
	// return %rxString[0]; 같은 의미, rxString의 첫 번째 요소의 주소를 반환
}

void UART0_ISR_Receive(){
	static uint8_t head = 0;
	volatile uint8_t data;
	
	data = UDR0; // UDRn : 송수신된 데이터가 저장되는 버퍼 레지스터
	
	// 만약 data 문자열의 끝을 만나면 끝에 NULL을 넣어주겠다.
	if((data == '\n') || (data == '\r')){
		rxString[head]	= '\0';
		head = 0;
		rxReadyFlag = 1;
	}
	// 문자열의 끝이 아니라면(문자가 들어올때마다) rxString[head]에 data 문자를 넣겠다.
	else{
		rxString[head] = data;
		head++;
	}
}