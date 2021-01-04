// Sonos related functions

void makeSonosIPList () {
  // construct sonos list from the conf json file
  // SonosUnit g_SonosUnits[NUM_SONOS_UNITS]
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

void makeSonosMenuList(){
  // make a list just for the sonos selection function, it includes options that are not 
  // sonos units such as cancel and setup
  for (int i = 0; i < g_SonosMenuLength ; i++){
    g_SonosMenuList[i] = g_SonosUnits[i].UnitName;
  }
  g_SonosMenuList[g_SonosMenuLength -2] = "Cancel";
  g_SonosMenuList[g_SonosMenuLength -1] = "Setup";
  // check list
  for (int i =0; i < g_SonosMenuLength ; i++) {
    Serial.print(" Select Sonos Menu item: ");Serial.println(g_SonosMenuList[i]);
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
