/*
 * HC_06.c
 *
 * Created: 2019-08-22 오후 8:48:37
 *  Author: LYW
 */ 

#include "HC_06.h"

volatile uint8_t BT_rxString[64] = {0};
volatile uint8_t BT_rxReadyFlag = 0;

void BT_UART1_Init(){
	
	/* Enable receiver and transmitter */
	UCSR1B |= (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
	// RXCIE0 => 수신 인터럽트 사용(USART0_RX_vect)
	UCSR1A |= (1 << U2X1); // 2배속 모드 설정
	// UCSR0A |= _BV(U2X1); 2배속 모드 설정 매크로 사용
	// UCSR0C 대부분 기본 설정 사용 비동기, 8bit 데이터, no parity, 1비트 정지
	
	UBRR1H = 0;
	UBRR1L = 207;	// 9600 baud -> p.220 참고
}

void BT_UART1_transmit(char data)
{
	while ( !(UCSR1A & (1<<UDRE1)) );
	UDR1 = data;
}

unsigned char BT_UART1_receive(void)
{
	while ( !(UCSR1A & (1<<RXC1)) );
	return UDR1;
}

void BT_UART1_printf_string(char *str)
{
	for (int i=0; str[i]; i++)
	BT_UART1_transmit(str[i]);
}

uint8_t BT_isRxD(){
	return (UCSR1A & (1<<7));
}

uint8_t BT_isRxString(){ // 들어온 String이 있는가?	
	return BT_rxReadyFlag;
}

uint8_t* BT_getRxString(){ // 리턴자료형 뒤에 *를 붙이면 반환되는 값이 주소임을 알려줌
	BT_rxReadyFlag = 0;
	return BT_rxString;
	// return %rxString[0]; 같은 의미, rxString의 첫 번째 요소의 주소를 반환
}

void BT_UART1_ISR_Receive(){
	static uint8_t BT_head = 0;
	volatile uint8_t BT_data;
	
	BT_data = UDR1; // UDRn : 송수신된 데이터가 저장되는 버퍼 레지스터
	
	// 만약 data 문자열의 끝을 만나면 끝에 NULL을 넣어주겠다.
	if((BT_data == '\n') || (BT_data == '\r')){
		BT_rxString[BT_head]	= '\0';
		BT_head = 0;
		BT_rxReadyFlag = 1;
	}
	// 문자열의 끝이 아니라면(문자가 들어올때마다) rxString[head]에 data 문자를 넣겠다.
	else{
		BT_rxString[BT_head] = BT_data;
		BT_head++;
	}
}