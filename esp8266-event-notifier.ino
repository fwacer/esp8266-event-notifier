/*
  2020-04-24
  Bryce Dombrowski
  
  Modified from https://github.com/SensorsIot/Reminder-with-Google-Calender
*/

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "HTTPSRedirect.h"
#include "credentials.h"

//Connection Settings
const char* host = "script.google.com";
const int httpsPort = 443;

unsigned long entryCalender;

const int rPin = D1;
const int gPin = D2;
const int bPin = D3;
String ledColour = "000000";

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
}eventType;

HTTPSRedirect* client = nullptr;

String calendarData = "";
bool calenderUpToDate;

void setColour (String colour){
  long temp = strtol(&colour[0], NULL, 16); // convert string (of a hex number) to a long
  int r = (temp >> 16) & 0xff;
  int g = (temp >> 8) & 0xff;
  int b = (temp >> 0) & 0xff;
  analogWrite(rPin, r*4);
  analogWrite(gPin, g*4);
  analogWrite(bPin, b*4);
  Serial.println("#"+colour);
}

void setColourRGB (int r, int g, int b, int delayTime){ // range 0-255
  analogWrite(rPin, r*4);
  analogWrite(gPin, g*4);
  analogWrite(bPin, b*4);
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
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    ESP.reset();
  }
  Serial.println("Connected to Google");
}

void getCalendar() {
  Serial.println("Start Request");
  // HTTPSRedirect client(httpsPort);
  unsigned long getCalenderEntry = millis();

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    ESP.reset();
  }
  //Fetch Google Calendar events
  String url = String("/macros/s/") + GScriptIdRead + "/exec";
  client->GET(url, host);
  calendarData = client->getResponseBody();
  Serial.print("Calendar Data---> ");
  Serial.println(calendarData);
  calenderUpToDate = true;
  yield();
}

void manageStatus() {
  delay(500);
  switch (eventType) {
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
  // Finds the highest priority event, then sets "eventType" to that value.
  if (calendarData.indexOf("\'"+String(InPerson)+"\'", 0) >= 0 ){
    eventType = InPerson;
  }else if (calendarData.indexOf("\'"+String(VideoCall)+"\'", 0) >= 0 ){
    eventType = VideoCall;
  }else if (calendarData.indexOf("\'"+String(PhoneCall)+"\'", 0) >= 0 ){
    eventType = PhoneCall;
  //}else if (calendarData.indexOf("\'\'\'", 0) >= 0 ){
    // Colour is calendar default
  }else {
    eventType = None;
  }
}


void setup() {
  Serial.begin(9600);
  connectToWifi();
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  rainbow(15); // startup colours
  setColourRGB(0,0,0, 0);
  getCalendar();
  entryCalender = millis();
}


void loop() {
  if (millis() > entryCalender + UPDATETIME) {
    getCalendar();
    entryCalender = millis();
  }
  classifyEvent(calendarData);
  manageStatus();
}
