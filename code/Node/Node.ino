#include <ESP8266WiFi.h>  
#include <PubSubClient.h> 
#include <string.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>


// const char* ssid = "Huflit-GV";       
// const char* password = "gvhuflit@123";  

// const char* ssid = "Lucid Coffee 2.4G";      
// const char* password = "lucidcoffee"; 

const char* ssid = "Sho0_";       
const char* password = "sownnnnn"; 

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "Sho";
const char* mqttPassword = "1234";
const String name = "1";

int led;
long water;
float t;
float h;
char buffer[256];
StaticJsonDocument<300> doc;
String mode = "auto";



SoftwareSerial serial_ESP(D2, D3);

#define ledpin D1
#define motionpin D7


char messageBuff[100]; 

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(ledpin, OUTPUT);
  pinMode(motionpin, INPUT);

  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT);
  Serial.begin(9600);
  serial_ESP.begin(115200);

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
  if (mode == "auto"){
    if (serial_ESP.available()) {
      DeserializationError err = deserializeJson(doc, serial_ESP);
      String dataRecive = serial_ESP.readString();

      serializeJson(doc, buffer);

      if (err == DeserializationError::Ok) {
        Serial.println("Led: " + String(doc["Led"].as<float>()) + " | Water: " + String(doc["Water"].as<float>()) + " |  Temp: " + String(doc["Temp"].as<float>()) + " |  Humd: " + String(doc["Humid"].as<float>()));
      }
      led = doc["Led"];
      int motion = digitalRead(motionpin);
      Serial.println("Motion: " + String(motion));
      if (led == 50) {
        if (motion == 1) {
          led = 255;
        }
      }
      client.publish("sho/temp", buffer);
    }
  }
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
    if (client.connect("ShoPucID")) { 
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

  if (message.indexOf("control") >= 0) {
    deserializeJson(doc, message);
    mode = "control";
    String leds = doc["Led"];
    int level = doc["Level"];
    Serial.println("CONTROL");
    Serial.println(leds);
    Serial.println(String(level));


    if (leds.indexOf(name) >= 0) {
      analogWrite(ledpin, level);
    }
    if (leds.indexOf("all") >= 0) {
      analogWrite(ledpin, level);
    }
  } 
  else if (message.indexOf("auto") >= 0){
    mode = "auto";
  }
  if (mode=="auto"){
    analogWrite(ledpin, led);
  }
}
