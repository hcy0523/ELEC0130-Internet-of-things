#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <utility/wifi_drv.h>


// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// Watson IoT connection details
#define MQTT_HOST "q4svz9.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:q4svz9:mkr1010:9c9c1fe1a8d0"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "mkr1010_9c9c1fe1a8d0"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"


// Add WiFi connection information
char ssid[] = "zxxi";     //  your network SSID (name)
char pass[] = "zxxi1234.";  // your network password
const int LED1 = 0;
const int LED2 = 1;
const int buzzer = 2;
int WiFistatus = WL_IDLE_STATUS;     // the Wifi radio's status

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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
//  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
  for(int i=0;i < length;i++){
    Serial.print((char)payload[i]);
    }
 Serial.println("-----进入payload函数----");

  if((char)payload[0]== '1' && (char)payload[1]== '1'){
   
      WiFiDrv::analogWrite(25, 255); //green LED switched on
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      delay(2000);
      WiFiDrv::analogWrite(25, 0); //green LED switched on
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
    }else if((char)payload[0] == '1' && (char)payload[1]== '2'){
      WiFiDrv::analogWrite(25, 255);
      WiFiDrv::analogWrite(26, 0);//blue LED switched on
      WiFiDrv::analogWrite(27, 255);
      delay(2000);
      WiFiDrv::analogWrite(25, 0); //green LED switched on
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
    }else if((char)payload[0] == '2' && (char)payload[1]== '1'){
      WiFiDrv::analogWrite(25, 0);
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 255); //purple LED switched on
      delay(2000);
      WiFiDrv::analogWrite(25, 0); //green LED switched on
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
    }else if((char)payload[0] == '2' && (char)payload[1]== '2'){
      WiFiDrv::analogWrite(25, 255);
      WiFiDrv::analogWrite(26, 0); //red LED switched on
      WiFiDrv::analogWrite(27, 0);
     // tone(buzzer, 1000); // Send 1KHz sound signal...
      delay(2000);
      WiFiDrv::analogWrite(25, 0); //green LED switched on
      WiFiDrv::analogWrite(26, 0);
      WiFiDrv::analogWrite(27, 0);
      //noTone(buzzer);  
    }

}

void setup() {
  //Initialize serial and wait for port to open:
  WiFiDrv::pinMode(25, OUTPUT); //configure green/red pin as an output pin
  WiFiDrv::pinMode(26, OUTPUT); //configure red/green pin as an output pin
  WiFiDrv::pinMode(27, OUTPUT); //define blue pin as an output pin
  //pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
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
    delay(10000);
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
      delay(5000);
    }
  }
  
   //float light_value=analogRead(A0);
   //float temper = 80;

  // Check if any reads failed and exit early (to try again).
  /*if (isnan(light_value)) {
    Serial.println("Failed to read data!");
  } else {

    // Send data to Watson IoT Platform
    status["light_value"] = light_value;
    status["temper"] = temper;
    serializeJson(jsonDoc, msg, 50);
    Serial.println(msg);
    if (!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");
    }
  }*/

  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 4; i++) {
    mqtt.loop();
    delay(1000);
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
