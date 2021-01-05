// Sonos related functions

void updateSonosStatus(long updateInterval) {
  // gets play state of the Sonos unit, updates global struct g_SonosInfo
  static long timeCheck = millis();
  long now = millis();
  if (!g_ControlsActive) {
    if ( now - timeCheck > updateInterval) {
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
      // get source and URI
      char uri[50] = "";
      TrackInfo track = g_sonos.getTrackInfo(g_ActiveUnit, uri, sizeof(uri));
      g_SonosInfo.uri = String(track.uri);
      g_SonosInfo.source  = g_sonos.getSourceFromURI(track.uri);
      // get full track info
      FullTrackInfo info;
      info = g_sonos.getFullTrackInfo(g_ActiveUnit);
//      g_SonosInfo.album = info.album;
//      g_SonosInfo.title = info.title;
//      g_SonosInfo.creator = info.creator;

      if (g_SonosInfo.playState == "Playing") {
        Serial.print("Source: "); Serial.println(g_SonosInfo.source);
//        if (   g_SonosInfo.source == SPOTIFY_SCHEME
//               || g_SonosInfo.source == SPOTIFYSTATION_SCHEME
//               || g_SonosInfo.source == HTTP_SCHEME
//               || g_SonosInfo.source == FILE_SCHEME
//               || g_SonosInfo.source == QUEUE_SCHEME ) {
//          // only these sources seem to have full track information available
         
          g_SonosInfo.title = String(info.title);
          g_SonosInfo.creator = String(info.creator);
          g_SonosInfo.album = String(info.album);
          g_TrackInfoAvailable = true;
//        }
        //        else {
        //          g_SonosInfo.title = "";
        //          g_SonosInfo.creator = "";
        //          g_SonosInfo.album = "";
        //          g_TrackInfoAvailable = false;
        //        }
        g_TrackInfoAvailable = true;
        StatusDisplayOn == true;
        statusDisplay();
      }
      timeCheck = millis();
    }
  }
}

void makeSonosIPList () {
  // construct sonos list from the conf json file
  int count = 0;
  for (int x = 3; x < (NUM_SONOS_UNITS * 2) + 3; x = x + 2) {
    // get name
    String unitName = conf.values[x];
    // get ip
    String unitIPStr = conf.values[x + 1];
    IPAddress unitIP;
    unitIP.fromString(unitIPStr);

    if (unitName != "none" && unitName != "" && unitIPStr  != "192.168.1.x") {
      // add to list of sonos units
      g_SonosUnits[count].UnitName = unitName;
      g_SonosUnits[count].UnitIPAddress = unitIP;
      count ++;
    }
  }
  g_numOfActiveUnits = count;
  g_SonosMenuLength = count + 2;
  makeSonosMenuList();
}

void makeSonosMenuList() {
  // make a list just for the sonos selection function, it includes options that are not
  // sonos units such as cancel and setup
  for (int i = 0; i < g_SonosMenuLength ; i++) {
    g_SonosMenuList[i] = g_SonosUnits[i].UnitName;
  }
  g_SonosMenuList[g_SonosMenuLength - 2] = "Cancel";
  g_SonosMenuList[g_SonosMenuLength - 1] = "Setup";
  // check list
  for (int i = 0; i < g_SonosMenuLength ; i++) {
    Serial.print(" Select Sonos Menu item: "); Serial.println(g_SonosMenuList[i]);
  }
}

void printOutSonosList() {
  // prints list of sonos units to console
  for (int i = 0; i < NUM_SONOS_UNITS; i++) {
    Serial.print("Sonos Unit "); Serial.print(i + 1); Serial.print(", :"); Serial.println(g_SonosUnits[i].UnitName);
    Serial.print("IP Address: "); Serial.println(g_SonosUnits[i].UnitIPAddress);
  }
}

void saveLastUnit() {
  // saves active  unit to NVS
  String IPString = CurrentConfig.currentSonosUnit.toString();
  NVS.setString("UnitIP", IPString);
  NVS.setString("UnitName", CurrentConfig.currentSonosUnitName);
}

void getLastUnit() {
  // gets last unit made active
  String UnitIP = NVS.getString("UnitIP");
  CurrentConfig.currentSonosUnit.fromString(UnitIP);  //convert string to ipaddress
  CurrentConfig.currentSonosUnitName = NVS.getString("UnitName");
}
