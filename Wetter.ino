/*
  ccs811basic.ino - Demo sketch printing results of the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library
#include "ClosedCube_HDC1080.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>


// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3
ClosedCube_HDC1080 hdc1080;
ESP8266WiFiMulti WiFiMulti;

// Wiring for Nano: VDD to 3v3, GND to GND, SDA to A4, SCL to A5, nWAKE to 13
//CCS811 ccs811(13); 

// nWAKE not controlled via Arduino host, so connect CCS811.nWAKE to GND
//CCS811 ccs811; 

// Wiring for ESP32 NodeMCU boards: VDD to 3V3, GND to GND, SDA to 21, SCL to 22, nWAKE to D3 (or GND)
//CCS811 ccs811(23); // nWAKE on 23

// WiFi Config
#define WiFi_SSID "Wifi"
#define WiFi_Password "PW"

void setup() {
 Serial.begin(115200);
  delay(10);
  Serial.println("");

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WiFi_SSID, WiFi_Password);
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // hdc1080 info
  hdc1080.begin(0x40);
  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, nWAKE to D3, SDA to D2, SCL to D1]
  Wire.begin(4, 5);
  //Wire.begin();

  Serial.println("CCS811 test");
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if ( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);

  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("init: CCS811 start FAILED");
}


void loop() {
  // Read
    uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw);
  if ( errstat == CCS811_ERRSTAT_OK ) {

    Serial.print("\n\ntemperature: ");
    Serial.print(hdc1080.readTemperature());
    Serial.print(" C");

    Serial.print("\nhumidity: ");
    Serial.print(hdc1080.readHumidity());
    Serial.print(" %");

    Serial.print("\neCO2 concentration: ");
    Serial.print(eco2);
    Serial.print(" ppm");

    Serial.print("\nTVOC concentration: ");
    Serial.print(etvoc);
    Serial.print(" ppb");

  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
  
  // Wait
  delay(1000); 
}
