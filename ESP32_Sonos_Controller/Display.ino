/*
   Display related functions
*/

void displayText(String lines[3], int numLines) {
  // displays 3 rows of text, centered.
  const byte dispWidth = 128;                  // how many characters can be displayed
  const long charWidth = 6.5;                 // typical character width
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setBrightness(255);
  //display->setFont(ArialMT_Plain_16);       //alternative text, is really big
  //Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setFont(Roboto_Condensed_12);
  //Heltec.display->setFont(DejaVu_Sans_12);
  int lineSpacing[] = {0, 16, 31};            //y position for starting pixel for each of 4 rows, based on the 10pt font
  int lineStart = 0;                          //x position for starting pixel
  Heltec.display->clear();                    // clear what is on the display

  for (int i = 0; i < numLines; i++) {
    // loop through text to be displayed, center short text, left align long text
    int lineLength = Heltec.display->getStringWidth(lines[i]);
    if (lineLength < dispWidth) {         //adjust start to center, only do this for first two lines of text
      lineStart = ((dispWidth - lineLength) / 2);
    }
    else {
      lineStart = 0;
    }
    if (i <= 1 ) {
      Heltec.display -> drawString(lineStart, lineSpacing[i], lines[i]);  //load the lines to the display buffer
    }
    else {
      // wrap the third line of text, typically for something that might be long, ie artist, title, but shorten it
      Heltec.display -> drawStringMaxWidth(lineStart, lineSpacing[i], dispWidth, lines[i]);
    }
  }
  Heltec.display->display();           // display the lines
  //g_DisplayStarted = millis();            //reset the timer for the
}

////////////////////////////////////////////////////////////////////////////

void showStatus(int seconds) {
  // shows the staus display periodically
  static long lastTimeChecked = millis();     // last time we checked
  const long checkFreq = seconds * 1000;              // how often we check and  update the display
  if (millis() - lastTimeChecked > checkFreq && StatusDisplayOn == true && !g_ControlsActive) {
    // update the status display
    statusDisplay();
    lastTimeChecked = millis();
  }
}
///////////////////////////////////////////////////////////////////////////
void DisplayTimeout(int timeout) {    // timeout is in minutes
  /*Times out the status display, turns it off after a period of inactivity.  This saves the OLEDS
  */
  const long displayTimeout = timeout * 60000;         // time we let display stay on, in milliseconds
  //static long timeDisplayStarted = millis();
  long displayCheck = millis();
  if (g_SonosInfo.playState != "Playing") {
    if (displayCheck - g_TimeDisplayStarted >= displayTimeout) {
      Heltec.display->clear();        // clear buffer
      Heltec.display->display();      // display empty buffer , ie turn display off
      StatusDisplayOn = false;
    }
  }
  else {
    g_TimeDisplayStarted = millis();
  }
}

/////////////////////////////////////////////////////////////////////////////

void updateSonosStatus(long updateInterval, boolean doNow) {
  // gets play state of the Sonos unit, updates global struct g_SonosInfo
  static long timeCheck = millis();
  long now = millis();
  if (!g_ControlsActive) {
    if ( now - timeCheck > updateInterval || doNow == true) {
      Serial.println("Getting data on current sonos unit");
      // get volume
      g_SonosInfo.volume = g_sonos.getVolume(g_ActiveUnit);      // get the volume of current unit
      // get playstate
      byte playerState = g_sonos.getState(g_ActiveUnit);    // get playstate
      switch (playerState) {
        case SONOS_STATE_PLAYING:
          g_SonosInfo.playState = "Playing";
          break;
        case SONOS_STATE_PAUSED:
          g_SonosInfo.playState = "Paused";
          break;
        case SONOS_STATE_STOPPED:
          g_SonosInfo.playState = "Stopped";
          break;
        default:
          g_SonosInfo.playState = "Unknown";
          break;
      }
      if (g_SonosInfo.playState == "Playing") {
        // only get track info if the unit is playing
        // get source
        SonosInfo info;
        info = g_sonos.getSonosInfo(g_ActiveUnit);
        g_SonosInfo.source = String(info.source);
        Serial.print("Source: "); Serial.println(g_SonosInfo.source);
        if (   g_SonosInfo.source == SPOTIFY_SCHEME
               || g_SonosInfo.source == SPOTIFYSTATION_SCHEME
               || g_SonosInfo.source == HTTP_SCHEME
               || g_SonosInfo.source == FILE_SCHEME
               || g_SonosInfo.source == QUEUE_SCHEME ) {
          // only these sources seem to have full track information available
          FullTrackInfo info = g_sonos.getFullTrackInfo(g_ActiveUnit);
          g_SonosInfo.title = String(info.title);
          g_SonosInfo.creator = String(info.creator);
          g_SonosInfo.album = String(info.album);
          g_TrackInfoAvailable = true;
        }
        else {
          g_SonosInfo.title = "";
          g_SonosInfo.creator = "";
          g_SonosInfo.album = "";
          g_TrackInfoAvailable = false;
        }
        StatusDisplayOn == true;
        statusDisplay();
      }
      timeCheck = millis();
      doNow = false;
    }
  }
}

void statusDisplay() {
  /*
      Status of sonos s system, time, weather that is displayed when display is not showing other stuff,
      ie, not when the volume is changing or when we are showing a menu structure.
      There are several different displays.
      Check for g_UIStarted (millis) and only show the update if it's been 5 seconds or so from when the last UI action occured.
        This is to prevent the display updating when the volume is being changed or menu being selecteed.
  */

  if (StatusDisplayOn = true) {
    // Only update the status display if nothing else is using the display.
    // Must ensure that other functions using the display turn it back on when they are finished.
    // gets current volume, playstate, track info for updating the display

    String displayLines[3];
    // get battery percent and current time
    int battPercent = batteryPercent();
    String hourMinutes = CurrentDT(CURR_TIME);

    // make first two lines of the display
    displayLines[0] = g_ActiveUnitName + " " + g_SonosInfo.playState + " Vol: " + g_SonosInfo.volume;
    displayLines[1] = String(g_Weather.currTemp) + "c  " + g_Weather.currShortDesc + " "
                      + hourMinutes + " b:" + battPercent + "%";
    if (g_TrackInfoAvailable) {
      // we have track information, so display it
      displayLines[2] = g_SonosInfo.creator;
      displayLines[2] += " : ";
      displayLines[2] += g_SonosInfo.title;
      displayLines[2] += " : ";
      displayLines[2] += g_SonosInfo.album;
    }
    else if (!g_TrackInfoAvailable) {
      // no track information
      displayLines[2] = "No track information is available for this source";
    }
    Serial.println(displayLines[0]);
    Serial.println(displayLines[1]);
    Serial.println(displayLines[2]);
    Serial.println(" -------------------------------------------------------");
    Serial.println();
    displayText(displayLines);
    //g_TimeDisplayStarted = millis();
  }
}

int batteryPercent() {
  // reads the battery voltage, returns an int 0 - 100 percent
  // battery voltage constants
  const long FULL_BATTERY = 4100;       // fully charged, divide by 1000 for mv
  const long EMPTY_BATTERY = 3200;      // fully discharged
  long BATT_RANGE = FULL_BATTERY - EMPTY_BATTERY;
  const float BATT_ADJ_MV = 0.00225;  // to convert battery reading to mv
  const float BATT_ADJ = 2.25;        // to convert battery reading to a 3000 - 4200 range
  const int LOW_BATT = 20;
  int avg = 6;                // number of readings to average
  static int avgPercent = 50;
  static int readingNo = 1;
  static int totalPercent = 0;

  Serial.println("Checking the Battery Voltage");
  // TODO: take several readings
  adcStart(BATT_PIN);
  //while (adcBusy(BATT_PIN));
  Serial.printf("Battery power in GPIO 37: ");
  Serial.println(analogRead(BATT_PIN));
  float voltReading  =  analogRead(BATT_PIN) * BATT_ADJ ;
  Serial.print("voltReading: "); Serial.println(voltReading);
  float battCalc = (voltReading - EMPTY_BATTERY) / BATT_RANGE;
  Serial.print("battCalc: "); Serial.println(battCalc);
  int battPercent = battCalc * 100;
  Serial.print("Battery Percent: "); Serial.println(battPercent);
  adcEnd(BATT_PIN);
  if (readingNo <= avg) {
    // average readings
    totalPercent = totalPercent + battPercent;
    readingNo ++;
  }
  else if ( readingNo > avg) {
    avgPercent = totalPercent / avg;
    totalPercent = 0;
    readingNo = 1 ;
  }
  if (avgPercent < LOW_BATT ) {
    g_LowBattery = true;
  }
  else if (avgPercent >= LOW_BATT) {
    g_LowBattery = false;
  }
  return avgPercent;
}

void pDInt(String label, int value) {
  // convienience method for printing debug messages
  Serial.print(label);
  Serial.println(value);
  Serial.println();
}

void pwrLED() {
  //blinks the power LED once per second, indicates that the unit is on.
  // we only do this when the display is off.
  long blinkOn = 2000;
  long blinkOff = 500;
  static unsigned long blinkTimer = millis();
  static boolean ledOn = true;
  if (g_LowBattery == true) {
    // if battery is low turn on and off
    if (ledOn == true) {
      if ( millis() - blinkTimer > blinkOn) {
        ledOn = false;  // turn off
        ledcWrite(0, 0);
        blinkTimer = millis();
      }
    }
    else if (!ledOn) {
      if ( millis() - blinkTimer > blinkOff) {
        // turn on
        ledcWrite(0, 50);
        ledOn = true;
        blinkTimer = millis();
      }
    }
  }
  else if (!g_LowBattery) {
    ledcWrite(0, 50);
  }
}
