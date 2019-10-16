/*
 * step.c
 *
 * Created: 2019-08-26 오후 5:14:14
 *  Author: LYW
 */ 
#include "step.h"

// 1상여자방식 Full Step
//uint8_t step_data[] = {0x01, 0x02, 0x04, 0x08};
// 2상여자방식 Full Step
uint8_t step_data[] = { (1<<STEP_A0) | (1<<STEP_B1), (1<<STEP_A0) | (1<<STEP_A1),
							(1<<STEP_A1) | (1<<STEP_B0), (1<<STEP_B0) | (1<<STEP_B1) };

void step_Init(){
	DDR_STEP |= (1<<STEP_A0) | (1<<STEP_A1) | (1<<STEP_B0) | (1<<STEP_B1);
}

void step_Forward(uint32_t _degree){
	int8_t step_index = -1;
	_degree = (_degree * 2050) / 360;
	for(int i = 0; i < _degree; i++){
		step_index++;
		if(step_index >=  4) step_index = 0;
		PORT_STEP = step_data[step_index];
		
		_delay_ms(2);
	}
}
void step_Backward(uint32_t _degree){
	int8_t step_index = -1;
	_degree = (_degree * 2050) / 360;
	for(int i = 0; i < _degree; i++){
		step_index--;
		if(step_index < 0) step_index = 3;
		PORT_STEP = step_data[step_index];
		
		_delay_ms(2);
	}
}