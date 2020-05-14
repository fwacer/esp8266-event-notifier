# esp8266-event-notifier
This is my Google Calendar Busy Indicator. It connects over WiFi to a calendar, then turns the top LED to the colour of the event (Green is "empty" or "free"). This allows the end user to tell if the owner of the calendar is busy and what type of meeting they are in. 
On the front face, there is a seven-segment display that shows the time that the owner of the calendar is free next. 
I plan to use this at home, to track when my parent is busy and when they will next be free.

Demo video: https://youtu.be/q-8Iey-jSQw

[![Video thumbnail of the ESP8266 event notifier prototype](https://img.youtube.com/vi/q-8Iey-jSQw/0.jpg)](https://www.youtube.com/watch?v=q-8Iey-jSQw)

The underlying code is based on SensorsIOT's implementation of a similar idea: https://github.com/SensorsIot/Reminder-with-Google-Calender

Blog post: https://brycedombrowski.com/2020/05/spring-2020-google-calendar-busy-indicator/

# Instructions to make this project:
- Download _esp8266-event-notifier.ino_ to your project folder and modify it as needed. The #defines at the top contain most of the things you will want to modify.
- From the library HTTPSRedirect, download _HTTPSRedirect.h_ and _HTTPSRedirect.cpp_ and place them in your local project folder. https://github.com/electronicsguy/ESP8266/tree/master/HTTPSRedirect
- Copy/paste the contents of ReadCalendarEvent.gs to a new project file on script.google.com. Make sure to rename _\_calendarName_ to the name of your calendar. Then Publish->Deploy as web app . Make sure to "Anyone, even anonymous" when selecting "who has access to the app".  Copy the secret from the resulting URL: ```https://script.google.com/macros/s/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/exec```. The XXs are your secret. Note: every time you make a change to the code, you must increment the project version by 1 or the new code will not be live for your next request. For more information on this whole bullet point step, see Andreas Spiess's excellent video: https://youtu.be/sm1-l5-z3ag?t=199
- Create a file called _credentials.h_. It should contain the following:
  ```C++
  #define CREDENTIALS
  const char* SSID_NAME = "dlink"; // Your wifi's name
  const char* SSID_PASSWORD = "password"; // Your wifi password
  const char *GSCRIPT_ID =  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; //replace with your secret
  ```
- Make the circuit! I've included both an Eagle schematic and a PDF of my setup. Feel free to modify it or make your own from scratch.
