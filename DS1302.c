/*
 * DS1302.c
 *
 * Created: 2019-07-11 오후 2:08:55
 *  Author: LYW
 */ 

#include "DS1302.h"

struct _ds1302 stTime;
// DS1302 stTime; 위와 같은 의미

// 포인터 배열 : 배열에 들어가는 값은 주소
char *dayofweek[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

void DS1302_Init(){
	DS1302_CLK_DDR |= (1<<DS1302_CLK);
	DS1302_DAT_DDR |= (1<<DS1302_DAT);
	DS1302_RST_DDR |= (1<<DS1302_RST);
}

void DS1302_Selected(){
	// CE High
	DS1302_RST_PORT |= (1<<DS1302_RST);
}
void DS1302_Deselected(){
	// CE Low
	DS1302_RST_PORT &= ~(1<<DS1302_RST);
}

void DS1302_Clock(){
	DS1302_CLK_PORT &= ~(1<<DS1302_CLK);	// clk 1 -> h -> 1
	DS1302_CLK_PORT |= (1<<DS1302_CLK);
	DS1302_CLK_PORT &= ~(1<<DS1302_CLK);
}

void DS1302_DataBitSet(){
	DS1302_DAT_PORT |= (1<<DS1302_DAT);	// data high로 설정
}

void DS1302_DataBitReset(){
	DS1302_DAT_PORT &= ~(1<<DS1302_DAT);	// data low로 설정
}

uint8_t DS1302_Read_DataPin(){
	return (DS1302_DAT_PIN & (1 << DS1302_DAT));
}

void DS1302_Change_ReadMode(){
	DS1302_DAT_DDR &= ~(1<<DS1302_DAT);	// read mode
}

void DS1302_Change_WriteMode(){
	DS1302_DAT_DDR |= (1<<DS1302_DAT);	// write mode
}

uint8_t decimal_to_bcd(uint8_t decimal){
	return ( ((decimal/10) << 4) | (decimal % 10) );
}

uint8_t bcd_to_decimal(uint8_t bcd){
	return ( ((bcd >> 4) * 10) + (bcd & 0x0f) );
}

void DS1302_TxData(uint8_t _txData){
	DS1302_Change_WriteMode();
	
	for(int i = 0; i < 8; i++){
		if (_txData & (1<<i))
		DS1302_DataBitSet();
		else
		DS1302_DataBitReset();
		
		// 클럭을 올려줬다가 내려준다 이것을 8번 반복
		DS1302_Clock();
	}
}

uint8_t DS1302_RxData(){
	uint8_t rxData = 0;
	DS1302_Change_ReadMode();
	
	for(int i = 0; i < 8; i++){
		// data = A ? b : c; A가 참이면 b 반환, A가 거짓(0이 아니면)이면 c 반환
		rxData |= DS1302_Read_DataPin() ? (1<<i) : 0;
		
		// 클럭을 올려줬다가 내려준다 이것을 8번 반복
		DS1302_Clock();
	}
	return rxData;
}

void DS1302_WriteData(uint8_t address, uint8_t data){
	DS1302_Selected();	// CE 활성화
	DS1302_TxData(address);	// 주소를 보내고
	DS1302_TxData(decimal_to_bcd(data));	// 보내는 포멧이 bcd 이기 때문
	DS1302_Deselected();	// CE 비활성화
}

uint8_t DS1302_ReadData(uint8_t address){
	uint8_t rxData = 0;
	DS1302_Selected();
	DS1302_TxData(address+1);	// 주소를 읽어오고 // read 주소가 write 주소보다 1 더 많기 때문에 +1
	rxData = DS1302_RxData();
	DS1302_Deselected();
	
	return bcd_to_decimal(rxData);	
}
	
void DS1302_GetTime(DS1302 *timeData){
	timeData->seconds = DS1302_ReadData(ADDRESS_SECONDS); // datasheet 참고
	timeData->minutes = DS1302_ReadData(ADDRESS_MINUTES);
	timeData->hour = DS1302_ReadData(ADDRESS_HOUR);
}

void DS1302_GetDate(DS1302 *dateData){
	dateData->date = DS1302_ReadData(ADDRESS_DATE); // datasheet 참고
	dateData->month = DS1302_ReadData(ADDRESS_MONTH);
	dateData->dayofweek = DS1302_ReadData(ADDRESS_DAYOFWEEK);
	dateData->year = DS1302_ReadData(ADDRESS_YEAR);
}

/* 위의 구조체를 사용한 코드와 비슷한 의미(참고용)
void DS1302_GetTime(uint8_t *sec, uint8_t *min, uint8_t *hour){
	*sec = DS1302_ReadData(0x81); // datasheet 참고
	*min = DS1302_ReadData(0x83);
	*hour = DS1302_ReadData(0x85);
}

void DS1302_GetDate(uint8_t *date, uint8_t *month, uint8_t *day, uint8_t dayofweek, uint8_t year){
	*date = DS1302_ReadData(0x87); // datasheet 참고
	*month = DS1302_ReadData(0x89);
	*day = DS1302_ReadData(0x8b);
	*dayofweek = DS1302_ReadData(0x85);
	*year = DS1302_ReadData(0x8d);
}
*/

void DS1302_SetTimeDates(DS1302 timeDate){
	DS1302_WriteData(ADDRESS_SECONDS, timeDate.seconds);
	DS1302_WriteData(ADDRESS_MINUTES, timeDate.minutes);
	DS1302_WriteData(ADDRESS_HOUR, timeDate.hour);
	DS1302_WriteData(ADDRESS_DATE, timeDate.date);
	DS1302_WriteData(ADDRESS_MONTH, timeDate.month);
	DS1302_WriteData(ADDRESS_DAYOFWEEK, timeDate.dayofweek);
	DS1302_WriteData(ADDRESS_YEAR, timeDate.year);
}