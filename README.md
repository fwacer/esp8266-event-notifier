# esp8266-event-notifier
This is my Google Calendar Busy Indicator. It connects over WiFi to a calendar, then turns the top LED to the colour of the event (Green is "empty" or "free"). This allows the end user to tell if the owner of the calendar is busy and what type of meeting they are in. 
On the front face, there is a seven-segment display that shows the time that the owner of the calendar is free next. 

I plan to use this at home, to track when my parent is busy and when they will next be free.

A blog write up will eventually be available here: https://brycedombrowski.com
The underlying code is based on SensorSlot's implementation of a similar idea: https://github.com/SensorsIot/Reminder-with-Google-Calender

To do:
- Securely mount the internal components (likely with fasteners and/or epoxy).
- Finish cleaning up the cut rough edges with a metal file.
