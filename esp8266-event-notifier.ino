/*
  2020-04-24
  Bryce Dombrowski
  
  Modified from https://github.com/SensorsIot/Reminder-with-Google-Calender
*/

#include <ESP8266WiFi.h>
// #include <Ticker.h>
#include "HTTPSRedirect.h"
#include "credentials.h"

// GLOBALS //

//Connection Settings
const char* HOST = "script.google.com";
const int HTTPS_PORT = 443;
HTTPSRedirect* CLIENT = nullptr;
unsigned long ENTRY_CALENDER;
String CALENDAR_DATA = "";
bool CALENDAR_UP_TO_DATE;

String LED_COLOUR = "000000";
const int PIN_RED = D1;
const int PIN_GREEN = D2;
const int PIN_BLUE = D3;
const int PIN_DATA = D5; // Shift register SER
const int PIN_CLOCK = D6; // Shift register SRCLK
const int PIN_LATCH = D7; // Shift register RCLK

#define UPDATETIME 36000000 // Once every 10 hours

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
  // DP G F E D C B A
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
};

// FUNCTIONS //

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
}

void displayTime(String time){ // Format HH:MM, 12hr clock
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[time.substr(4,5).toInt()]); // LSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[time.substr(3,4).toInt()]); // MSB digit of minute
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[time.substr(1,2).toInt()]); // LSB digit of hour
  digitalWrite(PIN_CLOCK, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, SEVEN_SEGMENT[time.substr(0,1).toInt()]); // MSB digit of hour
  digitalWrite(PIN_LATCH, HIGH); // Move value to display register
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
  delay(500);
  switch (EVENT_TYPE) {
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
  }
  yield();
}

void classifyEvent(String calendarString) {
  // Finds the highest priority event, then sets "EVENT_TYPE" to that value.
  if (CALENDAR_DATA.indexOf("\'"+String(InPerson)+"\'", 0) >= 0 ){
    EVENT_TYPE = InPerson;
  }else if (CALENDAR_DATA.indexOf("\'"+String(VideoCall)+"\'", 0) >= 0 ){
    EVENT_TYPE = VideoCall;
  }else if (CALENDAR_DATA.indexOf("\'"+String(PhoneCall)+"\'", 0) >= 0 ){
    EVENT_TYPE = PhoneCall;
  //}else if (CALENDAR_DATA.indexOf("\'\'\'", 0) >= 0 ){
    // Colour is calendar default
  }else {
    EVENT_TYPE = None;
  }
}


void setup() {
  Serial.begin(9600);
  connectToWifi();
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  
  rainbow(15); // startup colours
  setColourRGB(0,0,0, 0);
  getCalendar();
  ENTRY_CALENDER = millis();
}


void loop() {
  if (millis() > ENTRY_CALENDER + UPDATETIME) {
    getCalendar();
    ENTRY_CALENDER = millis();
  }
  classifyEvent(CALENDAR_DATA);
  manageStatus();
}
