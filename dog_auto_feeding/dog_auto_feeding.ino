#include<Servo.h>
#include<ThreeWire.h>
#include<RtcDS1302.h>
#include"HX711.h"

// DS1302 CLK/SCLK --> 6
// DS1302 DAT/IO --> 5
// DS1302 RST/CE --> 4
ThreeWire myWire(5,6,4);// IO, SCLK, CE

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 10; // 파랑
const int LOADCELL_SCK_PIN = 11;  // 보라

#define SERVO_PIN 13

#define FEEDING         10000  // 피딩 시간 조절
#define FEEDING_SCALE   5      // 피딩 양 조절
#define BREAKFAST_HOUR  6
#define BREAKFAST_MIN   0
#define DINNER_HOUR     17
#define DINNER_MIN      20

RtcDS1302<ThreeWire> Rtc(myWire);

Servo motor;

RtcDateTime old_now;

HX711 scale;

void setup() {
  Serial.begin(9600);
  
  motor.attach(SERVO_PIN);

  Serial.print("compiled : ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if(Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
     
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }
  
  RtcDateTime now = Rtc.GetDateTime();
  RtcDateTime old_now = Rtc.GetDateTime();
  if (now < compiled) 
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // RTC Init END
  
  // HX711 Init
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  InitHX711();
  scale.power_up();
  
  // First Run
  Serial.println("First Run");
  motor.write(250);
  delay(FEEDING);
}

String str = "";

void loop() {
  delay(5);
  RtcDateTime now = Rtc.GetDateTime();
  pinMode(SERVO_PIN, INPUT);
  
  if( old_now != now )
  {
    printDateTime(now);
    Serial.println(); 

    Serial.println(scale.get_units(5), 1);
    
    old_now = now;
    
    if( now.Hour() == BREAKFAST_HOUR || now.Hour() == DINNER_HOUR )
    {
      if( ( now.Minute() == BREAKFAST_MIN || now.Minute() == DINNER_MIN ) && now.Second() == 0 )
      {
        scale.tare();

        delay(1000);
        
        Serial.println("FEEDING!!! HAPPY NUN");

        pinMode(SERVO_PIN, OUTPUT);
        motor.write(250);
        //delay(FEEDING);
        for( double i = scale.get_units(10); i < FEEDING_SCALE; i = scale.get_units(10)){
          now = Rtc.GetDateTime();
          if( now.Second() > old_now.Second() + 20 ) break;
          Serial.println(i, 1);
        }
      }
    } 
  }
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

void InitHX711()
{ 
  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  Serial.println("Initializing the scale");

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale
}
