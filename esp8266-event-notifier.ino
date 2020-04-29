/*
  2020-04-24
  Bryce Dombrowski
  
  Modified from https://github.com/SensorsIot/Reminder-with-Google-Calender
*/

#include <ESP8266WiFi.h>
//#include <Time.h>
//#include <NTPClient.h>
//#include <WiFiUdp.h>
#include "HTTPSRedirect.h"
#include "credentials.h"

// GLOBALS // ****************************************************************************

//Connection Settings
const char* HOST = "script.google.com";
const int HTTPS_PORT = 443;
HTTPSRedirect* CLIENT = nullptr;
const unsigned long RESET_TIME = 36000000; // 10 hours in milliseconds
unsigned long ENTRY_CALENDER;
unsigned long ENTRY_UPDATE;
unsigned long ENTRY_FREETIME;
unsigned int CURRENT_DISPLAY;
String CALENDAR_DATA = "";
bool CALENDAR_UP_TO_DATE;
bool BLINK = true;
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");

String LED_COLOUR = "000000";
const int PIN_RED = D1;
const int PIN_GREEN = D2;
const int PIN_BLUE = D3;
const int PIN_DATA = D5; // Shift register SER
const int PIN_CLOCK = D6; // Shift register SRCLK
const int PIN_LATCH = D7; // Shift register RCLK

#define UPDATETIME 1800000 // Once every half hour (in milliseconds)

#ifndef CREDENTIALS
const char* ssid = "........."; //replace with your ssid
const char* password = ".........."; //replace with your password
//Google Script ID
const char *GScriptIdRead = "............"; //replace with your gscript id for reading the calendar
#endif

// Possible calendar event colours
#define PALE_BLUE 1 // Lavender
#define PALE_GREEN 2 // Sage
#define MAUVE 3 // Grape
#define PALE_RED 4 // Flamingo
#define YELLOW 5 // Banana
#define ORANGE 6 // Tangerine
#define CYAN 7 // Peacock
#define GRAY 8 // Graphite
#define BLUE 9 // Blueberry
#define GREEN 10 // Basil
#define RED 11 // Tomato

enum eventTypes{
  // Event types are in order of importance
  InPerson = RED,
  VideoCall = BLUE,
  PhoneCall = YELLOW,
  None = GREEN
}EVENT_TYPE;
String NEXT_FREE_TIME = "00:00";

/*
const byte SEVEN_SEGMENT[] = { // Common cathode
  // DP G F E D C B A
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
};*/

const byte SEVEN_SEGMENT[] = { // Common anode
  // DP G F E D C B A (I hooked mine up backwards, but just use MSBFIRST instead of LSBFIRST)
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000, // 9
  0b10101011, // n (10)
  0b10100011, // o (11)
  0b10100111, // c (12)
  0b11111111, // nothing (13)
  0b10000011, // b (14)
  0b10000111  // t (15)
};



// FUNCTIONS // ****************************************************************************

void setColour (String colour){
  long temp = strtol(&colour[0], NULL, 16); // convert string (of a hex number) to a long
  int r = (temp >> 16) & 0xff;
  int g = (temp >> 8) & 0xff;
  int b = (temp >> 0) & 0xff;
  analogWrite(PIN_RED, r*4);
  analogWrite(PIN_GREEN, g*4);
  analogWrite(PIN_BLUE, b*4);
  Serial.println("#"+colour);
}

void setColourRGB (int r, int g, int b, int delayTime){ // range 0-255
  analogWrite(PIN_RED, r*4);
  analogWrite(PIN_GREEN, g*4);
  analogWrite(PIN_BLUE, b*4);
  delay(delayTime);
}

void rainbow(int secondsPerCycle){ // goes through the rainbow
  int delayTime = round(secondsPerCycle*1000/6/1023);
  int r = 255;
  int g = 0;
  int b = 0;
  for(g=0; g<255; g++){
    setColourRGB(r,g,b, delayTime);
  }
  for(r=255; r>0; r--){
    setColourRGB(r,g,b, delayTime);
  }
  for(b=0; b<255; b++){
    setColourRGB(r,g,b, delayTime);
  }
  for(g=255; g>0; g--){
    setColourRGB(r,g,b, delayTime);
  }
  for(r=0; r<255; r++){
    setColourRGB(r,g,b, delayTime);
  }
  for(b=255; b>0; b--){
    setColourRGB(r,g,b, delayTime);
  }
  setColourRGB(0,0,0, 0);
}

void displaySevenSegment(unsigned int data){ // Format of 32-bit binary number: 0b[8-bit MSB Hour][8-bit LSB Hour][8-bit MSB Minute][8-bit LSB Minute]
  if (CURRENT_DISPLAY == data) return; // Don't bother running if the display hasn't changed
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, (data >> 0) & 0xFF ); // LSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, (data >> 8) & 0xFF); // MSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, (data >> 16) & 0xFF); // LSB digit of hour
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, (data >> 24) & 0xFF); // MSB digit of hour
  digitalWrite(PIN_LATCH, HIGH); // Move value to display register
}

void displayTime(String timeString){ // Format "HH:MM" 24hr
  Serial.print("Time to display: ");
  Serial.println(timeString);
  /*
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  Serial.println(timeString);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[timeString.substring(4,5).toInt()] & ~(PM ? 0x80 : 0x00) ); // LSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[timeString.substring(3,4).toInt()]); // MSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[timeString.substring(1,2).toInt()]); // LSB digit of hour
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[timeString.substring(0,1).toInt()]); // MSB digit of hour
  digitalWrite(PIN_LATCH, HIGH); // Move value to display register
  */
  bool PM = false; // We are trying to find out if the post medidiem boolean needs to be set
  int hour = timeString.substring(0,timeString.indexOf(":")).toInt();
  if (hour>12){
    PM = true;
    hour -= 12;
  }
  String hourString = String(hour);
  int hourMSB, hourLSB;
  if (hourString.length()>1){ // Two-digit number
    hourMSB = SEVEN_SEGMENT[hourString.substring(0,1).toInt()];
    hourLSB = SEVEN_SEGMENT[hourString.substring(1,2).toInt()];
  }else{ // Single digit
    hourMSB = 0xFF; // nothing
    hourLSB = SEVEN_SEGMENT[hour];
  }
  unsigned int data = 0x00;
  data = hourMSB; // MSB digit of hour
  data = (data << 8) | hourLSB & ~(BLINK ? 0x80 : 0x00) /*Set bit DP if BLINK is true*/; // LSB digit of hour
  data = (data << 8) | SEVEN_SEGMENT[timeString.substring(3,4).toInt()]; // MSB digit of minute
  data = (data << 8) | SEVEN_SEGMENT[timeString.substring(4,5).toInt()] & ~(PM ? 0x80 : 0x00)/*Set bit DP if PM is true (common anode)*/; // LSB digit of minute
  
  displaySevenSegment(data);
}
void displayBoot(){ // Shows on time display "boot"
  unsigned int data = SEVEN_SEGMENT[14]; // "b"
  data = (data << 8) | SEVEN_SEGMENT[11]; // "o"
  data = (data << 8) | SEVEN_SEGMENT[11]; // "o"
  data = (data << 8) | SEVEN_SEGMENT[15]; // "t"
  displaySevenSegment(data);
}
void displayError(int delayTime = 1000){ // Shows on time display "no conn" and flashes the LED pink and mustard colour
  setColourRGB(255,0,166, 0); //Pink
  // Show on display "no  "
  unsigned int data = SEVEN_SEGMENT[10]; // "n"
  data = (data << 8) | SEVEN_SEGMENT[11]; // "o"
  data = (data << 16) | 0xFFFF; // nothing
  displaySevenSegment(data);
  delay(delayTime);

  setColourRGB(255,213,0, 0); //Mustard
  // Show on display "conn"
  data = 0x00;
  data = (data << 8) | SEVEN_SEGMENT[12]; // "c"
  data = (data << 8) | SEVEN_SEGMENT[11]; // "o"
  data = (data << 8) | SEVEN_SEGMENT[10]; // "n"
  data = (data << 8) | SEVEN_SEGMENT[10]; // "n"
  data = (data << 16) | 0xFFFF; // nothing
  displaySevenSegment(data);
  delay(delayTime);
  displayClear();
  
  /*
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0xFF); // LSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0xFF); // MSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[11]); // LSB digit of hour
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[10]); // MSB digit of hour
  digitalWrite(PIN_LATCH, HIGH); // Move value to display register
  

  setColourRGB(255,213,0, 0); //Mustard
  // Show on display "conn"
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[10]); // LSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[10]); // MSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[11]); // LSB digit of hour
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[12]); // MSB digit of hour
  digitalWrite(PIN_LATCH, HIGH); // Move value to display register
  delay(delayTime);*/
}

void displayClear(){ // Clear the seven segment display
  displaySevenSegment(0xFFFFFFFF);
}

//Connect to wifi
void connectToWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  CLIENT = new HTTPSRedirect(HTTPS_PORT);
  CLIENT->setInsecure();
  CLIENT->setPrintResponseBody(true);
  CLIENT->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(HOST);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = CLIENT->connect(HOST, HTTPS_PORT);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(HOST);
    for (int i=0; i<2; i++){ // Let user know that the device is having problems
      displayError();
      delay(500);
    }
    Serial.println("Exiting...");
    ESP.reset();
  }
  Serial.println("Connected to Google");
}

void getCalendar() {
  Serial.println("Start Request");
  // HTTPSRedirect CLIENT(HTTPS_PORT);
  unsigned long getCalenderEntry = millis();

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 2; i++) {
    int retval = CLIENT->connect(HOST, HTTPS_PORT);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(HOST);
    for(int i = 0; i<2; i++){
      displayError();
      delay(500);
    }
    Serial.println("Exiting...");
    ESP.reset();
  }
  //Fetch Google Calendar events
  String url = String("/macros/s/") + GScriptIdRead + "/exec";
  CLIENT->GET(url, HOST);
  CALENDAR_DATA = CLIENT->getResponseBody();
  Serial.print("Calendar Data---> ");
  Serial.println(CALENDAR_DATA);
  CALENDAR_UP_TO_DATE = true;
  yield();
}

void manageStatus() {
  /*switch (EVENT_TYPE) { // State machine
    case InPerson:
      setColourRGB(255,0,0, 0); // Red
      break;
    case VideoCall:
      setColourRGB(0,0,255, 0); // Blue
      break;
    case PhoneCall:
      setColourRGB(255,130,0, 0); // Yellow
      break;
    default:
      setColourRGB(0,255,0, 0); // Green // maybe add some variation in colour for fun (slight random variations)
      break;
  }*/
  if ((millis()/1000 < ENTRY_FREETIME) && (EVENT_TYPE != None)) {
    displayTime(NEXT_FREE_TIME);
  } else {
    displayClear();
  }
}

void classifyEvent(String calendar_data) {
  Serial.println(calendar_data);
  // Finds the highest priority event, then sets "EVENT_TYPE" to that value. (Like a state machine)
  
  int tildeIndex = calendar_data.indexOf("~");
  int inPersonIndex = calendar_data.indexOf(String(InPerson));
  int videoCallIndex = calendar_data.indexOf(String(VideoCall));
  int phoneCallIndex = calendar_data.indexOf(String(PhoneCall));
  
  if ( (inPersonIndex >= 0) && (inPersonIndex < tildeIndex) ){
    EVENT_TYPE = InPerson;
    setColourRGB(255,0,0, 0); // Red
  } else if ( (videoCallIndex >= 0) && (videoCallIndex < tildeIndex) ){
    EVENT_TYPE = VideoCall;
    setColourRGB(0,0,255, 0); // Blue
  } else if ( (phoneCallIndex >= 0) && (phoneCallIndex < tildeIndex) ){
    EVENT_TYPE = PhoneCall;
    setColourRGB(255,130,0, 0); // Yellow
  } else {
    EVENT_TYPE = None;
    setColourRGB(0,255,0, 0); // Green // maybe add some variation in colour for fun (slight random variations)
  }

  // Get the calendar's next free time
  int timeIndex = tildeIndex+1;
  if(timeIndex >= 0 ){
    String newTime = calendar_data.substring(timeIndex, calendar_data.indexOf(",s=", timeIndex)-3 ); // Grab only "15:23"
    if (newTime != NEXT_FREE_TIME){
      NEXT_FREE_TIME = newTime;
    }
  }else{
    NEXT_FREE_TIME = "00:00"; // Error
    Serial.println("NEXT_FREE_TIME error");
  }

  // Get the number of seconds until the calendar's next free time, since the ESP8266 doesn't actually know what time it is.
  int msTimeIndex = calendar_data.indexOf("s=", 0) + 2;
  if(timeIndex >= 0 ){
    ENTRY_FREETIME = millis()/1000 + calendar_data.substring(msTimeIndex, calendar_data.length()).toInt();
  }else{
    Serial.println("msTimeIndex error");
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  ENTRY_CALENDER = millis();
  ENTRY_UPDATE = millis();
  ENTRY_FREETIME = millis();
  
  setColourRGB(0,0,0, 0);
  displayBoot(); // Show boot message on seven segment display
  
  connectToWifi();
  rainbow(15); // Startup colours, let us know we connected successfully

  // Run everything the first time
  getCalendar();
  classifyEvent(CALENDAR_DATA);
  manageStatus();
}

void loop() {
  yield(); // Let the ESP8266 do background stuff if it needs to
  if (millis() > ENTRY_CALENDER + UPDATETIME) {
    getCalendar();
    ENTRY_CALENDER = millis();
    classifyEvent(CALENDAR_DATA);
  }
  if (millis() > ENTRY_UPDATE + 1000) { // once per second
    ENTRY_UPDATE = millis();
    BLINK = !BLINK;
    manageStatus();
  }
  if (millis() > RESET_TIME){ // Reboot regularly
    Serial.println("Reached 10 hour time limit. Rebooting...");
    ESP.reset();
  }
}
