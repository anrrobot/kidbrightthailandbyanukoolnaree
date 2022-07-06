//buletooth
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_bt_device.h"
#include "KB_ht16k33.h"
#include "KB_music.h"
#include <IRremote.h>
#include <iKB-1.h>
#include <Wire.h>

iKB_1 ikb(&Wire1);
KB_music music = KB_music();
KB_8x16Matrix matrix = KB_8x16Matrix();
typedef int Number;
typedef int Boolean;
using namespace std;
static const uint8_t KB_BUTTON1 = 16;
static const uint8_t KB_BUTTON2 = 14;

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

int RECV_PIN = 19;
int SEND_PIN = 18;
IRrecv irrecv(RECV_PIN);
decode_results results;
uint16_t sAddress = 0x0102;
uint8_t sCommand;
uint8_t sRepeats = 2;
    
//UUID kidbright AI
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

String tarum="";
uint32_t begin_time;
int state = 0;
uint32_t next_time;
int takeone=1;
void showLED(int num){
  for(int i=0; i<4; i++){
    if(i==(num-1)){
      ikb.digitalWrite(i, HIGH);
    }else{
      ikb.digitalWrite(i, LOW);
    }
  }
}

void printDeviceAddress() {
  const uint8_t* point = esp_bt_dev_get_address();
  for (int i = 0; i < 6; i++) {
    char str[3];
    sprintf(str, "%02X", (int)point[i]);
    Serial.print(str);
    if (i < 5){
      Serial.print(":");
    }
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();     
      if (rxValue.length() > 0) {
        tarum = rxValue.c_str();
      }
    }
};

void conductor_start(){

  for(int i=160; i>50; i--){
    ikb.servo(1, i);
    delay(20);
  }
  for(int i=0; i<30; i++){
    ikb.servo(2, i);
    delay(20);
  }
  for(int i=30; i>0; i--){
    ikb.servo(2, i);
    delay(20);
  }
  
}

void conductor_stop(){

  for(int i=50; i<160; i++){
    ikb.servo(1, i);
    delay(20);
  }

  
}

void conductor_act1(){
  for(int i=0; i<30; i++){
    ikb.servo(2, i);
    ikb.servo(1, i);
    delay(20);
  }
  for(int i=30; i>0; i--){
    ikb.servo(2, i);
    ikb.servo(1, i);
    delay(20);
  }
}

void conductor_act2(){
  for(int i=50; i>30; i--){
    ikb.servo(1, i);
    delay(20);
  }
  for(int i=30; i<50; i++){
    ikb.servo(1, i);
    delay(20);
  }
}

void conductor_act3(){
  for(int i=50; i>30; i--){
    ikb.servo(2, i);
    delay(20);
  }
  for(int i=30; i<50; i++){
    ikb.servo(2, i);
    delay(20);
  }
}
  /*for(int i=50; i>30; i--){
    ikb.servo(1, i);
    delay(20);
  }
  for(int i=30; i<50; i++){
    ikb.servo(1, i);
    delay(20);
  }*/


void setup() {
  Serial.begin(115200);
  music.begin();
  Wire1.begin(4,5);
  ikb.begin();
  matrix.displayBegin();
  pinMode(KB_BUTTON1, INPUT_PULLUP);
  pinMode(KB_BUTTON2, INPUT_PULLUP);
  
  irrecv.enableIRIn();
  irrecv.blink13(true);
  IrSender.begin(SEND_PIN, ENABLE_LED_FEEDBACK);
  
  // Create the BLE Device
  BLEDevice::init("Conductor");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.print("BT MAC: ");
  printDeviceAddress();
  Serial.println("");
  Serial.println("Waiting a client connection to notify...");
  matrix.scrollText(String("Wait"));
  music.tone(659, 250);
  ikb.servo(1, 160);
  ikb.servo(2, 10);
  state = 0;
  
}

void loop() {

  if (irrecv.decode(&results)) {
    Serial.print("receive IR : ");
    Serial.println(results.value);
    irrecv.resume();
  }
  
  //if (((int)digitalRead(KB_BUTTON1)) == 0) {
  if (ikb.digitalRead(6) == 0) {
    music.tone(659, 250);
    state=1;
    begin_time = millis();
  }
  
  if(state==0){
    matrix.printText(0, 0, String("W"));
    delay(100);
    
  }else if (state == 1 ){
    if(millis() < (begin_time+ 20000)) {
      Serial.println("wait for speaker");
      matrix.printText(0, 0, String("S"));
      //number = 1
      sCommand = 0xFF30CF;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      delay(40);
    }else if(millis() < (begin_time+ 41000)) {
      if (takeone == 1){
        conductor_start();
        takeone = 0;
      Serial.println("send ponglang play");
      matrix.printText(0, 0, String("P"));
      conductor_act1();
      //number = 2
      sCommand = 0xFF18E7;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      delay(40);
    }else if(millis() < (begin_time+ 43000)) {
      Serial.println("wait");
      matrix.printText(0, 0, String("w"));
    }else if(millis() < (begin_time+ 58500)) {
      Serial.println("send phin play");
      Serial.println("send dancer move");
      matrix.printText(0, 0, String("D"));
      conductor_act2();
      //number = 3
      sCommand = 0xFF7A85;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      delay(40);
    }else{
      Serial.println("send musicial play");
      matrix.printText(0, 0, String("A"));
      state=2;
      conductor_act2();
      //number = 4
      sCommand = 0xFF10EF;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      delay(40);
    }
    delay(1000);
  }else if (state == 2 ){
    conductor_act2();
    Serial.println(tarum);
    matrix.scrollText(tarum);
    matrix.printText(0, 0, String(tarum));

    if(tarum == "111") {
      //number = 5
      sCommand = 0xFF38C7;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      showLED(2);
      delay(40);
    }else if(tarum == "222") {
      //number = 6
      sCommand = 0xFF5AA5;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      showLED(3);
      delay(40);
    }else if(tarum == "333") {
      //number = 7
      sCommand = 0xFF42BD;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      showLED(4);
      delay(40);
    }else if(tarum == "444") {
      //number = 8
      sCommand = 0xFF4AB5;
      IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
      showLED(1);
      delay(40);
    }
    
    if(millis() > (begin_time+ 115000)) {
      state=3;
      takeone = 1;
    }
    delay(100);
  }else if (state == 3 ){
    if (takeone == 1){
        conductor_stop();
        takeone = 0;
    }
    Serial.println("send stop all");
    matrix.printText(0, 0, "End");
    //number = 9
     sCommand = 0xFF52AD;
     IrSender.sendNEC(sAddress & 0xFF, sCommand, sRepeats);
    delay(1000);
  }

  
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
  
}
}
