/*
  control functions using rotary encoder and pushbutton
*/

void buttonEvent(AceButton* /*encButton*/, uint8_t eventType, uint8_t buttonState) {
  /*The event handler for the encoder button.
     This processes the click, doubleclick, longpress.  These (can) change depending on mode set.
     Currently a double click always skips to next track, and a long press always brings up the main menu.
     A single click action changes depending on where we are in the main menu, or other menus.  The global const
     g_SingleClickAction is generally set as we scroll through menus.
  */
  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  g_ControlsActive = true;
  switch (eventType) {
    // single click
    case AceButton::kEventClicked: {
        //case AceButton::kEventReleased: {
        /*
           determines what happens with a single click.  The global g_SingleClickAction is set when a menu item
           item is shown on the main menu, or on a sub menu (such as select unit), ie as the main menu is scrolled the
           single press action is set.  Then, when the button is clicked the matching action below will
           be executed.
        */
        switch (g_SingleClickAction) {
          case STATE_PLAYING:
            pausePlay();              // call the pause play function
            break;
          case STATE_CHANGE_UNIT:
            // When we are in the select unit menu, a single click will make the currently shown unit active.
            {
              unitSelectClick();
            }
            break;
          default:
            pausePlay();          // calling the pause play function is the default.
            break;
        }
      }
      break;
    case AceButton::kEventDoubleClicked: {
        // double click always skips to the next track
        g_sonos.skip(g_ActiveUnit, SONOS_DIRECTION_FORWARD);
        String splash[3] = {"Skipping", "Forward To The", "Next Track"};
        displayText(splash);
      }
      break;
    case AceButton::kEventLongPressed: {
        /*
           long press always just brings up the main menu
           change input modes for volume control and encoder button to operate the main menu
        */

        changeSonosUnit();
      }
      break;
  }
}

void checkEncoder() {
  // Checks to see if the encoder has turned, if so it calls rotaryHandler, otherwise does nothing
  // This is called from the main loop
  // initialize variables, make sure we don't call encHandler first loop!:
  const int encHyst = 1 ;   //encoder hysterises, for extra debouncing
  long newPosition = sonosEnc.getCount();
  static long oldPosition = newPosition;     // will be initiallized once, so first time this is called it won't change volume.
  int encDirection;

  // check to see if position has changed:
  if (abs(newPosition - oldPosition) > encHyst ) {
    g_ControlsActive = true;
    if (newPosition > oldPosition) {
      encDirection = CW;
    }
    else if (newPosition < oldPosition ) {
      encDirection = CCW;
    }
    oldPosition = newPosition;
    rotaryHandler(encDirection);        // it's changed, so call the handler
    g_EncoderEvent = millis();
  }
}

/////////////////////////////////////////////////////////////////////////////////

void rotaryHandler(int encDirection) {
  // calls the appropriate function when the encoder is turned depending on mode
  // note these are called each time the rotary control moves one step up or down
  switch (g_EncInputMode) {
    case STATE_PLAYING:
      setSonosVolume(encDirection);
      break;
    case STATE_CHANGE_UNIT:
      // step through sonos units
      scrollUnits(encDirection);
      break;
  }
}
/////////////////////////////////////////////////////////////////////////////////////

void setSonosVolume(int encDirection) {
  /* Changes the volume of the active sonos unit with the rotary encoder.  It periodically gets the volume
    of the active unit and caches it.  We have to cache the volume because the sonos units sometimes respond to slowely to get the
    volume while we are also changing it.  We also accumulate  a number of encoder readings then change the volume, also to reduce the netork traffic to the
    sonos units.
    The display update function is suspended with a flag while the user is changing the volume.
  */

  int activeVolume;            // volume of the active unit
  const int changeVolumePeriod = 1000; //  how often we check volume of the active unit, in milliseconds
  static long lastVolumeChange = 0;
  //static int oldVolume = g_sonos.getVolume(g_ActiveUnit); // save the old position of the encoder
  //use cached volume instead
  static int oldVolume =  g_SonosInfo.volume;
  Serial.print("Old Volume: "); Serial.println(oldVolume);
  int newVolume;
  String textToDisplay[3];
  const byte VOL_CHANGE_UP = 4;
  const byte VOL_CHANGE_DOWN = 4;      // reduces volume more quickly

  if (encDirection == CW) {
    newVolume = oldVolume + VOL_CHANGE_UP;
    if (newVolume >= 100) {             // check high, low limits, if reading is >100 or less than 0 correct
      newVolume = 100;
    }
  }
  else if (encDirection == CCW) {
    newVolume = oldVolume - VOL_CHANGE_DOWN;
    if (newVolume <= 0) {
      newVolume = 0;
    }
  }
  oldVolume = newVolume; // save old position
  Serial.println(newVolume);
  g_SonosInfo.volume = newVolume;
  g_sonos.setVolume(g_ActiveUnit, newVolume); // change the volume
  textToDisplay[0] = String(g_ActiveUnitName + " Volume");
  textToDisplay[1] = "";
  textToDisplay[2] = " ";
  displayText(textToDisplay);
  Heltec.display ->drawProgressBar(10, 20, 110, 15, newVolume);
  Heltec.display->display();
}
//////////////////////////////////////////////////////////////////////////////

void pausePlay() {
  // pauses or plays the sonos unit
  g_TimeDisplayStarted = millis();
  String textToDisplay[3];
  if (StatusDisplayOn == true) {

    if (g_SonosInfo.playState == "Playing") {
      Serial.println(F("Pausing active unit "));
      g_sonos.pause(g_ActiveUnit);
      textToDisplay[1] = F("Change to Paused");
      g_SonosInfo.playState = "Paused";
    }
    else if (g_SonosInfo.playState == "Paused" || g_SonosInfo.playState == "Stopped") {
      Serial.println(F("Playing active unit "));
      g_sonos.play(g_ActiveUnit);
      textToDisplay[1] = F("Change to Playing");
      g_SonosInfo.playState = "Playing";
    }

    //statusDisplay();
  }
  else if (StatusDisplayOn == false) {
    //if display is timed out we just turn it back on,
    StatusDisplayOn = true;
    //statusDisplay();        // immediatly show status.
  }
  textToDisplay[0] = String(g_ActiveUnitName);
  textToDisplay[2] = "";
  displayText(textToDisplay);
  //delay(2000);
  g_ControlsActive = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void encoderTimer() {
  // sets a flag that lets other functions know that the encoder is in use
  long encoderTimeOut = 2000;    // time after last encoder action that other functions will be blocked
  if (millis() - g_EncoderEvent <= encoderTimeOut) {
    g_ControlsActive = true;
  }
  else if (millis() - g_EncoderEvent >= encoderTimeOut) {
    g_ControlsActive = false;
  }
}
