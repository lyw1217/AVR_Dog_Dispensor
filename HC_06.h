/*
 * HC_06.h
 *
 * Created: 2019-08-22 오후 8:49:14
 *  Author: LYW
 */ 


#ifndef HC_06_H_
#define HC_06_H_

#define F_CPU 16000000UL

#include <avr/io.h>

void BT_UART1_Init();
void BT_UART1_transmit(char data);
unsigned char BT_UART1_receive(void);
void BT_UART1_printf_string(char *str);
uint8_t BT_isRxD();
uint8_t BT_isRxString();
uint8_t* BT_getRxString();
void BT_UART1_ISR_Receive();


#endif /* HC-06_H_ */