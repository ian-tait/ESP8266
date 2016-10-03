//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 MQTT temp sensor node


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ThingSpeak.h"

//create 1-wire connection on pin 2 and connect it to the dallasTemp library
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "###"
const char* ssid = "###";
const char* password = "###";


//topic to publish to for the temperature
char* tempTopic = "NurseryTemperature";


WiFiClient wifiClient;
PubSubClient psClient(MQTT_SERVER, 1883, callback, wifiClient);

static unsigned long myChannelNumber = ####;
static const char 	*myWriteAPIKey = "#####";

void setup() {

  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);
  
  //start wifi subsystem
  WiFi.begin(ssid, password);
  
  ThingSpeak.begin(wifiClient);	// initialize ThingSpeak lib
  
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  //start the temperature sensors
  sensors.begin();

  //wait a bit before starting the main loop
  delay(500);
	
}



void loop(){

  // Send the command to update temperatures
  sensors.requestTemperatures(); 

  //get the new temperature
  float currentTempFloat = sensors.getTempCByIndex(0);

  //convert the temp float to a string and publish to the temp topic
  char temperature[10];
  dtostrf(currentTempFloat,4,3,temperature);
  Serial.println(temperature);
  psClient.publish(tempTopic, temperature);
  ThingSpeak.writeField(myChannelNumber, 1, temperature, myWriteAPIKey);

  Serial.println(psClient.connected());
  Serial.println(WiFi.status());

  //reconnect if connection is lost
  if (!psClient.connected() && WiFi.status() == 3) {
    Serial.println("Connection lost... Reconnecting...");
    reconnect();
  }
  //maintain MQTT connection
  psClient.loop();
  //MUST delay to allow ESP8266 WIFI functions to run
  delay(2500); 
}


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {}








//networking functions

void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    //debug printing
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() == WL_CONNECTED) {
    // Loop until we're reconnected to the MQTT server
    while (!psClient.connected()) {


      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      Serial.print("MAC address: ");
      Serial.println(clientName);
           
      Serial.print("Attempting MQTT connection...");
      //if connected, subscribe to the topic(s) we want to be notified about
      if (psClient.connect((char*) clientName.c_str())) {
        Serial.println("\tMTQQ Connected");
        //psClient.subscribe(lightTopic);
      }

      //otherwise print failed for debugging
      else {
        Serial.println("\tFailed.");
        abort();
      }
    }
  }
}


//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}


