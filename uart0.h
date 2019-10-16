/*
 * uart.h
 *
 * Created: 2019-07-03 오전 10:26:51
 *  Author: LYW
 */ 


#ifndef UART0_H_
#define UART0_H_

#define F_CPU 16000000UL

#include <avr/io.h>

void UART0_Init();
void UART0_transmit(char data);
unsigned char UART0_receive(void);
void UART0_printf_string(char *str);
uint8_t isRxD();
uint8_t isRxString();
uint8_t *getRxString();
void UART0_ISR_Receive();

#endif /* UART0_H_ */