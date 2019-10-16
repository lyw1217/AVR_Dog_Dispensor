/*
 * main.h
 *
 * Created: 2019-08-20 오후 7:02:00
 *  Author: LYW
 */ 

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "button.h"
#include "buzzer.h"
#include "DS1302.h"
#include "I2C_LCD.h"
#include "servo.h"
#include "time.h"
#include "HC_06.h"
#include "DHT.h"

enum {CLOCK, SHOW_TH, TIME_SETTING, FEEDING_SETTING, FEEDING, SHOW_SETTING, SERVO_SETTING} RUN_STATE;

// quantity unit(per a time)
#define UNIT	5
// blutooth STATE PIN
#define STATE_DDR	DDRG
#define STATE_PIN	PING
#define HC06_STATE	PING4

#define RELAY_DDR	DDRG
#define RELAY_PORT	PORTG
#define RELAY_SIG	PORTG1

void print_Menu();
void show_Clock();
uint8_t set_State();
void feeding(uint8_t _quantity, uint8_t _servo_degree);
void BT_transmit_TH();
void set_DS1302();
void set_Feeding();
void feeding_at_time();
void show_Set();

#endif /* MAIN_H_ */