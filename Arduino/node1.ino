#define PING  1
#define PONG  2

#define CODE  PING  
#include "string.h"

#include <SoftwareSerial.h>
#include "SNIPE.h" 
#define TXpin 12
#define RXpin 13
#define ATSerial Serial

//--선풍기 팬--
#define FAN_PIN7 7
#define FAN_PIN6 6

//--미세먼지--
int measurePin = 1; //Connect dust sensor to Arduino A1 pin
int ledPower = 2;   //Connect 3 led driver pins of dust sensor to Arduino D2

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

#define NUMREADINGS 10 // 평균내는 개수 수정
int readings[NUMREADINGS] = {0,};
int myindex = 0;
#undef index//재정의
int total = 0;
int valf = 0;

//--서브모터--
#include <Servo.h>
Servo myservo;  
int pos = 0;    //position
bool moved = false;

String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";  //16byte hex key

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

void setup() {
  ATSerial.begin(115200);
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }

  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }

  if (!SNIPE.lora_setFreq(LORA_CH_6)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  if (!SNIPE.lora_setSf(LORA_SF_12)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }  
    
  DebugSerial.println("SNIPE LoRa PingPong Test");

  //--선풍기 팬--
  pinMode(FAN_PIN6,OUTPUT);
  pinMode(FAN_PIN7,OUTPUT); 

  //--서브모터--
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);   // initialize servo position

  //--미세먼지--
  pinMode(ledPower, OUTPUT);
  
  delay(100); 
}


void loop() {
  digitalWrite(FAN_PIN6, HIGH);
  digitalWrite(FAN_PIN7, HIGH);

  // 불꽃 센서 읽기
  int flameSensorValuef = analogRead(A2);
  String flameSensorValue = String(flameSensorValuef);
  DebugSerial.println("flameSensorValue   "+flameSensorValue);

  // 미세먼지 센서 읽기
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin);
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024.0);
  dustDensity = 0.17 * calcVoltage - 0.1;

  total -= readings[myindex];
  readings[myindex] = dustDensity * 1000;
  total += readings[myindex];
  myindex++;

  if (myindex >= NUMREADINGS) {
    myindex = 0;
  }
  valf = total / NUMREADINGS;
  String val = String(valf);
  DebugSerial.println("val   "+val);

  // 추가된 부분: 조건에 따라 DC 모터와 팬 동작
  if ((flameSensorValuef < 900) || (valf > 60)) { //자동 제어
    if (!moved) {
      for (pos = 0; pos <= 180; pos += 1) {
              myservo.write(pos);
              delay(13);
            }
      moved = true;
      delay(100);
      digitalWrite(FAN_PIN6, LOW);
      digitalWrite(FAN_PIN7, HIGH);
    }
  } else {
    if (moved) {
      for (pos = 180; pos >= 0; pos -= 1) {
        myservo.write(pos);
        delay(13);
      }
      moved = false;
      delay(100);
      digitalWrite(FAN_PIN6, HIGH);
      digitalWrite(FAN_PIN7, HIGH);
    }
  }

#if CODE == PING         
  String flameSensor_Value = String(flameSensorValuef); // 불꽃센서
  String va_l = String(valf);//미세먼지
  String msg = "1 " + flameSensor_Value + " " + va_l;
    
  String ver = SNIPE.lora_recv();
  delay(500);
  DebugSerial.println(ver);       

if (ver =="1"){
    if (SNIPE.lora_send(msg)) {
      DebugSerial.println("send success");
      DebugSerial.println("msg => " + msg);
    } else {
      DebugSerial.println("send fail");
      delay(500);
    }
    delay(500);    
}                                                           
  
#elif CODE == PONG
        String ver = SNIPE.lora_recv();
        delay(300);

        DebugSerial.println(ver);
        
        if (ver == "PING" )
        {
          DebugSerial.println("recv success");
          DebugSerial.println(SNIPE.lora_getRssi());
          DebugSerial.println(SNIPE.lora_getSnr());

          if(SNIPE.lora_send("PONG"))
          {
            DebugSerial.println("send success");
          }
          else
          {
            DebugSerial.println("send fail");
            delay(500);
          }
        }
       delay(1000);
#endif
}
