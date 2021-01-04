/*
   Menu and button related functions.
   A single button click has to initiate different actions depending on where we are, this is determined by the
   global variable g_SingleClickAction.  g_SingleClickAction changes as we scroll through various menus, so that a single
   click does the appropriate thing.
*/

void scrollUnits(int encDirection) {
  /*
     Uses volume control to scroll through the list of sonos units
     the currently displayed unit is made active when the button is pushed
  */
  static int menuListNum = -1;             //currently selected unit
  static IPAddress selectedUnitIP;                     //store the ip address of the unit we've current selected to make active
  String scrollUnitsDisp[3] = {"Selecting Sonos Unit", g_ActiveUnitName, "Push to Select"};
  String selectedUnitName;

  if (encDirection == CW) {
    menuListNum++;
    if (menuListNum > g_SonosMenuLength - 1) {
      menuListNum = 0;         // wrap around
    }
  }
  else if (encDirection == CCW) {
    menuListNum--;
    if (menuListNum < 0) {
      menuListNum = g_SonosMenuLength - 1;
    }
  }
  selectedUnitIP = g_SonosUnits[menuListNum].UnitIPAddress;
  selectedUnitName = g_SonosMenuList[menuListNum];
  // display the current unit name , ip on the display
  scrollUnitsDisp[1] = selectedUnitName;
  if (menuListNum < g_SonosMenuLength - 2) {
    scrollUnitsDisp[2] = selectedUnitIP.toString();
  }
  else if (menuListNum == g_SonosMenuLength - 2) {
    // this is cancel
    scrollUnitsDisp[2] = "Cancel Selecting a new Player";
  }
  else if (menuListNum == g_SonosMenuLength - 1) {
    // this is setup
    scrollUnitsDisp[0] = "Click to Setup WiFi,";
    scrollUnitsDisp[1] = "Location, or Sonos Units";
    scrollUnitsDisp[2] = "Use your phone & a web browser";
  }
  displayText(scrollUnitsDisp);      // update the display with the currently selected  unit
  g_CurrentUnitNum = menuListNum;          //used in unitSelectClick to select right  unit from list of units
}

void unitSelectClick() {
  // single button click make the currently selected unit the active unit.
  if (g_CurrentUnitNum == g_SonosMenuLength - 2) {
    // cancel
    exitChangeSonos();
  }
  else if (g_CurrentUnitNum == g_SonosMenuLength - 1) {
    // go to setup
    // display instructions
    displaySetupInstructions();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(conf.getApName(), "", 1);
    server.on("/", handleRoot);
    server.begin(80);
    StatusDisplayOn = false;
    g_State = SETUP;
  }
  else {
    g_ActiveUnit = g_SonosUnits[g_CurrentUnitNum].UnitIPAddress;    // make the currently selected unit the active unit
    g_ActiveUnitName = g_SonosUnits[g_CurrentUnitNum].UnitName;
    // Save to default configuration NVS
    // convert IPAddress to String for storage in NVS
    String IPString = g_ActiveUnit.toString();
    NVS.setString("UnitIP", IPString);
    NVS.setString("UnitName", g_ActiveUnitName);
    String scrollUnitsDisp[3];
    // display the selected unit on the screen
    scrollUnitsDisp[0] = g_ActiveUnitName + " is Active";
    scrollUnitsDisp[1] = IPString;
    scrollUnitsDisp[2] = "";
    displayText(scrollUnitsDisp);
    delay(500); //leave dispay on for at least a bit
    Serial.print(g_ActiveUnitName); Serial.println(" has been selected");
    exitChangeSonos();
  }
}

void changeSonosUnit() {
  // encoder input mode, this enters the scrollUnits function when the control is turned.
  g_EncInputMode = STATE_CHANGE_UNIT;
  g_SingleClickAction = STATE_CHANGE_UNIT;    // single button press mode
  StatusDisplayOn = false;              //turn status display off while we scroll through sonos units
  // splash display
  String splash[3] = {"Changing Sonos Unit", "Vol Ctrl Selects", "Pushbutton Activates"};
  displayText(splash);
}

void exitChangeSonos() {
  g_EncInputMode = STATE_PLAYING;
  g_SingleClickAction = STATE_PLAYING;
  StatusDisplayOn = true;
 
  g_ControlsActive = false;
  statusDisplay();
}

void displaySetupInstructions() {
  String instructions[3];
  instructions[0] = "On phone set WiFi";
  instructions[1] = "To Sonos_Controller";
  instructions[2] = "Open browser @ 192.168.4.1, then restart";
  displayText(instructions);
}
