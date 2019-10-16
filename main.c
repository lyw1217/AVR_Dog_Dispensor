/*
* Dog_Dispenser.c
*
* Created: 2019-08-20 오후 6:57:27
* Author : LYW
*/

#include "main.h"

ISR(USART1_RX_vect){
	BT_UART1_ISR_Receive();
	if((UDR1 == '\n') || (UDR1 == '\r')){
		_delay_ms(2);
	}
}

ISR(TIMER0_COMP_vect){
	incMilliSec();
	incTime();
}

// output을 UART1_transmit()이라는 함수를 불러서 사용하겠다
FILE OUTPUT = FDEV_SETUP_STREAM(BT_UART1_transmit, NULL, _FDEV_SETUP_WRITE);

//////////////////////////////////////////////
//---------- 전역 변수 선언부 시작 ----------//
uint8_t num = 1;
uint8_t times = 0;
uint8_t quantity = 0;
uint8_t settingFlag = 0;
// 블루투스로 1회만 출력하기 위한 flag
uint8_t input_flag = 0;
// settingState 0 ~ 5 시간 및 날짜 셋팅
uint8_t settingState = 0;
// 깜빡임을 주기 위한 flag
uint8_t blink_flag = 0;
uint8_t connect_state = 0;
uint8_t feeding_flag = 0;

// 하루에 급식할 시간, 분을 설정해줄 때마다 set_count 증가
uint8_t set_count = 0;
// strtok 사용을 위한 포인터
char* time_Tokken;

char buff[20], time_buff[20], date_buff[20], bt_buff[40];

//uint8_t** feed_time;
uint8_t feed_time[6][2] = {0,};

uint32_t prevMillis;

uint8_t *receiveData;

uint8_t dht_count = 0;
uint8_t temp[2] = {0,};
uint8_t humi[2] = {0,};

uint8_t servo_degree = 200;
//---------- 전역 변수 선언부  끝 ----------//
////////////////////////////////////////////




////////////////////////////////////////
//---------- main 함수 시작 ----------//
int main(void)
{
	// HC-06 STATE 입력
	STATE_DDR &= ~(1 << HC06_STATE);
	// 절전모드 위한 릴레이
	RELAY_DDR |= (1 << RELAY_SIG);
	
	// 외부 9전원 ON
	RELAY_PORT |= (1 << RELAY_SIG);
	
	I2C_LCD_init();
	I2C_LCD_write_string_XY(0, 0, "INITIALIZING....");
	
	DHT_Setup();
	
	BT_UART1_Init();
	
	DS1302_Init();
	timer0_Init();
	
	servo_Init();
	servo_Run(0);
	
	button_Init();
	
	sei();
	stdout = &OUTPUT;
	
	prevMillis = millis();
	
	buzzer_Init();
	powerOnBuzzer();
	
	DHT_Read(temp, humi);
	
	RUN_STATE = CLOCK;
	
	I2C_LCD_clear();
	
	//---------- 루프 시작 ----------//
	while (1)
	{
		// 블루투스와 스마트폰 연결되었을 때 메뉴 출력
		if( ( (STATE_PIN & (0x01 << HC06_STATE)) == (0x01 << HC06_STATE)) && (connect_state == 0) ){
			print_Menu();
			buttonBuzzer();
			connect_state = 1;
			}else if( ( (STATE_PIN & (0x01 << HC06_STATE)) == 0) ){
			connect_state = 0;
		}
		
		if(BT_isRxString() && (settingFlag == 0)){
			receiveData = BT_getRxString();
			
			num = atoi(receiveData);
			RUN_STATE = set_State();
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
		}
		
		// 버튼 1번 눌리면 1회 수동 급여
		if(button1_State()){
			RUN_STATE = FEEDING;
		}
		
		switch(RUN_STATE){
			case CLOCK:
			show_Clock();
			feeding_at_time();
			break;
			
			case SHOW_TH:
			BT_transmit_TH();
			RUN_STATE = CLOCK;
			break;
			
			case TIME_SETTING:
			if(settingFlag == 1) set_DS1302();
			break;
			
			case FEEDING_SETTING:
			if(settingFlag == 1) set_Feeding();
			break;
			
			case SHOW_SETTING:
			show_Set();
			RUN_STATE = CLOCK;
			break;
			
			case SERVO_SETTING:
			if(settingFlag == 1) servo_Set();
			break;
			
			case FEEDING:
			feeding(1, servo_degree);
			BT_UART1_printf_string("Manual Feeding Complete!\n");
			RUN_STATE = CLOCK;
			break;
		}
	}
	//---------- 루프 끝 ----------//
	return 0;
}
//---------- main 함수 끝 ----------//
/////////////////////////////////////



/////////////////////////////////////
//---------- 함수 선언부 ----------//
void servo_Set(){
	if(input_flag == 0){
		BT_UART1_printf_string("\n---- SERVO SETTING ----\n>");
		BT_UART1_printf_string("\nInput Servo degree. (default : 100)\n>");
		input_flag = 1;
	}
	
	if(millis() - prevMillis > 500){
		if (blink_flag == 0){
			I2C_LCD_write_string_XY(0, 0, "||SERVO DEGREE||");
			I2C_LCD_write_string_XY(1, 0, "  degree : ---  ");
			blink_flag = 1;
		}
		else{
			I2C_LCD_write_string_XY(1, 0, "  degree :      ");
			blink_flag = 0;
		}
		prevMillis = millis();
	}
	
	if(BT_isRxString()){
		receiveData = BT_getRxString();
		BT_UART1_printf_string("\n");
		
		servo_degree = atoi(receiveData);
		
		if(servo_degree < 0 || servo_degree > 255){
			BT_UART1_printf_string("It is possible to insert degree from 0 to 255.\n");
			return;
		}
		
		sprintf(bt_buff, "Set %d\n\n", servo_degree);
		BT_UART1_printf_string(bt_buff);
		print_Menu();
		
		input_flag = 0;
		blink_flag = 0;
		settingFlag = 0;
		RUN_STATE = CLOCK;
		// 딜레이를 주지 않으면 블루투스가 잘 안됨
		_delay_ms(2);
	}
}


void print_Menu(){
	BT_UART1_printf_string("\n---------------------- M E N U -------------------\n");
	BT_UART1_printf_string("   1. Show Current Temperature & Humidity\n");
	BT_UART1_printf_string("   2. Set Current Time & Date\n");
	BT_UART1_printf_string("   3. Set Feeding Period\n");
	BT_UART1_printf_string("   4. Show Setting\n");
	BT_UART1_printf_string("   5. Feeding Once\n");
	BT_UART1_printf_string("   6. Servo Setting\n");
	BT_UART1_printf_string("---------------------------------------------------\n");
	BT_UART1_printf_string("Select Number >\n");
}

void show_Clock(){
	DS1302_GetTime(&stTime);
	DS1302_GetDate(&stTime);
	
	if(millis() - prevMillis > 200){
		// 초기 temp가 측정이 안되어있는 경우를 위해 temp[0]==0 일 때 온도 읽어오기
		// 약 10초에 한 번 온도 측정
		dht_count++;
		if((dht_count % 40) || (temp[0] == 0)){
			DHT_Read(temp, humi);
			dht_count = 0;
		}
		
		sprintf(date_buff, "%04d.%02d.%02d (%s)", 2000+stTime.year, stTime.month, stTime.date, dayofweek[stTime.dayofweek]);
		sprintf(time_buff, "%02d:%02d:%02d  %02d.%01dC", stTime.hour, stTime.minutes, stTime.seconds, temp[0], temp[1]);
		I2C_LCD_write_string_XY(0, 0, date_buff);
		I2C_LCD_write_string_XY(1, 0, time_buff);
		prevMillis = millis();
	}
}

uint8_t set_State(){
	I2C_LCD_clear();
	switch(num){
		case 1:
		return SHOW_TH;
		break;
		
		case 2:
		settingFlag = 1;
		return TIME_SETTING;
		break;
		
		case 3:
		settingFlag = 1;
		return FEEDING_SETTING;
		break;
		
		case 4:
		return SHOW_SETTING;
		break;
		
		case 5:
		return FEEDING;
		break;
		
		case 6:
		settingFlag = 1;
		return SERVO_SETTING;
		break;
		
		default:
		BT_UART1_printf_string("Please, Press the Number from 1 to 4\n");
		print_Menu();
		return CLOCK;
		break;
	}
}

void feeding(uint8_t _quantity, uint8_t _servo_degree){
	for(int i = 0; i < _quantity; i++){
		// 서보모터 헌팅현상을 막기 위해 서보모터 동작 전/후 ON/OFF
		SERVO_DDR |= (1 << SERVO_SIG);
		servo_Run(_servo_degree);
		_delay_ms(700);
		servo_Run(0);
		_delay_ms(700);
		SERVO_DDR &= ~(1 << SERVO_SIG);
		buttonBuzzer();
	}
}

void BT_transmit_TH(){
	if(temp[0] == 0 || humi[0] == 0){
		BT_UART1_printf_string("Please wait a moment for it to be measured.\n");
	}
	else{
		sprintf(bt_buff,"Current Temp = %02d.%01d℃ , Humi = %02d.%01d", temp[0], temp[1], humi[0], humi[1]);
		BT_UART1_printf_string(bt_buff);
		BT_UART1_printf_string("%\n");
	}
}

void set_DS1302(){
	// DS1302 세팅
	switch (settingState){
		//---------- 날짜 설정 시작 ----------//
		case 0:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Year.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				I2C_LCD_write_string_XY(0, 0, "||DATE SETING|| ");
				I2C_LCD_write_string_XY(1, 0, "   20YY.MM.DD   ");
				blink_flag = 1;
			}
			else{
				I2C_LCD_write_string_XY(1, 0, "   20--.MM.DD   ");
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.year = atoi(receiveData);
			
			input_flag = 0;
			blink_flag = 0;
			settingState = 1;
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
		}
		break;
		
		
		case 1:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Month.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				sprintf(buff, "   20%02d.MM.DD   ", stTime.year);
				I2C_LCD_write_string_XY(0, 0, "||DATE SETING|| ");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				sprintf(buff, "   20%02d.--.DD   ", stTime.year);
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.month = atoi(receiveData);
			
			input_flag = 0;
			blink_flag = 0;
			settingState = 2;
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
		}
		break;
		
		
		case 2:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Date.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				sprintf(buff, "   20%02d.%02d.DD   ", stTime.year, stTime.month);
				I2C_LCD_write_string_XY(0, 0, "||DATE SETING|| ");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				sprintf(buff, "   20%02d.%02d.--   ", stTime.year, stTime.month);
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.date = atoi(receiveData);
			// 요일 계산
			stTime.dayofweek = getDayofWeek((2000+stTime.year), stTime.month, stTime.date);
			
			input_flag = 0;
			blink_flag = 0;
			settingState = 3;
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
			
		}
		break;
		//---------- 날짜 설정 끝 ----------//
		//---------- 시간 설정 시작 ----------//
		case 3:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Hour.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				I2C_LCD_write_string_XY(0, 0, "<<TIME SETING>> ");
				I2C_LCD_write_string_XY(1, 0, "    HH:MM:SS    ");
				blink_flag = 1;
			}
			else{
				I2C_LCD_write_string_XY(1, 0, "    --:MM:SS    ");
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.hour = atoi(receiveData);
			
			input_flag = 0;
			blink_flag = 0;
			settingState = 4;
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
		}
		break;
		
		
		case 4:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Minute.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				sprintf(buff, "    %02d:MM:SS    ", stTime.hour);
				I2C_LCD_write_string_XY(0, 0, "<<TIME SETING>> ");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				sprintf(buff, "    %02d:--:SS    ", stTime.hour);
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.minutes = atoi(receiveData);
			
			input_flag = 0;
			blink_flag = 0;
			settingState = 5;
			
			// 딜레이를 주지 않으면 블루투스가 잘 안됨
			_delay_ms(2);
		}
		break;
		
		
		case 5:
		if(input_flag == 0){
			BT_UART1_printf_string("\nInput Current Second.\n>");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				sprintf(buff, "    %02d:%02d:SS    ", stTime.hour, stTime.minutes);
				I2C_LCD_write_string_XY(0, 0, "<<TIME SETING>> ");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				sprintf(buff, "    %02d:%02d:--    ", stTime.hour, stTime.minutes);
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			stTime.seconds = atoi(receiveData);
			DS1302_SetTimeDates(stTime);
			
			I2C_LCD_clear();
			BT_UART1_printf_string("SETTING COMPLETE\n\n");
			I2C_LCD_write_string_XY(0, 0, "SETTING COMPLETE");
			_delay_ms(1000);
			
			// 전역 변수 초기화 및 시간 표시 상태 이동
			settingState = 0;
			blink_flag = 0;
			settingFlag = 0;
			input_flag = 0;
			RUN_STATE = CLOCK;
			print_Menu();
		}
		break;
		//---------- 시간 설정 끝 ----------//
	}
}



void set_Feeding(){
	static uint8_t duplicate_flag = 0;
	
	switch(settingState){
		case 0:
		if(input_flag == 0){
			BT_UART1_printf_string("---------- Set Feeding Period ----------\n\n");
			BT_UART1_printf_string("Step 1. How many times a day?\n");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				I2C_LCD_write_string_XY(0, 0, "How many times  ");
				sprintf(buff, "a day?   0 times");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				I2C_LCD_write_string_XY(0, 0, "How many times  ");
				sprintf(buff, "a day?   - times");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			
			times = atoi(receiveData);
			
			// 하루에 6회 초과는 못하게 설정(2차원 메모리 동적할당 위험)
			// 새끼 강아지가 하루에 최대 6회정도,
			if(times > 6 || times <= 0){
				BT_UART1_printf_string("It is possible to insert times from 1 to 6.\n");
				input_flag = 0;
				break;
			}
			
			// times만큼 동적할당한 2차원 배열 feed_time[times][2]에 시간, 분 값을 넣어야함
			/*
			// 동적 할당하면 4번째 입력부터 에러가 남
			feed_time = (uint8_t**)malloc(sizeof(uint8_t) * times);
			if(feed_time == NULL){ // 오류 처리
			BT_UART1_printf_string("FAIL!! Please try again");
			break;
			}
			for(int i=0; i < times; i++){
			feed_time[i] = (uint8_t*)malloc(sizeof(uint8_t) * 2);
			}
			*/
			
			sprintf(buff, "a day?   %01d times", times);
			I2C_LCD_write_string_XY(1, 0, buff);
			sprintf(buff, "%01d times a day\n\n", times);
			BT_UART1_printf_string(buff);
			_delay_ms(500);
			
			blink_flag = 0;
			input_flag = 0;
			settingState = 1;
		}
		break;
		
		
		case 1:
		if(input_flag == 0){
			BT_UART1_printf_string("Step 2. What time should I feed?\n");
			BT_UART1_printf_string("Please, Enter the time sequentially.(ex: 08:30)\n");
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				I2C_LCD_write_string_XY(0, 0, "What time should");
				sprintf(buff, "I feed? %02d times", (set_count+1));
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 1;
			}
			else{
				I2C_LCD_write_string_XY(0, 0, "What time should");
				sprintf(buff, "I feed? -- times");
				I2C_LCD_write_string_XY(1, 0, buff);
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			BT_UART1_printf_string("Set ");
			BT_UART1_printf_string(receiveData);
			BT_UART1_printf_string("\n");
			
			uint8_t j = 0;
			
			// times만큼 동적할당한 2차원 배열 feed_time[times][2]에 시간, 분 값을 넣어야함
			// 08:30 과 같이 정해진 양식으로 값을 받아서
			time_Tokken = strtok(receiveData, ":");
			while(time_Tokken != NULL){
				feed_time[set_count][j] = atoi(time_Tokken);
				j++;
				if(j >= 2) j = 0;
				time_Tokken = strtok(NULL, ":");
			}
			
			// 예외처리, 시간값에 맞게 입력하게끔
			if(feed_time[set_count][0] > 23 || feed_time[set_count][0] < 0 ){
				BT_UART1_printf_string("Please, Insert the hour from 0 to 23\n");
				break;
				}else if(feed_time[set_count][1] > 59 || feed_time[set_count][0] < 0 ){
				BT_UART1_printf_string("Please, Insert the minute from 0 to 59\n");
				break;
			}
			
			// 예외처리, 중복된 값 재입력
			for(int i = 0; i < set_count; i++){
				if(   feed_time[set_count][0] == feed_time[i][0]
				&& feed_time[set_count][1] == feed_time[i][1] )
				{
					BT_UART1_printf_string("Duplicate value found.\n");
					duplicate_flag = 1;
				}
			}
			if(duplicate_flag == 1){
				duplicate_flag = 0;
				break;
			}
			set_count++;
		}
		
		// times만큼 시간값을 설정해준다면 전역변수 초기화 후 완료
		if(set_count >= times){
			// 설정 값 보여주기
			BT_UART1_printf_string("\n       --Setting Time--\n");
			for(int i = 0; i < set_count; i++){
				if(i == 3) BT_UART1_printf_string("\n");
				sprintf(bt_buff, "[%02d:%02d] ", feed_time[i][0], feed_time[i][1]);
				BT_UART1_printf_string(bt_buff);
			}
			BT_UART1_printf_string("\n\n");
			
			blink_flag = 0;
			input_flag = 0;
			settingState = 2;
			break;
		}
		break;
		
		case 2:
		if(input_flag == 0){
			BT_UART1_printf_string("Step 3. How many feed do you give at a time?\n");
			sprintf(bt_buff, "unit : %dg , ex) '1' = %dg, '3' = %dg\n", UNIT, UNIT, UNIT * 3);
			BT_UART1_printf_string(bt_buff);
			input_flag = 1;
		}
		
		if(millis() - prevMillis > 500){
			if (blink_flag == 0){
				I2C_LCD_write_string_XY(0, 0, "How many feed do");
				I2C_LCD_write_string_XY(1, 0, "you give?       ");
				blink_flag = 1;
			}
			else{
				I2C_LCD_write_string_XY(0, 0, "How many feed do");
				I2C_LCD_write_string_XY(1, 0, "you give? ----- ");
				blink_flag = 0;
			}
			prevMillis = millis();
		}
		
		if(BT_isRxString()){
			receiveData = BT_getRxString();
			quantity = atoi(receiveData);
			if(quantity <= 0){
				BT_UART1_printf_string("Please, Enter a number above 0\n");
				break;
			}
			sprintf(bt_buff, "Set %dg\n", (quantity*UNIT));
			BT_UART1_printf_string(bt_buff);
			
			// 설정값 출력 및 전역변수 초기화 후 완료
			BT_UART1_printf_string("\nSetting Complete!\n");
			show_Set();
			
			I2C_LCD_write_string_XY(0, 0, "Setting Complete");
			I2C_LCD_write_string_XY(1, 0, "      ---       ");
			_delay_ms(1000);
			
			blink_flag = 0;
			input_flag = 0;
			set_count = 0;
			settingState = 0;
			settingFlag = 0;
			RUN_STATE = CLOCK;
			
			print_Menu();
			break;
		}
		break;
	}
}

void feeding_at_time(){
	for(int i = 0; i < times; i++){
		if( (feed_time[i][0] == stTime.hour) && (feed_time[i][1] == stTime.minutes)
		    && (feeding_flag == 0) ){
			feeding(quantity, servo_degree);
			sprintf(bt_buff, "TIME : %02d:%02d\n", stTime.hour, stTime.minutes);
			BT_UART1_printf_string(bt_buff);
			sprintf(bt_buff, "Feed : %dg\n", (quantity * UNIT));
			BT_UART1_printf_string(bt_buff);
			BT_UART1_printf_string("Auto Feeding Complete!\n");
			feeding_flag = 1;
		}
		// 1분 뒤 flag 초기화
		else if( (feed_time[i][0] == stTime.hour) && (feed_time[i][1] == (stTime.minutes + 1))
		         && (feeding_flag == 1) ){
			feeding_flag = 0;
		}
	}
}

void show_Set(){
	BT_UART1_printf_string("\n\n----------SETTING----------\n");
	sprintf(bt_buff, "  ○ %d times a day\n  ○ ", times);
	BT_UART1_printf_string(bt_buff);
	for(int i = 0; i < times; i++){
		if(i == 3) BT_UART1_printf_string("\n       ");
		sprintf(bt_buff, "[%02d:%02d] ", feed_time[i][0], feed_time[i][1]);
		BT_UART1_printf_string(bt_buff);
	}
	sprintf(bt_buff, "\n  ○ %dg per a time\n\n\n\n", quantity * UNIT);
	BT_UART1_printf_string(bt_buff);
}
//---------- 함수 선언부 끝 ----------//
//////////////////////////////////////