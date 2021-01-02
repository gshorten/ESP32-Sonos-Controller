// Functions to get time, date, weather

String CurrentDT(int retType) {
  // gets the current date and time, returns the a String based on the specified retType
  String daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  String months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  String dayOfWeek = daysOfTheWeek[timeClient.getDay()];
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon;
  String currentMonthName = months[currentMonth];
  int currentYear = ptm->tm_year + 1900;
  int hour = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  String strMinutes = addLeadingZero(minutes);
  // int seconds = timeClient.getSeconds();
  String longDateAndTime = String(dayOfWeek + ", " + currentMonthName + "  " + monthDay + "  " + hour + ":" + minutes);
  String longDate = String(dayOfWeek + " " + currentMonthName + " " + monthDay);
  String currTime = String(String(hour) + ":" + strMinutes);
  String shortDate = String(currentMonthName + " " + monthDay);
  switch (retType) {
    case LONG_DATETIME:
      return longDateAndTime;
      break;
    case LONG_DATE:
      return longDate;
      break;
    case CURR_TIME:
      return currTime;
      break;
    case SHORT_DATE:
      return shortDate;
      break;
    default:
      return String("no date");
      break;
  }
}

String addLeadingZero(int num) {
  // if int is less than 10 then a leading zero is added to the num, an converts int to a string
  String retString;
  if (num <= 10) {
    retString = "0" + String(num);
  }
  else {
    retString = String(num);
  }
  return retString;
}

weatherInfo getWeather() {
  /*
    gets the local current and forecast weather and prints to serial monitor, returns a short sring with weather info
    sets global g_Weather string for use in other displays
  */
  String splash[3] = {"Getting Local Weather", "", ""};
  static weatherInfo weather;
  const String apiKey = "12ef6e437e63c471aa36d88ad6599e0f";       //openweathermap.org api key
  // get saved weather city
  String cityID = CurrentConfig.locationNameID;
  const String weatherServer = "api.openweathermap.org";
  String weatherResponse;
  unsigned int updateFreq = 300000;     //update every 10 minutes
  static long lastUpdate = millis();
 
  if (millis() - lastUpdate >= updateFreq || g_firstTime == true) {
    // update weather
    lastUpdate = millis();
    g_firstTime = false;        //first time flag ensures we get the weather first time we boot.
    if (webClient.connect("api.openweathermap.org", 80)) {
      Serial.println("Connecting to OpenWeatherMap.org");
      webClient.print("GET  /data/2.5/weather?id=" + cityID + "&appid=" + apiKey + "&units=metric");
      Serial.print("Weather location is: "); Serial.println(CurrentConfig.locationName);
      webClient.println();
    }
    else {
      Serial.println("unable to connect to openweathermap.org");
    }
    while (webClient.connected()) {
      Serial.println("getting weather response");
      delay(1000);
      weatherResponse = webClient.readStringUntil('\n');
      Serial.println(weatherResponse);
    }
    // client.println("Connection: close");
    StaticJsonDocument<6000> jsonWeather;
    DeserializationError err = deserializeJson(jsonWeather, weatherResponse);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
    }
    else {
      //deserialize the weather json file, extract info for the weather display
      Serial.println("Deserializing the weather Json");
      String currDescription = jsonWeather["weather"][0]["main"];
      int currTemp = round(jsonWeather["main"]["temp"]);
      int currWind = round(jsonWeather["wind"]["speed"]);
      //CurrWeatherDisp = String(currTemp) + "c, " + currDescription + " " + String(currWind) + " km/h" ;
      //Serial.println(CurrWeatherDisp);
      weather.currTemp = currTemp;
      weather.currWind = currWind;
      weather.currShortDesc = currDescription;
      weather.currLongDesc = "";
      weather.fcstTemp = 0;
      weather.fcstWind = 0;
      weather.fcstShortDesc = "";
      weather.fcstLongDesc = "";
      Serial.println("Weather Struct data:");
      Serial.print("Current Temp: "); Serial.println(weather.currTemp);
      Serial.print("Short Desc: "); Serial.println(weather.currShortDesc);
    }
  }
  return weather;
}

void scrollLocations(int encDirection) {
  // scrolls through weather cities when encoder state is SCROLL_LOCATIONS
  static int location = -1;
  if (encDirection == CW) {
    location++;
    if (location > NUM_LOCATIONS - 1) {
      location = 0;         // wrap around
    }
  }
  else if (encDirection == CCW) {
    location--;
    if (location < 0) {
      location = NUM_LOCATIONS - 1;
    }
  }
  g_currSelectedLocation = location;
  // display location
  String locationDisplay[3];
  int lineIndex;
  // FIRST LINE - shows item before the selected item
  if (location == 0) {
    lineIndex = NUM_LOCATIONS - 1;       // if menuItem is the last item in the list the first display line is the previous item
  }
  else {
    lineIndex = location - 1;      // if menu item is not at 0 then show previous item
  }
  locationDisplay[0] = g_Locations[lineIndex][0];   // display location name

  // SECOND LINE : shows the selected menu item
  lineIndex = location;
  String middleLine = g_Locations[lineIndex][0];   // middle line shows the selected item
  middleLine.toUpperCase();
  locationDisplay[1] = ">> " + middleLine + " <<";

  // THIRD LINE: shows the next item in the list
  if (location == NUM_LOCATIONS - 1) {   //if menuItem is at the top of the list,
    lineIndex = 0;                  // then wrap to the bottom for the third line
  }
  else {
    lineIndex++;        // else third line shows next item in the list
  }
  locationDisplay[2] = g_Locations[lineIndex][0];

  displayText(locationDisplay);
  delay(100);
}
