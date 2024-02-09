#include <ESP8266WiFi.h>   
#include <PubSubClient.h>  
#include <string.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>


// const char* ssid = "Huflit-GV";         
// const char* password = "gvhuflit@123";  

const char* ssid = "Sho0_";         
const char* password = "sownnnnn";  

// const char* ssid = "Lucid Coffee 2.4G";        
// const char* password = "lucidcoffee";  

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "Sho";
const char* mqttPassword = "1234";


const int ledpin = D1;  
const int motionpin = D7;
const String name = "3";

char messageBuff[100]; 

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);  

  pinMode(ledpin, OUTPUT);
  pinMode(motionpin, INPUT);


  startWiFi();

  connectBroker();

  client.subscribe("sho/temp");
  client.publish("sho/temp", "Connected");
}

void loop() {
  if (!client.connected()) {
    connectBroker();
  }
 
  client.loop();
}

void startWiFi() {
  WiFi.begin(ssid, password);  
  Serial.print("Connecting to ");
  Serial.print(ssid);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n");
  Serial.println("Connection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}

void connectBroker() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("BlockyNodeClientID")) { 
      Serial.println("\n");
      Serial.println("MQTT connected");
    } else {
      Serial.println("\n");
      Serial.print("MQTT failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i;
  for (i = 0; i < length; i++) {
    messageBuff[i] = (char)payload[i];
  }
  messageBuff[i] = '\0';

  String message = String(messageBuff);
  Serial.println(message);
  DynamicJsonDocument doc(256);
  deserializeJson(doc, message);

  if (message.indexOf("Mode") >= 0) {
    String leds = doc["Led"];
    int level = doc["Level"];

    if (leds.indexOf(name) >= 0){
        analogWrite(ledpin, level);
      }
    if (leds.indexOf("all") >= 0){
        analogWrite(ledpin, level);
      }
    }
  else {
    int led = doc["Led"];
    Serial.println("Led: " + String(led));
    Serial.println(" Motion: " + String(digitalRead(motionpin)));
    if (led == 50) {
      if (digitalRead(motionpin) == 1) {
        led = 255;
      } else {
        led = 50;
      }
      analogWrite(ledpin, led);
    } else {
      analogWrite(ledpin, led);
    }
  }
}
