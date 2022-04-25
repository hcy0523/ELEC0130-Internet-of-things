#include <Wire.h>
#include "rgb_lcd.h"
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// Watson IoT connection details
#define MQTT_HOST "htyn6s.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:htyn6s:mkr1010:7c9ebd3b4f84"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "mkr1010_7c9ebd3b4f84"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"

// Add WiFi connection information
char ssid[] = "OnePlus 9R";     //  your network SSID (name)
char pass[] = "hcy980523";  // your network password
const int LED1=0;
const int buzzer = 3; 
int WiFistatus = WL_IDLE_STATUS;     // the Wifi radio's status
int State=0;
int Coma=0;
int r=0;
int g=0;
int b=0;
rgb_lcd lcd;
// --------------------------------------------------------------------------------------------
//        SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS LINE
// --------------------------------------------------------------------------------------------

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

// variables to hold data
StaticJsonDocument<100> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[50];

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("The cough Level is ");
  Serial.println((char)payload[0]);
  Serial.print("The Coma Level is ");
  Serial.println((char)payload[1]);
//  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
//  for(int i=0;i < length;i++){
//    Serial.print((char)payload[i]);
//    }
  State=payload[0];
  Coma=payload[1];
  void noDisplay();
//  if((char)State == '0'){
//    Serial.println("Normal");
//  }else if((char)State == '1'){
//    Serial.println("You have coughed less than 3 Minutes");
//    
//  }else if((char)State == '2'){
//    Serial.println("You have coughed in 3-5 Minutes");
//  }else if((char)State == '3'){
//    Serial.println("You have coughed longer than 5 Minutes");
//    }
}

void setup() {
  //Initialize serial and wait for port to open:
  WiFiDrv::pinMode(25, OUTPUT); //configure green pin as an output pin
  WiFiDrv::pinMode(26, OUTPUT); //configure red pin as an output pin
  WiFiDrv::pinMode(27, OUTPUT); //define blue pin as an output pin
  pinMode(buzzer, OUTPUT); 
  Serial.begin(9600);
 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  void display();
  lcd.clear();
  r=0;
  g=255;
  b=255;
  lcd.setRGB(r, g, b);
  lcd.print("Wait ");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:

  while (WiFistatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    WiFistatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  
  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");

  // Connect to MQTT - IBM Watson IoT Platform
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);

  } else {
    Serial.println("MQTT Failed to connect!");
  }
}

void loop() {
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(2000);
    }
  }
  lcd.clear();
  if((char)Coma == '0'){//如果没昏迷，根据咳嗽等级
    if((char)State == '0'){//咳嗽等级为0，绿色2s，lcd显示Noraml,然后熄屏18s
      digitalWrite(LED1,LOW);
      r=0;
      g=100;
      b=0;
      lcd.setRGB(r, g, b);
      lcd.print("Normal");
      WiFiDrv::analogWrite(25, 0); //green LED switched on
      WiFiDrv::analogWrite(26, 255);
      WiFiDrv::analogWrite(27, 0);
      delay(2000);  
      r=0;
      g=30;
      b=0;
      lcd.setRGB(r, g, b);
      WiFiDrv::analogWrite(25, 0); //all LED switched off
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      delay(16500); 
    }else if((char)State == '1'){//咳嗽等级为1，黄色5s，lcd显示 cough < 3 Mins
      digitalWrite(LED1,LOW);
      r=255;
      g=255;
      b=0;
      lcd.setRGB(r, g, b);
      lcd.print("cough < 3 Mins");
      WiFiDrv::analogWrite(25, 255); //yellow LED switched on
      WiFiDrv::analogWrite(26, 255);
      WiFiDrv::analogWrite(27, 0);
      delay(5000);  
      WiFiDrv::analogWrite(25, 0); //all LED switched off
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      delay(3500);
    }else if((char)State == '2'){//咳嗽等级为2，橙色3s，lcd显示 cough 3-5 Mins
      digitalWrite(LED1,LOW);
      r=255;
      g=50;
      b=0;
      lcd.setRGB(r, g, b);
      lcd.print("cough 3-5 Mins");
      WiFiDrv::analogWrite(25, 255);
      WiFiDrv::analogWrite(26, 50); //orange LED switched on
      WiFiDrv::analogWrite(27, 0);
      delay(3000);    
      WiFiDrv::analogWrite(25, 0); //all LED switched off
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      delay(1500); 
    }else if((char)State == '3'){//咳嗽等级为3，红色1.5s，外置灯1.5s,lcd显示 cough > 5 Mins!!
      r=255;
      g=0;
      b=0;
      lcd.setRGB(r, g, b);
      lcd.print("cough > 5 Mins!!");
      WiFiDrv::analogWrite(25, 255);
      WiFiDrv::analogWrite(26, 0); //red LED switched on
      WiFiDrv::analogWrite(27, 0);
      digitalWrite(LED1,HIGH);
      delay(1500);
      r=125;
      g=0;
      b=0;
      lcd.setRGB(r, g, b);       
      WiFiDrv::analogWrite(25, 0); //all LED switched off
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
    
      digitalWrite(LED1,LOW);
      }else{
        r=0;
        g=255;
        b=255;
        lcd.setRGB(r, g, b);
        lcd.print("Wait ");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        }

//  // Check if any reads failed and exit early (to try again).
//  if (isnan(LIGHT_VALUE)) {
//    Serial.println("Failed to read data!");
//  } else {
//
//    // Send data to Watson IoT Platform
//    status["LIGHT_VALUE"] = LIGHT_VALUE;
//    serializeJson(jsonDoc, msg, 50);
//    Serial.println(msg);
//    if (!mqtt.publish(MQTT_TOPIC, msg)) {
//      Serial.println("MQTT Publish failed");
//    }
  }else if((char)Coma == '1')//如果昏迷直接,红灯+外置灯+蜂鸣器1.5s 
    r=255;
      g=0;
      b=0;
      lcd.setRGB(r, g, b);
      lcd.print("He's Coma !!");
      WiFiDrv::analogWrite(25, 255);
      WiFiDrv::analogWrite(26, 0); //red LED switched on
      WiFiDrv::analogWrite(27, 0);
      digitalWrite(LED1,HIGH);
      tone(buzzer, 1000); // Send 1KHz sound signal...
      delay(1500);
      r=125;
      g=0;
      b=0;
      lcd.setRGB(r, g, b);       
      WiFiDrv::analogWrite(25, 0); //all LED switched off
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      noTone(buzzer);     // Stop sound...
      digitalWrite(LED1,LOW);
      }else{
        r=0;
        g=255;
        b=255;
        lcd.setRGB(r, g, b);
        lcd.print("Wait ");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
  }
//  }

  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 4; i++) {
    mqtt.loop();
    delay(500);
  }
}

void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}
