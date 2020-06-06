/*
  2020-06-05
  Bryce Dombrowski

  Allows control of an RGB led through the serial monitor. Useful for determining accurate colour values for use in programs.
*/

// INCLUDES // ************************************************************************************************************************


// DEFINES // *************************************************************************************************************************

// Define pins
#define PIN_RED   D1 // Red anode of the RGB LED. Must be a PWM pin.
#define PIN_GREEN D2 // Green anode of the RGB LED. Must be a PWM pin.
#define PIN_BLUE  D3 // Blue anode of the RGB LED. Must be a PWM pin.

// FUNCTIONS // ***********************************************************************************************************************

void setColourRGB (int r, int g, int b, int delayTime=0){ // range 0-255
  // Note: analogWrite() initiates a PWM signal to control the brightness of each colour, by modifying the average voltage. The pins used here must be PWM.
  analogWrite(PIN_RED, r*4);
  analogWrite(PIN_GREEN, g*4);
  analogWrite(PIN_BLUE, b*4);
  delay(delayTime);
}

void setup() { // Main code that runs once, initializes variables.
  Serial.begin(9600);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  setColourRGB(0,0,0);
}

void loop() { // Code that runs continuously on a loop
  yield(); // Let the ESP8266 do background stuff if it needs to
  int r = 0;
  int g = 0;
  int b = 0;
  int i = 0;
  Serial.print("r=");  
  while (Serial.available() == 0) delay(1); // Wait for input (but make sure to delay or yield periodically for the NodeMCU board)
  r = Serial.readString().toInt();
  Serial.print(String(r)+" g=");  
  while (Serial.available() == 0) delay(1); // Wait for input (but make sure to delay or yield periodically for the NodeMCU board)
  g = Serial.readString().toInt();
  Serial.print(String(g)+" b=");  
  while (Serial.available() == 0) delay(1); // Wait for input (but make sure to delay or yield periodically for the NodeMCU board)
  b = Serial.readString().toInt();
  Serial.println(String(b));
  setColourRGB(r,g,b);
}
