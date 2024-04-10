#include <WiFi.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include "PCF8574.h" // IO expander module
#include "time.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN       1  //TX Pin GPIO 1
#define NUMPIXELS 2  //16

// Wifi Credentials
const char *ssid = "Dialog 4G 8D8";
const char *password = "H69eaF5M";

//Wifi Things
long WifiS; // Strength (RSSI)
String MAC;

// Objects to Send
String JSON;

// MQTT Credentials
const char *mqtt_broker = "broker.mqtt.cool";
const char *topic = "XO/RFID_TEST";
const char *mqtt_user = "XO_TEST";
const char *mqtt_pw = "XOTEST2024";
const int mqtt_port = 1883;

// For time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
struct tm timeinfo;
char timeString[20];

int RstBtn;
int potPin_D;
long potPin_A;

// GPIO Pins
const int analogPins[] = {32, 33, 34, 35};
const int digitalPins[] = {4, 12, 13, 14, 15, 25 ,26, 27};

// Initializing LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Initializing Expander
PCF8574 pcf8574(0x3F);

// Create Instances
WiFiClient espClient;
PubSubClient client(espClient);

// Setup RFID Module
#define RST_PIN 20
#define SS_PIN  5
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Setup Pixels
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

void check_RFID(void);

void callback(char *topic, byte *payload, unsigned int length) {
  lcd.clear();
  lcd.print("Message arrived : ");
  lcd.println(topic);
  lcd.println("Msg: ");
  for (int i = 0; i < length; i++) {
      lcd.print((char) payload[i]);
  }
  delay(500);
  lcd.clear();
}

void getTime() {

  while (!getLocalTime(&timeinfo)){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println("Trying to get time..");
    delay(2000);
    lcd.clear();
  }
  lcd.setCursor(0,0);
  lcd.print("Time Synced..!");
  lcd.setCursor(0,1);
  lcd.println(&timeinfo, "%H:%M:%S");
  delay(2000);
  lcd.clear();
}

void check_RFID() {

  // Reading RFID Presence
  if (!mfrc522.PICC_IsNewCardPresent()) {   
    delay(50);
    return;
  }
  
  if (!mfrc522.PICC_ReadCardSerial()) {   
    delay(50);
    return;
  }
 
  pcf8574.digitalWrite(P2, HIGH);
  delay(50);
  pcf8574.digitalWrite(P2, LOW);   

  //Getting time
  getLocalTime(&timeinfo);
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  
  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  lcd.setCursor(0,0);      
  lcd.print("RFID Detected!");
  lcd.setCursor(0,1);
  lcd.print("Card :" + String(content.c_str()));
  WifiS = WiFi.RSSI();
  JSON = "{\"Time\":" + String(timeString) + ", \"DeviceID\":" + MAC + ", \"ReadCard\":" + content.c_str() + ", \"WifiS\":" + String(WifiS) + "}";
  client.publish(topic, JSON.c_str());
  delay(1000);
  lcd.clear();
}

void restartDevice() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Restarting Device.");
  //Setting Beep Tone
  pcf8574.digitalWrite(P2, HIGH);
  delay(50);
  pcf8574.digitalWrite(P2, LOW);
  lcd.clear();
  lcd.print("Restarting Device..");
  pcf8574.digitalWrite(P2, HIGH);
  delay(50);
  pcf8574.digitalWrite(P2, LOW);
  delay(50);
  lcd.clear();
  lcd.print("Restarting Device...");
  pcf8574.digitalWrite(P2, HIGH);
  delay(50);
  pcf8574.digitalWrite(P2, LOW);
  delay(50);
  pcf8574.digitalWrite(P2, HIGH);
  delay(50);
  pcf8574.digitalWrite(P2, LOW);
  delay(50);
  lcd.clear();
  ESP.restart();
}

void setup() {

  // GPIO Declaration - Analog
  for (int i = 0; i < 4; i++) {
    pinMode(analogPins[i], INPUT);
  }

  // GPIO Declaration - Analog
  for (int i = 0; i < 8; i++) {
    pinMode(digitalPins[i], INPUT);
  }

  pixels.begin(); 
  delay(100); 

  SPI.begin();

  pixels.setPixelColor(0, pixels.Color(0,0,0)); //Blue LED , LED1
  pixels.show();
  pixels.setPixelColor(1, pixels.Color(0,0,0)); // GREEN LED , LED2
  pixels.show();

  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to RFID Reader..");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);

  // Initializing RFID
  mfrc522.PCD_Init();
  delay(50);

  // Initializing IO Expander
  pcf8574.pinMode(P0, INPUT_PULLUP);
  pcf8574.pinMode(P1, INPUT_PULLUP);
  pcf8574.pinMode(P2, OUTPUT);
  digitalWrite(P2,LOW);
  pcf8574.begin();
  pcf8574.digitalWrite(P2, LOW);

  MAC = WiFi.macAddress();
  WiFi.mode(WIFI_STA);

  //Connecting to Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print("Connecting to WiFi.");
    delay(200);
    lcd.clear();
    lcd.print("Connecting to WiFi..");
    delay(200);
    lcd.clear();
    lcd.print("Connecting to WiFi...");
    delay(200);
    lcd.clear();
  }
  if(WiFi.status() == WL_CONNECTED){
    lcd.setCursor(0, 0);
    lcd.print("Wifi Connected..!");
    delay(1500);
    lcd.clear();
  }

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  //client.connect("RFID-TEST", mqtt_user, mqtt_pw);

  while (!client.connected()) {
    String client_id = "esp32-client-";
    lcd.print("Connecting to MQTT.");
    delay(200);
    lcd.clear();
    lcd.print("Connecting to MQTT..");
    delay(200);
    lcd.clear();
    lcd.print("Connecting to MQTT...");
    delay(200);
    lcd.clear();
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_pw)) {
      lcd.setCursor(0, 0);
      lcd.print("MQTT Connected..");
      delay(1500);
      lcd.clear();

    } else {
      lcd.print("MQTT Failed : \n\t");
      lcd.print(client.state());
      delay(2000);
      lcd.clear();
    }
  }

  //Setting up time 
  configTime(gmtOffset_sec, 0, ntpServer);
  delay(100);
  getTime();
  client.publish(topic, "Hi from ESP..");

}

void loop() {

  // Uncomment for Checking Digital in
  //--------------------------------------------------
  // potPin_D = digitalRead(4);
  // lcd.print("Pin 4 (D) - " + String(potPin_D));
  // delay(2000);
  //--------------------------------------------------
  
  // Uncomment for Checking Analog in
  //--------------------------------------------------
  // potPin_A = analogRead(33);
  // lcd.print("Pin 11 (A) - " + String(potPin_A));
  // delay(2000);
  //--------------------------------------------------

  // lcd.clear();

  client.loop();
  check_RFID();
  RstBtn = pcf8574.digitalRead(P0);
  if (RstBtn ==0){
    restartDevice();
  }
}
