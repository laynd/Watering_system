
/* Watering system with 2 separate sub-systems. First sub-system based on two soil moisture sensors (2 for redundancy), second sub-system 
 *  based on water level sensor (boxes with water containers). Built in fail safe in case of soil moisture sensor malfunction. If  
 *  malfunction detected system goes into emergency operation. Watering for 1 hour every 6 hours. Both sub-systems. Designed to work 
 *  with secondary Arduino board which is running i2c amoled screen. Communication via serial bus. There is known bug: sensor readings 
 *  cannot be less than 3 digits. Otherwise order of displayed information is off.  
 *  
 *  Copyright by Daniel Lameka. 6/27/2015
 *
 *  01/23/2016 New Changes. Adding real time module & keypad array 4x4. Due to issues with moisture sensor will need to switch to time based watering.
 */
 
#include "Wire.h"   // for time module lib
#include "RTClib.h" // time module lib
#include "Keypad.h"

#define DS1307_ADDRESS 0x68
#define TIME 1500 // 1500 ms = 10 min delay
#define CNT 400
#define BAUD 9600
#define ON LOW
#define OFF HIGH
#define WATERLEVELS 1
#define WATERLVLRELAY 4
#define SOILS1 2
#define SOILS2 3
#define SOILSRELAY 3

byte zero = 0x00; // workaround for issue #52 time module related
RTC_DS1307 RTC;   // time module related

// keypad initialization
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 11, 10, 9}; //connect to the column pinouts of the keypad
// initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// keypad initialization

char h1, h2, m1, m2, key;
int hn1, hn2, mn1, mn2, hStart, mStart, hStop, mStop, min2GO;
int a=0;
int i=0;
int currentHour=0;
int currentMinute=0;
int s=0;
int sensor1=0;
int sensor2=0;
int average=0;
int level=0;
int dataMax=0;
int dataMin=0;
long sensor1a=0;
long sensor2a=0;
int critical=0;
int criticaltest=0;
long y1=0;
long x1=0;
int testing=0;
int trigger=0;
int time1=0;
int lvl=0;
int lvltrigger=0;
int counter=0;

void setup() {
 
 Serial.begin(BAUD);
 RTC.begin(); 
 if (! RTC.isrunning()) {Serial.println("RTC is NOT running!");}
 
 keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
 
 critical=0;
 dataMin=900;
 counter=400;
 pinMode(SOILSRELAY, OUTPUT);
 pinMode(WATERLVLRELAY, OUTPUT);
 digitalWrite(SOILSRELAY, OFF);
 digitalWrite(WATERLVLRELAY, OFF);
 pinMode(13, OUTPUT);
 digitalWrite(13, OFF); 
 lvltrigger=0;
 level=1; //initialization with assumption that tank is full
 counter=0;
 trigger=0;
 hStart=0;
 hStop=0;
 mStart=0;
 mStop=0;
 key='K';
 
 hStart=0;
 mStart=2;
 hStop=1;
 mStop=1;
 min2GO=0;

  
 
 Serial.print("@");

  
 }

void loop() {
  // time module testing block of code
   /* DateTime now = RTC.now();
    currentHour=now.hour();
    currentMinute=now.minute();
    
    
    Serial.print(currentHour);
    Serial.print("h: ");
    Serial.print(currentMinute);
    Serial.print("m: ");

    */
    
    // time module testing block of code
    
    key = keypad.getKey(); // reading keypad
    if(key)  // Check for a valid key.
    {

      Serial.println(key);
    }
      /*switch (key)
      {
        case 'A': // adjusting time if time is incorrect in time module
          Serial.print("A");
          h1=keypad.waitForKey(); Serial.print(h1); hn1=char2number(h1);
          h2=keypad.waitForKey(); Serial.print(h2); hn2=char2number(h2);
          m1=keypad.waitForKey(); Serial.print(m1); mn1=char2number(m1);
          m2=keypad.waitForKey(); Serial.print(m2); mn2=char2number(mn2);
          setTime(combiner(hn1,hn2), combiner(mn1,mn2));
          Serial.print("&");
          break;
        case 'B': // setting start time of watering
          Serial.print("B");
          h1=keypad.waitForKey(); Serial.print(h1); hn1=char2number(h1);
          h2=keypad.waitForKey(); Serial.print(h2); hn2=char2number(h2);
          m1=keypad.waitForKey(); Serial.print(m1); mn1=char2number(m1);
          m2=keypad.waitForKey(); Serial.print(m2); mn2=char2number(mn2);
          hStart=combiner(hn1, hn2);
          mStart=combiner(mn1, mn2);
          Serial.print("&");
          break;
        case 'C': // setting stop time of watering
          Serial.print("C");
          h1=keypad.waitForKey(); Serial.print(h1); hn1=char2number(h1);
          h2=keypad.waitForKey(); Serial.print(h2); hn2=char2number(h2);
          m1=keypad.waitForKey(); Serial.print(m1); mn1=char2number(m1);
          m2=keypad.waitForKey(); Serial.print(m2); mn2=char2number(mn2);
          hStop=combiner(hn1, hn2);
          mStop=combiner(mn1, mn2);
          Serial.print("&");
          break;
        case 'D': // enable/disable watering 
          Serial.print("D");
          h1='D';
          while (h1=='D'){
           //h1=keypad.getKey();
           Serial.print(h1);
          }
          Serial.print("&");
          break;
          
        default:
         break;
      }
    }
    
    min2GO=minutes2go(hStart, mStart, hStop, mStop);
    
    Serial.print(min2GO);
    h1=keypad.waitForKey();
    
    
    /*
    if (currentHour>=hStart && currentHour<=hStop) {
      if (currentMinute>=mStart && currentMinute<=mStop){
      digitalWrite(SOILSRELAY, ON);
      digitalWrite(13, OFF);
      Serial.print("ON");
       }
      else {
        digitalWrite(SOILSRELAY, OFF);
        digitalWrite(13, ON);
        Serial.print("OFF");
      }
    }
    */ 
    //Serial.println();    
    
    
   // delay(1000);
// sending data to 2nd unit with display or PC over serial bus
    /*Serial.print("@");
    delay(1);
    Serial.print(sensor1);
    Serial.print(sensor2);
    Serial.print(average);
    Serial.print(dataMin);
    Serial.print(dataMax);
    Serial.print(level);
    */
// sending data to 2nd unit with display or PC over serial bus

// time module based operation


/*    
// Sensor related code 
  if (critical==0){
// sensor reading for 10 min. 10 min interval used in watering counters
    x1=0;
    y1=0;
    for (i = 0; i < CNT; i++){
       delay(TIME); 
       sensor1=analogRead(SOILS1);
       sensor2=analogRead(SOILS2);
       sensor1a=x1+sensor1;
       x1=sensor1a;
       sensor2a=y1+sensor2;
       y1=sensor2a;
    }
    sensor1=x1/CNT;
    sensor2=y1/CNT;
    average=avg(sensor1, sensor2);
    critical=test(sensor1, sensor2);
    if (average>dataMax){dataMax=average;}
    if (average<dataMin){dataMin=average;}
delay(1);
// watering based on soil moisture level. Trigger at level 350 until reaching 600. Watering 1 hour every hour until goal reached
    if(average<=350 && trigger==0){trigger++;}
    if(trigger>0){
      counter++;
      if(counter<=6){digitalWrite(SOILSRELAY, ON);}
      else{
        if(counter<=12){digitalWrite(SOILSRELAY, OFF);}
        else{
          if(counter>12 && average<=600){counter=0;}
          else{
           counter=0;
           trigger=0;
          }
       }
      }
    }
delay(1);    
// watering boxes based on water level sensor readings. watering for 1 hour after 1 hour delay when triggered
    if(lvltrigger==0 && level==1){
      lvl=analogRead(WATERLEVELS);
      digitalWrite(WATERLVLRELAY, OFF);
      }
    if(lvl==0){lvltrigger++;}
    if(lvltrigger>0){
       level=0;
       if(lvltrigger>6){digitalWrite(WATERLVLRELAY, ON);}
       else{digitalWrite(WATERLVLRELAY, OFF);}
       if(lvltrigger==12){
         lvltrigger=0;
         level=1;
       }
    }
*/
  }
  
// in case of sensor malfunction system goes into timer mode. Watering both systems every 6 hours for 1 hour  
/*  else{
    counter=0;
    digitalWrite(SOILSRELAY, OFF);
    digitalWrite(WATERLVLRELAY, OFF);
    for(i=0; i<5; i++){Serial.print("&"); delay(10);}
    while(1){
      counter++;
      Serial.print("&"); // second unit recognizes malfunction message
      for(a=0; a<7; a++){for (i = 0; i < CNT; i++){delay(TIME);}} // 1 hour interval
      if(counter==6){
        digitalWrite(SOILSRELAY, ON);
        digitalWrite(WATERLVLRELAY, ON); 
        for(a=0; a<7; a++){for (i = 0; i < CNT; i++){delay(TIME);}} // 1 hour interval
        digitalWrite(SOILSRELAY, OFF);
        digitalWrite(WATERLVLRELAY, OFF);
        counter=0;
      }
    }
  }
*/ 
 
 
 
}

int minutes2go(int startHour, int startMin, int stopHour, int stopMin){
  int v1=0;
  int v2=0;
  int v3=0;
  v1=startHour*60+startMin;
  v2=stopHour*60+stopMin;
  return v3=v2-v1;
}

int char2number(char xyz){
  int a1=0;
  switch (xyz)
      {
        case '1': a1=1; break;
        case '2': a1=2; break;
        case '3': a1=3; break;
        case '4': a1=4; break;
        case '5': a1=5; break;
        case '6': a1=6; break;
        case '7': a1=7; break;
        case '8': a1=8; break;
        case '9': a1=9; break;
        case '0': a1=0; break;
        default: a1=0;  break;
      }
      return a1;
}

int combiner(int x, int y){
  int comb=0;
  return comb=x*10+y;
}

int avg(int x, int y){
  int a=0;
  int i=0;
  i=x+y;
  a=i/2;
  return a;
}

int test(int x, int y){
  int a=0;
  if(x>y){a=x-y;}else{a=y-x;}
  if(a<200){return 0;}else{return 1;}
}

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

void setTime(byte hour, byte minute) {

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(zero); //start
  Wire.endTransmission();

}

// Taking care of some special events.
void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
    case PRESSED:
        if (key == '#') {
            
        }
        break;

    case RELEASED:
        if (key == '*') {
            
            
        }
        break;

    case HOLD:
        if (key == '*') {
            
        }
        break;
    }
}
