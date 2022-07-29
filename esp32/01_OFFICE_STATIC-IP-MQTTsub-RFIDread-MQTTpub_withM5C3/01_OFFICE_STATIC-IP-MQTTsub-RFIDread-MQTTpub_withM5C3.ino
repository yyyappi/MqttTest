#include <Wire.h>
#include "MFRC522_I2C.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* プッシュボタン */
const int buttonPin = 3;                // 接続ピン
int buttonState;                        // 状態
int lastButtonState = LOW;              // 直前の状態
unsigned long lastDebounceTime = 0;     // 直前の切替時刻
unsigned long debounceDelay = 50;       // デバウンスタイム(mSec.)

/* LED 設定 */
const int ledPin = 4;                   // 接続ピン
boolean ledFlag=false;                  // 状態

/* messageFlag MQTT-pubからメッセージがあったときのFlag */
boolean messageFlag=false;

/* OLED表示器 設定 */

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/* RFIDリーダライタ 設定 */
// 0x28 is i2c address on SDA. Check your address with i2cscanner if not match.
MFRC522 mfrc522(0x28);   // Create MFRC522 instance.

/* WIFI 設定*/
#include <WiFi.h>
const char* ssid="********";
const char* password="********";
const IPAddress ip(***,***,***,***);
const IPAddress gateway(***,***,***,***);
const IPAddress subnet(255,255,255,0);
const IPAddress DNS(***,***,***,***);

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

/* MQTT */

#include <AsyncMqttClient.h>
#define MQTT_HOST IPAddress(***,***,***,***)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

/* WIFI 接続 */
void connectToWifi() {
  WiFi.config(ip, gateway, subnet,DNS);
  delay(10);
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid,password);
}
void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

/* MQTT 接続 */
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}


/*****************************< MQTT CallBack functions >*****************************/

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  
  ledFlag =!ledFlag;
  messageFlag=!messageFlag;
 
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
/********************************************************************************/

void setup() {

  Serial.begin(115200);           // Initialize serial communications with the PC
  Wire.begin(1,0);                   // Initialize I2C

  pinMode(buttonPin,INPUT_PULLUP);  //ボタンピンのポートをプルアップ入力に設定
  pinMode(ledPin,OUTPUT);           //LEDピンのポートを出力に設定

  //MQTT Wifi 再接続タイマー設定
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  //Wifi接続
  WiFi.onEvent(WiFiEvent);
  connectToWifi();

  //MQTTコールバック関数設定
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  mfrc522.PCD_Init();             // Init MFRC522
  ShowReaderDetails();            // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

  //OLED begin
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));
  display.display();
  delay(2000);
}

void loop() {

  //if(isPushbuttonClicked()|| messageFlag){
  if(messageFlag){    
 /*****RFID読み込み****************/
   // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // Now a card is selected. The UID and SAK is in mfrc522.uid.

  // Dump UID
  display.clearDisplay();
  Serial.print(F("Card UID:"));

  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Card UID:"));  


  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    display.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    display.print(mfrc522.uid.uidByte[i], HEX);
    display.print(i<3 ? ":" : "");
    
  }

  Serial.println();
  display.println();
  display.display();
  
  /*****MQTT通信****************/

    short aa[]={1,0,0,0,0,0,0,0,0,0};
    char buf[60];
    for(short it=0;it<mfrc522.uid.size;it++){
      aa[it]=mfrc522.uid.uidByte[it];
    }

    sprintf(buf,"%02x %02x %02x %02x",aa[00],aa[01],aa[02],aa[03]);
    
    uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, buf);
    Serial.print("Publishing at QoS 2, packetId: ");
    Serial.println(packetIdPub2);
    Serial.println();
    
  delay(1000);
  }

  //LED点灯処理
  digitalWrite(ledPin,ledFlag);

  
}

/*****************************< RFID functions >*****************************/

//RFIDリーダの詳細を返す

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}

/*****************************< Other functions >*****************************/
/* プッシュボタンが押されているかを通知する */
bool isPushbuttonClicked() {
    // 状態が変わっていたらタイマーをリセットして
    int state = digitalRead(buttonPin);
    if (state != lastButtonState) {
        lastDebounceTime = millis();
    }
    // デバウンス時間後にボタンの状態が変化したら応答する
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (state != buttonState) {
            buttonState = state;
            if (buttonState == HIGH) {
                return true;
            }
        }
    }
    lastButtonState = state;      // ボタンの状態を退避する
    return false;
}
