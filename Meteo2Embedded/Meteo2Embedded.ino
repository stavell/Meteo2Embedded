/// 
/// @mainpage	Meteo2Embedded 
///
/// @details	<#details#>
/// @n 
/// @n 
/// @n @a	Developed with [embedXcode](http://embedXcode.weebly.com)
/// 
/// @author	Stavel
/// @author	Shumen-XC
/// @date	27.04.13 11:27
/// @version	<#version#>
/// 
/// @copyright	© Stavel, 2013 г.
/// @copyright	CC = BY NC SA
///
/// @see	ReadMe.txt for references

#include <avr/wdt.h>

#include "Arduino.h"
#include "Wire.h"

#include "Timer.h"
//#include "Adafruit_BMP085.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP085_U.h"
#include "DHT.h"

#include <avr/wdt.h>

#define DHTPIN A2
#define DHTTYPE DHT22

#define BT_WAIT_PINGS 20

extern HardwareSerial Serial;

Timer timer;
//Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);


uint8_t aliveLed1   = 10;
uint8_t aliveLed2   = 11;

uint8_t btPower1     = 12;
uint8_t btPower2     = 13;
uint8_t btStatus     = 9;

uint8_t propeller   = 2;
uint8_t wind0       = 4;
uint8_t wind1       = 5;
uint8_t wind2       = 6;
uint8_t wind3       = 7;
uint8_t windOK      = 8;


unsigned long propellerCount = 0;
float humidity = 0;
uint8_t windDir = 0;

void onEvery500ms();
void onEverySecond();
void onEveryTenSeconds();
void onEveryMinute();

void onPropellerInterrupt();
unsigned long getPropellerCount();

void measureTemperature();
void measurePresure();
void measureHumidity();

uint8_t readWindDirection();

void pingBluetoothLine();
void resetBluetoothModule();
void handleSerialMessage();

int pong = BT_WAIT_PINGS;

String serialBuffer = "";
boolean stringComplete = false;


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);


void displaySensorDetails(void)
{
    sensor_t sensor;
    bmp.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}


void setup() {
    wdt_enable(WDTO_8S);

    bmp.begin();
    

    
    Serial.begin(9600);
    
    pinMode(aliveLed1, OUTPUT);
    pinMode(aliveLed2, OUTPUT);
    
    pinMode(btStatus, INPUT);
   
    pinMode(wind0, INPUT);
    pinMode(wind1, INPUT);
    pinMode(wind2, INPUT);
    pinMode(wind3, INPUT);
    
    pinMode(btPower1,OUTPUT);
    pinMode(btPower2,OUTPUT);
    
    digitalWrite(btPower1, HIGH);
    digitalWrite(btPower2, HIGH);
    
    serialBuffer.reserve(64);
    
    attachInterrupt(0, onPropellerInterrupt, RISING);
    
    bmp.begin();
    dht.begin();
    
    timer.every(500, onEvery500ms);
    timer.every(1000, onEverySecond);
    timer.every(10000, onEveryTenSeconds);
    timer.every(60000, onEveryMinute);
        
    timer.oscillate(aliveLed1, 250, HIGH);
    timer.oscillate(aliveLed2, 250, LOW);
    
    humidity = dht.readHumidity(); 
}


void loop() {
    timer.update();
}

void onEvery500ms() {
    wdt_reset();
    
    sensors_event_t event;
    bmp.getEvent(&event);
    
    /* Display the results (barometric pressure is measure in hPa) */
    if (event.pressure)
    {
        /* Display atmospheric pressue in hPa */
        Serial.print("Pressure:    ");
        Serial.print(event.pressure);
        Serial.println(" hPa");
    }
    displaySensorDetails();
    
    
//    if(digitalRead(btPower1) && digitalRead(btPower2)) pingBluetoothLine();
}

void pingBluetoothLine() {
    Serial.println("AT");
    if(pong) {
        pong--;
    } else {
        resetBluetoothModule();
    }
}


void resetBluetoothModule() {
    timer.pulse(btPower1, 1500, HIGH);
    timer.pulse(btPower2, 1500, HIGH);
    pong = BT_WAIT_PINGS;
}

void serialEvent() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        serialBuffer += inChar;
        if (inChar == '\n') {
            stringComplete = true;
            handleSerialMessage();
        } 
    }
}



void handleSerialMessage() {
    //test for pong
    if(serialBuffer.indexOf("OK") > -1) {
        pong = 10;
    }
        
    serialBuffer = "";
    stringComplete = false;
}

void onEverySecond() {
//    Serial.print("p:");
//    Serial.print(bmp.readPressure());
//    Serial.print(",");
//    Serial.print("t:");
//    Serial.print(bmp.readTemperature());
//    Serial.print(",");
//    Serial.print("h:");
//    Serial.print(humidity);
//    Serial.print(",");
//    Serial.print("w:");
//    Serial.print(readWindDirection());
//    Serial.print(",");
//    Serial.print("s:");
//    
//    Serial.println(getPropellerCount());
}

void onEveryTenSeconds() {
}

void onEveryMinute() {
    humidity = dht.readHumidity();
}

void onPropellerInterrupt() {
    propellerCount++;
}

unsigned long getPropellerCount() {
    unsigned long tmp = propellerCount;
    propellerCount = 0;
    return tmp;
}

uint8_t readWindDirection() {
    windDir  = 0;
    
    windDir |= digitalRead(wind0) << 0;
    windDir |= digitalRead(wind1) << 1;
    windDir |= digitalRead(wind2) << 2;
    windDir |= digitalRead(wind3) << 3;
    
    return windDir;
}
 