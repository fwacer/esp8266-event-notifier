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
  
  
  
  var _calendarName = 'Bryce\'s Schedule';
  var Cal = CalendarApp.getCalendarsByName(_calendarName)[0];
  var Now = new Date();
  var OneMinuteFromNow = new Date(Now.getTime() + 60000); // One minute from now
  var OneHourFromNow  = new Date(Now.getTime() + 3600000); // One hour from now
  // Logger.log(Now);
  // Logger.log(OneHourFromNow);
  var events = Cal.getEvents(Now, OneMinuteFromNow); // Check if any events are occuring right now.
  //Logger.log(events.length);
  var eventsLater = Cal.getEvents(Now, OneHourFromNow); // Check if any event will be occuring before we would normally sync again
  
  var return_val = "";
  for (var i = 0; i < events.length; i++){
    return_val += events[i].getColor() + ',';
  }
  
  var earliestFreeTime = Now; // Earliest time the schedule could be free is right now
  var phoneHomeTime = OneHourFromNow; // Could also be named nextEventStartTime. This is when the microcontroller will next phone home at latest.
  for (var i = 0; i < eventsLater.length; i++){
    if ( (eventsLater[i].getStartTime() < earliestFreeTime) && (eventsLater[i].getEndTime() > earliestFreeTime)){ // This assumes eventsLater[] is sorted by earliest to latest start date
      if (!eventsLater[i].isAllDayEvent()){
        earliestFreeTime = eventsLater[i].getEndTime();
      }
    }
    if ( (eventsLater[i].getStartTime() < phoneHomeTime) && (eventsLater[i].getStartTime() > Now)){
      phoneHomeTime = eventsLater[i].getStartTime();
    }
  }
  return_val += "~" + earliestFreeTime.toLocaleTimeString(); // Formats as "~13:24:00" on the esp8266
  return_val += ",sToFreeTime=" + (Math.floor((earliestFreeTime.getTime() - Now.getTime())/1000)).toString(); //Formats as ",sToFreeTime=XXXXXX" // Seconds to wait until clock display should turn off
  return_val += ",sToNextEvent=" + (Math.floor((phoneHomeTime.getTime() - Now.getTime())/1000)).toString(); //Formats as ",sToNextEvent=XXXXXX" // Seconds until the next event (so the microcontroller should phone home then)
  return_val += ","; // Keep a comma here in case we want to add more arguments, then we won't need to change the previous code
  // Logger.log(return_val); // also logged in doGet() function
  return return_val;
}