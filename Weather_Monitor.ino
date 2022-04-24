#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME680.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>

const char* ssid     = "Tim's Network"; // ESP32 and ESP8266 uses 2.4GHZ wifi only
const char* password = "carlsagan"; 

//MQTT Setup Start

#include <PubSubClient.h>
//#define mqtt_server "192.168.7.238"
#define mqtt_server "192.168.7.214"
WiFiClient espClient;
PubSubClient client(espClient);


#define mqttTemp1 "chickenCoop/temp1"
#define mqttHum1 "chickenCoop/hum1"
#define mqttPress1 "chickenCoop/press1"
#define mqttGas1 "chickenCoop/gas1"
#define mqttLight1 "chickenCoop/light1"

#define mqttTemp2 "outside/temp2"
#define mqttHum2 "chickenCoop/hum2"
#define mqttPress2 "chickenCoop/press2"
#define mqttGas2 "chickenCoop/gas2"
#define mqttLight2 "outside/light2"

//MQTT incoming

//MQTT Setup End

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

Adafruit_BME680 bme1; // I2C
//Adafruit_BME680 bme2; // I2C
//Adafruit_BME680 bme2(BME_CS); // hardware SPI
Adafruit_BME680 bme2(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

const byte lightPin1 = 34;
const byte lightPin2 = 35;
float light1;
float light2;
float TF;

float temp1, hum1, press1, gas1, temp2, hum2, press2, gas2;

unsigned long millisNow = 0; //for delay purposes
unsigned int sendDelay = 10000; //delay before sending sensor info via MQTT

const int tempThresholdHigh = 140;
const int tempThresholdLow = -20;



void setup() {
  Serial.begin(9600);
  Serial.println();
  
  // begin Wifi connect
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //end Wifi connect

  client.setServer(mqtt_server, 1883);
  //client.setCallback(mqttCallback);
  
  delay(5000);
  
  unsigned status;
  //status = bme1.begin(0x76);
  status = bme2.begin(0X76); 
}

bool getValues() {

  temp1 = bme1.readTemperature()*9/5 + 32;
  temp2 = bme2.readTemperature()*9/5 + 32;
 
  hum1 = bme1.humidity;
  hum2 = bme2.humidity;
 
  press1 = bme1.pressure/100.0;
  press2 = bme2.pressure/100.0;
  gas1 = bme1.gas_resistance / 1000.0;
  gas2 = bme2.gas_resistance / 1000.0; 
  
  Serial.print("BME 1 Temperature = ");
  Serial.print(temp1);
  Serial.println(" °F");

  Serial.print("BME 1 Pressure = ");
  Serial.print((bme1.pressure) / 100.0F);
  Serial.println(" hPa");

  Serial.print("BME 1 Humidity = ");
  Serial.print(hum1);
  Serial.println(" %");

  Serial.print ("BME 1 Gas = ");
  Serial.print (bme1.gas_resistance / 1000.0);
  Serial.println (" KOhms");

  Serial.println();

  Serial.print("BME 2 Temperature = ");
  Serial.print(temp2);
  Serial.println(" °F");

  Serial.print("BME 2 Pressure = ");
  Serial.print(bme2.pressure / 100.0F);
  Serial.println(" hPa");

  Serial.print("BME 2 Humidity = ");
  Serial.print(bme2.humidity);
  Serial.println(" %");

  Serial.print("BME 2 Gas = ");
  Serial.print(bme2.gas_resistance / 1000.0);
  Serial.print(" KOhms");
  Serial.println();
  Serial.println();

  
  light1 = analogRead(lightPin1); //0-4095 12bit -- esp8266 10bit 0-1023 -- arduino 8bit 0-254
  light1 = light1/4095 * 100;
  Serial.print("Light reading 1 = ");
  Serial.print(light1);
  Serial.println(" %");
  Serial.println();

  light2 = analogRead(lightPin2); //0-4095 12bit -- esp8266 10bit 0-1023 -- arduino 8bit 0-254

  Serial.print("Light reading 2 =  ");
  Serial.println(light2);
  Serial.println();
  Serial.println();

  if (temp2 > tempThresholdHigh||temp2 < tempThresholdLow){
    return 0;
  }else{
    return 1;   
  }
 
}

void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!client.connected()) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
   
    if (client.connect("chickenCoopController")) {
      Serial.println("connected");
      //topicsSubscribe();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  //topicsSubscribe();
}


void loop() {
  if (!client.connected()){
    reconnect();
  }
  
  if (millis() > millisNow + sendDelay){
   if (getValues()) {
  client.publish(mqttTemp1, String(temp1).c_str(),true);
  client.publish(mqttHum1, String(hum1).c_str(),true);
  client.publish(mqttTemp2, String(temp2).c_str(),true);
  client.publish(mqttHum2, String(hum2).c_str(),true);
  client.publish(mqttPress1, String(press1).c_str(),true);
  client.publish(mqttPress2, String(press2).c_str(),true);
  client.publish(mqttGas1, String(gas1).c_str(),true);
  client.publish(mqttGas2, String(gas2).c_str(), true);
  client.publish(mqttLight1, String(light1).c_str(),true);
  client.publish(mqttLight2, String(light2).c_str(),true);
  
  millisNow = millis(); }
   
   }
   client.loop();
}
  
