#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "SNIPE.h" 

#define ATSerial Serial

#define TXpin 14
#define RXpin 12

const char* ssid = "gahyun"; // 사용할 Wi-Fi 네트워크 SSID
const char* password = "20001231"; // 사용할 Wi-Fi 네트워크 비밀번호

// URL 경로 또는 IP 주소를 입력해 주세요
const char* serverName = "https://6j1gk9du1k.execute-api.ap-northeast-2.amazonaws.com/prod/devices/1"; // HTTP POST 요청을 보낼 주소 (API Gateway 주소)

unsigned long lastTime = 0; // 마지막으로 HTTP POST 요청을 보낸 시간
unsigned long timerDelay = 5000; // 타이머 딜레이, 5초로 설정

// 16byte hex key (LoRa 암호화 키)
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";

// 소프트웨어 시리얼 통신 객체 생성
SoftwareSerial DebugSerial(RXpin,TXpin);
// SNIPE 라이브러리 객체 생성
SNIPE SNIPE(ATSerial);

void setup() {
  ATSerial.begin(115200);
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);

  WiFi.begin(ssid, password); // Wi-Fi 연결 시작
  Serial.println("연결 중"); // 시리얼 출력: 연결 중
  while(WiFi.status() != WL_CONNECTED) { // Wi-Fi 연결이 되었는지 확인
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi 네트워크에 연결되었습니다. IP 주소는 다음과 같습니다: ");
  Serial.println(WiFi.localIP()); // Wi-Fi 연결된 로컬 IP 주소 출력
  
  Serial.println("타이머를 5초로 설정하였습니다 (timerDelay 변수). 첫 번째 읽기 전에 5초가 걸릴 것입니다.");
  
  // LoRa 모듈 초기화 및 설정
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }  

  // LoRa 암호화 키 설정
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }

  // LoRa 주파수 설정
  if (!SNIPE.lora_setFreq(LORA_CH_6)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  // LoRa Spreading Factor (전송 속도 설정)
  if (!SNIPE.lora_setSf(LORA_SF_12)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  // LoRa 수신 타임아웃 설정
  if (!SNIPE.lora_setRxtout(3000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }    
    
  DebugSerial.println("Simple LoRa Gateway StartUP");
}

int deviceid = 0;
int flame = 0;
int valf = 0;
  
void loop() {
  char message[100];
  // 5초마다 HTTP POST 요청을 보냅니다.
  if ((millis() - lastTime) > timerDelay) {
    // Wi-Fi 연결 상태 확인
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      http.begin(client, serverName); // HTTP 요청을 위한 연결 세팅
      
      if(SNIPE.lora_send("1")){
        String ver = SNIPE.lora_recv();
        delay(300);
        parseReceivedData(ver);
        delay(500);
      }
      
      sprintf(message, "{\"device\":\"%d\",\"fire\":\"%d\",\"dust\":\"%d\"}", deviceid, flame, valf);
      
      // 콘텐츠 타입이 application/json인 HTTP 요청이 필요한 경우 아래 코드를 사용하세요.
      const uint8_t* payload;
      int payloadSize = strlen(message);
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(payload,payloadSize);
 
        
      http.end(); // 리소스 해제
    }
    else {
      Serial.println("WiFi 연결이 끊겼습니다.");
    }
    lastTime = millis(); // 현재 시간을 lastTime 변수에 업데이트
  }

  delay(500);
 }
 
void parseReceivedData(const String& data) {
  int index_one = data.indexOf(' ');
  int index_two = data.indexOf(' ', index_one + 1);

  String first = data.substring(0, index_one);
  String second = data.substring(index_one + 1, index_two);
  String third = data.substring(index_two + 1);

  deviceid = first.toInt();
  flame = second.toInt();
  valf = third.toInt();

  DebugSerial.println(String(deviceid));
  DebugSerial.println(String(flame));
  DebugSerial.println(String(valf));
}
 
