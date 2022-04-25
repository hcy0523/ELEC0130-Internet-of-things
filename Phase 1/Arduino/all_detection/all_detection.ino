#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <utility/wifi_drv.h>
#include <Arduino_MKRENV.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>



// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// Watson IoT connection details
#define MQTT_HOST "08psvo.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:08psvo:mkr1010:7c9ebd3b4e1c"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "mkr1010_7c9ebd3b4e1c"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"

//defined variable
//Sensors: temperature, humidity, cough (sound, motion, air pressure), taking medicine (magnetic switch)
//Extra functions, interrupt detection
int Ismotion,IsMagnetic,buttonState,sound_value;  //logical tracing
int temp_high,temp_low,count_hour=-1,count_A=0,count_B=0;  //temperature difference and suitable temperature part.
int count_cough=0,count_tag=0,cough_minutes=0; //Set the cough part flag
int count_Magn=0,Magn_days=0,Magn_temp;//Set the medication frequency.
int count_tag2=0,send_frequency=2; //Transmission frequency changing part
int collect_frequency=1; //Acquisition frequency changing part
unsigned long timestamp,times;  //Variable timestamp
int days,hours,minutes,seconds;   //Define time variables
int coma_tag=0,buzzer_tag=0; //Define coma variables
int randnumber=0,randnumber2=0; //Temporary random number
int Tempdiff_temp=0,Tempdiffnumber_temp=0,Tempfit_temp=0,Tempfitcount_temp=0; //Save value


//Partial port definition
const int ledred = 0; 
const int button_detect = 6; 
const int buzzer = 2; 
const int Magnetic_detect = 1; 
const int Motion_detect = 3; 
//const int buzzer2 = 7;  
 

// Add WiFi connection information
char ssid[] = "D48C Hyperoptic Fibre Broadband";     //  your network SSID (name)
char pass[] =  "YMM8cSY7Eagv";  // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
unsigned int localPort = 2390;      // local port to listen for UDP packets


//Acquire the network through NTP protocol
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
int WiFistatus = WL_IDLE_STATUS;     // the Wifi radio's status

// --------------------------------------------------------------------------------------------
//        SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS LINE
// --------------------------------------------------------------------------------------------

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

// variables to hold data
//Define json file names and variables
StaticJsonDocument<250> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[250];
char newmsg[250];

//Return function
void callback(char* topic, byte* payload, unsigned int length)   {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
}
/*
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
//  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.print((char *)payload);
//  for(int i=0;i < length;i++){
//    Serial.print((char)payload[i]);
//    }
  if((char)payload[0] == '1'){
    digitalWrite(LED1,HIGH);
    
  }else if((char)payload[0] == '2'){
    digitalWrite(LED1,HIGH);
    delay(5000);
    digitalWrite(LED1,LOW);
    }else{
    digitalWrite(LED1,LOW);
    
    }
}*/

void setup() {
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(ledred,OUTPUT);
  pinMode(button_detect,INPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(Magnetic_detect,INPUT);
  pinMode(Motion_detect,INPUT);
  randomSeed(analogRead(A1));
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }
  attachInterrupt(5, InterruptProc, RISING);  //Interrupt detection
  //attachInterrupt(5, InterruptProc2, RISING); 
  }
  /*String fv = WiFi.firmwareVersion(); //utc part
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }*/
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
  Udp.begin(localPort);
  
}


void loop() {
  //Mqtt link part
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
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  /*for(int i=0;i<collect_frequency;i++){
    delay(1000);
  }*/
  for(int i=0;i<collect_frequency;i++){
    delay(1000);  
  }
  //delay(1000);  
    
  //Timestamp module
  if (Udp.parsePacket()) {
    //Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    //Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    // utc timestamp
    unsigned long epoch = secsSince1900 - seventyYears;
    timestamp=epoch;
    unsigned long start_time= 1643673600; //This is the initial timestamp of 0: 00 on February 1st, 2022.
    unsigned long current=epoch - start_time;
    // print Unix time:
    Serial.print("epoch = ");
    Serial.println(epoch);
    //Serial.print("Current time = ");
    //Serial.println(current);
    hours=(epoch  % 86400L) / 3600;
    minutes=(epoch  % 3600) / 60;
    seconds=epoch % 60;
  }

  //Define variable detection
  //static char msg[250];
  count_tag++;
  count_tag2++;
  sound_value=analogRead(A0);
  Ismotion= digitalRead(Motion_detect);
  IsMagnetic= digitalRead(Magnetic_detect);
  Serial.print("Magnetic ");
  Serial.println(IsMagnetic);
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature)) {
    Serial.println("Failed to read data!");
  } else {

    
  //And temperature difference control module.
    if(count_hour==-1){
      temp_high=temperature;
      temp_low=temperature;
      count_hour=hours;
      count_A++;
      status["Tempdiff"] = 0;
      status["Tempdiffnumber"] =0;
      status["Tempfit"] = 0;
      status["Tempfitcount"] = 0;
      status["Magn_Count"] = 0;
      status["label"] = 0;
      status["note"] = "None";
      
      Serial.println(msg);
      serializeJson(jsonDoc, msg, 250);
      if(!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");}
      jsonDoc.clear();
      JsonObject payload = jsonDoc.to<JsonObject>();
      JsonObject status = payload.createNestedObject("d");
      strcpy(msg,newmsg);
      if(temperature>30||temperature<20){
        count_B++;
      }
      }
    if(hours!=count_hour){
      count_hour=hours;
      if(temperature>30||temperature<20){
        count_B++;
      }
      if(temperature>temp_high){
        temp_high=temperature;
      }
      if(temperature<temp_low){
        temp_low=temperature;
      }
      if(hours==8&&count_A<24){
        //status["TIMESTAMP"] = timestamp;      
        count_A=0;
        if(temp_high-temp_low>10){
          Serial.println("The temperature difference is too high！！！");
          Tempdiff_temp = 1;
          Tempdiffnumber_temp = temp_high-temp_low;
          temp_high=temperature;
          temp_low=temperature;
        }
        else{
          Serial.println("The temperature difference is good！！！");
          Tempdiff_temp = 0;
          Tempdiffnumber_temp = temp_high-temp_low;
          temp_high=temperature;
          temp_low=temperature;
          }
        if(count_B>=9){
          Tempfit_temp = 1;
          Tempfitcount_temp = count_B;
          }
        else{
          Tempfit_temp = 0;
          Tempfitcount_temp = count_B;
          }
        Magn_days++;
      }
      if(count_A==24){
        //status["TIMESTAMP"] = timestamp;    
        count_A=0;
        if(temp_high-temp_low>10){
          Serial.println("The temperature difference is too high！！！");
          Tempdiff_temp = 1;
          Tempdiffnumber_temp = temp_high-temp_low;
          temp_high=temperature;
          temp_low=temperature;
        }
        else{
          Serial.println("The temperature difference is good！！！");
          Tempdiff_temp = 0;
          Tempdiffnumber_temp = temp_high-temp_low;
          temp_high=temperature;
          temp_low=temperature;
          }
        if(count_B>=9){
          Tempfit_temp = 1;
          Tempfitcount_temp = count_B;
          }
        else{
          Tempfit_temp = 0;
          Tempfitcount_temp = count_B;
          }
        Magn_days++;
        if(Magn_days==7){
          status["Magn_Count"] = count_Magn;
          count_Magn=0;
        }
      }     
    }
    
    //Cough detection part
    if(Ismotion==1&&sound_value>50&&pressure>103.5){
          count_cough++;
    }
    //Determine whether this minute is a cough minute, if so, record it, otherwise, don't record it.
    //Set cough level
    if(count_tag==60){
      count_tag=0;
      status["TIMESTAMP"] = timestamp;      
      if(count_cough>=20){   
        cough_minutes++;
        if(cough_minutes>=3){
          if(cough_minutes>=5){
            status["CoughLevel1"]=3;  //Level 3 over 5 minutes
          }
          else{
            status["CoughLevel1"]=2;  //Level 2 over 3 minutes
          }
        }
        else{
          status["CoughLevel1"]=1;   //Cough, but less than three minutes, grade 1.
        }
      }
      else{
        status["CoughLevel1"] = 0;  //don't cough in the current minute.
        if(cough_minutes)
        {
          status["Cough_Minutes"] = cough_minutes;   //No upload required.
        }
        cough_minutes=0;
      }
      Serial.println(msg);
      serializeJson(jsonDoc, msg, 250);
      if(!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");}
      jsonDoc.clear();
      JsonObject payload = jsonDoc.to<JsonObject>();
      JsonObject status = payload.createNestedObject("d");
      strcpy(msg,newmsg);
    }
    /*
    //Coma detection
    if(Ismotion==0&&sound_value<10&&pressure>103.5){  //The same judgment logic
      coma_tag++;
      if(coma_tag>=300){  //After more than five minutes, the buzzer sounds, and then the accumulated time.
        tone(buzzer, 500);
        buzzer_tag++
      }
      if(buzzer_tag>=300){
        //Remind his roommate to use another buzzer.
        tone(buzzer2,1000); 
        status["TIMESTAMP"] = timestamp;  //Upload coma records
        status["IsComa"] = 1; 
        Serial.println(msg);
        serializeJson(jsonDoc, msg, 250);
        if(!mqtt.publish(MQTT_TOPIC, msg)) {
        Serial.println("MQTT Publish failed");}
        jsonDoc.clear();
        JsonObject payload = jsonDoc.to<JsonObject>();
        JsonObject status = payload.createNestedObject("d");
        strcpy(msg,newmsg);
      }
    }
    else{
      coma_tag=0; //To ensure continuous detection
    }*/
    
    if(IsMagnetic!=0){
      count_Magn++;
      Magn_temp=1;
    }
    
    
    //The time and frequency of uploading traditional data should be controlled by return parameters.
    if(count_tag2==send_frequency){
      count_tag2=0;
      Serial.print("count_tag2 ");
      Serial.println(count_tag2);
      status["TIMESTAMP"] = timestamp;
      temperature=temperature-4;
      status["Temperature"] = String(temperature,2);
      status["Humidity"] = String(humidity,2);
      //status["Tempdiff"] = Tempdiff_temp;
      status["Tempdiff"] = 1;
      //status["Tempdiffnumber"] =Tempdiffnumber_temp;
      status["Tempdiffnumber"] =12;
      //status["Tempfit"] = Tempfit_temp;
      status["Tempfit"] = 1;
      //status["Tempfitcount"] = Tempfitcount_temp;
     status["Tempfitcount"] = 10;
      /*if(Magn_temp==1){
        status["IsMagnetic"] =1;
        Magn_temp=0;
      }
      else{
        status["IsMagnetic"] =0;
      }*/
      status["IsMagnetic"] =0;
      //status["Magn_Count"] = count_Magn;
      status["Magn_Count"] = 3;
      status["label"] = 0;
      status["note"] = "None";
      
      Serial.println(msg);
      serializeJson(jsonDoc, msg, 250);
      if(!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");}
      jsonDoc.clear();
      JsonObject payload = jsonDoc.to<JsonObject>();
      JsonObject status = payload.createNestedObject("d");
      strcpy(msg,newmsg);
      
    }
    }
    
/*
    if(status!=status_temp){
      status_temp=status;
      Serial.println(msg);
      serializeJson(jsonDoc, msg, 250);
      if(!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");}
    }
    */
    /*if(msg!=msg_temp){
      msg_temp=msg;
      Serial.println(msg);
      serializeJson(jsonDoc, msg, 250);
      if(!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");}
    }*/

    //Serial.println(count_tag2);
    //status["timestamp"] = timestamp;
    //status.clear();
    //strcpy(msg, newmsg);
    

   /*  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 4; i++) {
    mqtt.loop();
    delay(1000);
  } */
  //msg=new byte[250];
  //strcpy(msg, newmsg);
  delay(3000);
  noTone(buzzer);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
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

//Interrupt function is used for the R1's own operation alarm
void InterruptProc() {
    digitalWrite(ledred, HIGH);
    delay(5000);
    //digitalWrite(ledred, LOW);
    tone(buzzer, 500);
    delay(5000);
  Serial.println("Interrupt！！！");

}
/* Interrupt function 2 is used for coma detection. R1 needs to press a button to stop the buzzer.
void InterruptProc2(){
  coma_tag=0;
  buzzer_tag=0；
  noTone(buzzer);
}*/
