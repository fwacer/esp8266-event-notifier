// Written by Bryce Dombrowski 2020

/* NOTE - this needs to be copy/pasted into a Google Apps Script ( https://script.google.com/ ).
   This file is not local normally, and is only here for reference.
*/ 

function doGet(){
  //Logger.log(ContentService.createTextOutput(GetEvents()));
  var val = ContentService.createTextOutput(GetEvents());
  Logger.log(val.getContent());
  return val;
}

function GetEvents(){
  /* Possible Event colours
  PALE_BLUE     Enum	 Pale Blue ("1").
  PALE_GREEN	Enum	 Pale Green ("2").
  MAUVE	        Enum	 Mauve ("3").
  PALE_RED	    Enum	 Pale Red ("4").
  YELLOW	    Enum	 Yellow ("5").
  ORANGE	    Enum	 Orange ("6").
  CYAN	        Enum	 Cyan ("7").
  GRAY	        Enum	 Gray ("8").
  BLUE	        Enum	 Blue ("9").
  GREEN	        Enum	 Green ("10").
  RED	        Enum	 Red ("11").
  */
  
  var _calendarName = 'Bryce\'s Schedule'; // Enter the name of the calendar as displayed to you here.
  var cal = CalendarApp.getCalendarsByName(_calendarName)[0];
  var now = new Date();
  var oneMinuteFromNow = new Date(now.getTime() + 60000); // One minute from now
  var fourteenHoursFromNow  = new Date(now.getTime() + 14*3600*1000); // Fourteen hours from now -> This is the longest that we will look when searching for the calendar's next free slot.
  
  var eventsNow = cal.getEvents(now, oneMinuteFromNow); // Check if any events are occuring right now.
  var eventsLater = cal.getEvents(now, fourteenHoursFromNow); // Check for the rest of the day's events
  
  var return_val = "";
  for (var i = 0; i < eventsNow.length; i++){
    return_val += eventsNow[i].getColor() + ','; // List colours numbers of all current ongoing events. We'll determine what this means on the microcontroller itself.
    if (eventsNow[i].getColor()=="") { // No colour is assigned, and we need to assign a default one or it will be missed
      return_val += '11,'; // Red - Our default calendar event colour
    }
  }
  
  var earliestFreeTime = now; // Earliest time the schedule could be free is right now
  var phoneHomeTime = new Date(now.getTime() + 3600*1000); // One hour is when the microcontroller will next phone home at latest.
  for (var i = 0; i < eventsLater.length; i++){ // Iterate over the day's events
    var startTime = eventsLater[i].getStartTime();
    var endTime = eventsLater[i].getEndTime();
    if (!eventsLater[i].isAllDayEvent()){ // Ignore all-day events
      if ( (startTime < new Date(earliestFreeTime.getTime() + 60000 /*Plus one minute*/)) && (endTime > earliestFreeTime)){ // Getting the next free time slot for display on the seven segment display
        earliestFreeTime = endTime;
      }
      if ( (startTime < phoneHomeTime) && (startTime > now)){ // Case where there is another event starting before we would normally phone home
        phoneHomeTime = startTime;
      }
      if ( (endTime < phoneHomeTime) && (startTime < now)){ // Case where a currently-running event is ending before we would normally phone home
        phoneHomeTime = endTime;
      }
    }
  }
  
  return_val += "~" + earliestFreeTime.toLocaleTimeString(); // Formats as "~13:24:00" on the esp8266. NOTE: this is different than how it formats if you run this code from your computer's internet browser!
  return_val += ",sToPhoneHome=" + (Math.floor((phoneHomeTime.getTime() - now.getTime())/1000)).toString(); // Formats as ",sToNextEvent=XXXXXX" // Seconds until the microcontroller should phone home again
  return_val += ",currentTime=" + now.toLocaleTimeString(); // Formats as ",currentTime=13:24:00" on the esp8266. NOTE: this is different than how it formats if you run this code from your computer's internet browser!
  return_val += ","; // Keep a comma here in case we want to add more arguments, then we won't need to change the previous code
  return return_val;
}