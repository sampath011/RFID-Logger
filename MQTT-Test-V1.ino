#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include "PCF8574.h"   //IO expander //
//Added for data transfer
#include <WiFi.h>
#include <PubSubClient.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
PCF8574 pcf8574(0x3F); 

// Connection Details

const char* ssid     = "Dialog 4G 8D8";
const char* password = "H69eaF5M";

const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttTopic = "RFID_XO_TEST";

WiFiClient espClient;
PubSubClient client(espClient);

//--------------------RFID------------------------------------------
#define RST_PIN 20 // not required
#define SS_PIN  5  // SS Pin GPIO5
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//---------------RGB LED TEST---------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN       1  //Tx pin GPIO1
#define NUMPIXELS 2  //16


int SW1;
int SW2;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

void check_RFID(void);
void scan_GPIO(void);

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker!");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {

  // Serial communication
  Serial.begin(115200);
  delay(100); 

  // Connecting to Wifi
  Serial.println();
  Serial.print("Connecting to Wifi..");

  pixels.begin(); 
  delay(100);  
  //G,R,B
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //Blue LED , LED1
  pixels.show();
  pixels.setPixelColor(1, pixels.Color(0,0, 0)); //GREEN LED , LED2
  pixels.show();

  pinMode(4, INPUT);  //IN 1
  pinMode(15, INPUT);
  pinMode(13, INPUT);
  pinMode(12, INPUT);
  pinMode(14, INPUT);
  pinMode(27, INPUT);
  pinMode(26, INPUT);
  pinMode(25, INPUT);
  pinMode(33, INPUT);
  pinMode(32, INPUT);
  pinMode(35, INPUT);
  pinMode(34, INPUT); //IN 12

  //-------------------
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  //-------------------

  pcf8574.pinMode(P0, INPUT_PULLUP);    //packet sending freq change input
  pcf8574.pinMode(P1, INPUT_PULLUP);    //packet sending freq change input
   
  pcf8574.pinMode(P2, OUTPUT);  //buzzer
  pcf8574.begin();

  pcf8574.digitalWrite(P2, LOW);    //LED ground
  
  lcd.begin(20,4);
  lcd.init();

  // Turn on the backlight.
  lcd.backlight();
  // Move the cursor characters to the right and
  // zero characters down (line 1).
  lcd.setCursor(0, 0);

  // Print HELLO to the screen, starting at 5,0.
  lcd.print("Initializing");
  delay(300);
  lcd.print("Initializing.");
  delay(300);
  lcd.print("Initializing..");
  delay(300);
  lcd.print("");

  // Tries
  int tryDelay = 500;
  int numberOfTries = 20;

  WiFi.begin(ssid, password);

  // Wait for the WiFi event
  while (true) {
      
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL:
          Serial.println("WiFi SSID not found");
          lcd.print("WiFi SSID not found";)
          break;
        case WL_CONNECT_FAILED:
          Serial.print("WiFi not connected! Reason: ");
          return;
          break;
        case WL_CONNECTION_LOST:
          Serial.println("WiFi Connection lost");
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("WiFi Scan completed");
          break;
        case WL_DISCONNECTED:
          Serial.println("WiFi disconnected!");
          break;
        case WL_CONNECTED:
          Serial.println("WiFi is connected!");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          lcd.print("Wifi Connected..!")
          delay (500);
          lcd.print("")
          return;
          break;
        default:
          Serial.print("WiFi Status: ");
          Serial.println(WiFi.status());
          break;
      }
      delay(tryDelay);
      
      if(numberOfTries <= 0){
        Serial.print("Failed to connect to WiFi!");
        // Use disconnect function to force stop trying to connect
        WiFi.disconnect();
        return;
      } else {
        numberOfTries--;
      }
  }

  // Setting up MQTT
  client.setServer(mqttServer, mqttPort);

  // Move the cursor to the next line and print
  // WORLD.
  lcd.setCursor(0, 1);     
  lcd.print("RFID CONTROLLER");
  lcd.setCursor(1, 2);      
  lcd.print("DEMO");
  lcd.setCursor(0, 3);      
  lcd.print("              ");
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }


SW1 = pcf8574.digitalRead(P1);
SW2 = pcf8574.digitalRead(P0);

//Serial.print("loop");
//pcf8574.digitalWrite(P2, HIGH);   
//  delay(1000);
//  pcf8574.digitalWrite(P2, LOW);    
//  delay(1000);

if(SW1==0){
  pixels.setPixelColor(0, pixels.Color(0, 0, 90)); //Blue LED , LED1
  pixels.show();
  pcf8574.digitalWrite(P2, HIGH); 
  lcd.setCursor(1, 3);   
  lcd.print("SW1");
  delay(100);
  pcf8574.digitalWrite(P2, LOW);   
  lcd.setCursor(1, 3);  
  delay(500); 
  lcd.print("              ");
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //Blue LED , LED1
  pixels.show();
}

if(SW2==0){
  pixels.setPixelColor(1, pixels.Color(0,30, 0)); //GREEN LED , LED2
  pixels.show();
  pcf8574.digitalWrite(P2, HIGH);
  lcd.setCursor(1, 3);   
  lcd.print("SW2");
  delay(100);
  pcf8574.digitalWrite(P2, LOW); 
  lcd.setCursor(1, 3);  
  delay(500); 
  pixels.setPixelColor(1, pixels.Color(0,0, 0)); //GREEN LED , LED2
  pixels.show(); 
  lcd.print("              ");
  
}

check_RFID();
scan_GPIO();

}

void check_RFID(void) {
  if (!mfrc522.PICC_IsNewCardPresent()) {   
    delay(50);
    return;
  }
  
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {   
    delay(50);
    return;
  }

  //-------------------------------------------------RFID----------------------------------------------

  // Shows the card ID on the serial console
     
  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  //Serial.println();
  content.toUpperCase();
  //Serial.println("Cart read:" + content);

  // Publish the UID to the MQTT topic
  client.publish(mqttTopic, content.c_str());

  if (!mfrc522.PICC_IsNewCardPresent()) {   
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    lcd.setCursor(1, 3);      
    lcd.print("RFID Detected!");
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
    delay(500);
    lcd.setCursor(0, 3);   
    //lcd.print("**************");
    lcd.print("              ");
  }
}


void scan_GPIO(void)
{
  if(digitalRead(4)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(15)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(13)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(12)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(14)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(27)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(26)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(25)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(33)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(32)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(35)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
   if(digitalRead(34)==0){
    pcf8574.digitalWrite(P2, HIGH);  //Buzzer
    delay(50);
    pcf8574.digitalWrite(P2, LOW);  //Buzzer
  }
  
}
