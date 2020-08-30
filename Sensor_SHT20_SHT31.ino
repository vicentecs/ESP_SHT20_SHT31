/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>

#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "DFRobot_SHT20.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();
DFRobot_SHT20  sht20;
BlynkTimer timer;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char BlynkAuth[] = "XXXXXXXXXXX";

void sendSensor()
{
  // put your main code here, to run repeatedly:
  float h20 = sht20.readHumidity();
  float t20 = sht20.readTemperature();
  
  float t31 = sht31.readTemperature();
  float h31 = sht31.readHumidity();
  
  if (! isnan(t20)) { // check if 'is not a number'
    Serial.print("Temp 20 *C = "); Serial.println(t20);
    Blynk.virtualWrite(V0, t20);    
  } else {
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h20)) { // check if 'is not a number'
    Serial.print("Hum. 20 % = "); Serial.println(h20);
    Blynk.virtualWrite(V1, h20);
  } else {
    Serial.println("Failed to read humidity");
  }

  if (! isnan(t31)) { // check if 'is not a number'
    Serial.print("Temp 31 *C = "); Serial.println(t31);
    Blynk.virtualWrite(V2, t31);
  } else {
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h31)) { // check if 'is not a number'
    Serial.print("Hum. 31 % = "); Serial.println(h31);
    Blynk.virtualWrite(V3, h31);
  } else {
    Serial.println("Failed to read humidity");
  }
  Serial.println();

  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "senha123")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  WiFi.setAutoReconnect(true);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("Local ip");
  Serial.println(WiFi.localIP());

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1)  delay(1);
  }
  
  sht20.initSHT20(); // Init SHT20 Sensor
  delay(100);
  sht20.checkSHT20(); // Check SHT20 Sensor

    // Initialize Blynk, and wait for a connection before doing anything else
  Serial.println("Connecting to Blynk Server");
  Blynk.config(BlynkAuth);
  while (!Blynk.connected())
    Blynk.run();
  Serial.println("Blynk connected! starting.");

  // Setup a function to be called every second
  timer.setInterval(10000L, sendSensor);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
  ArduinoOTA.handle();
  Blynk.run();
  timer.run();
}
