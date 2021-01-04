// Parameter setup for web configuration page, function to retrieve configuration info

void getSetup() {
  // gets saved settings, puts them in CurrentConfig
  // get the location
  int locationNum = conf.getString("location").toInt();
  CurrentConfig.locationName = g_Locations[locationNum][0];
  CurrentConfig.locationNameID = g_Locations[locationNum][1];
  CurrentConfig.locationUTC = g_Locations[locationNum][2];
  // get wifi ssid and password
  CurrentConfig.wifiSSID = conf.getString("ssid");
  CurrentConfig.wifiPassword = conf.getString("pwd");
  // get saved unit
  String UnitIP = NVS.getString("UnitIP");
  CurrentConfig.currentSonosUnit.fromString(UnitIP);  //convert string to ipaddress
  CurrentConfig.currentSonosUnitName = NVS.getString("UnitName");
}

String configString() {
  // generates configuration string (json template).
  String params = "["
                  "{"
                  "'name':'ssid',"
                  "'label':'WiFi Network (SSID)',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':''"
                  "},"
                  "{"
                  "'name':'pwd',"
                  "'label':'WiFi Password',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':''"
                  "},"

                  "{"
                  "'name':'location',"
                  "'label':'Location',"
                  "'type':" + String(INPUTSELECT) + ","
                  "'options':["
                  "{'v':'0','l':'Calgary'},"
                  "{'v':'1','l':'Almonte'},"
                  "{'v':'2','l':'Radium'},"
                  "{'v':'3','l':'Ottawa'},"
                  "{'v':'4','l':'Vancouver'},"
                  "{'v':'5','l':'New Toronto'},"
                  "{'v':'6','l':'Osoyoos'},"
                  "{'v':'7','l':'Montreal'},"
                  "{'v':'8','l':'Caledon'},"
                  "{'v':'9','l':'Downtown Toronto'}],"
                  "'default':'AM'"
                  "},"

                  "{"
                  "'name':'sonos1_name',"
                  "'label':'Player 1 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos1_ip',"
                  "'label':'Player 1 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos2_name',"
                  "'label':'Player 2 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos2_ip',"
                  "'label':'Player 2 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos3_name',"
                  "'label':'Player 3 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos3_ip',"
                  "'label':'Player 3 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos4_name',"
                  "'label':'Player 4 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos4_ip',"
                  "'label':'Player 4 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos5_name',"
                  "'label':'Player 5 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos5_ip',"
                  "'label':'Player 5 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos6_name',"
                  "'label':'Player 6 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos6_ip',"
                  "'label':'Player 6 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos7_name',"
                  "'label':'Player 7 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos7_ip',"
                  "'label':'Player 7 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos8_name',"
                  "'label':'Player 8 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos8_ip',"
                  "'label':'Player 8 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos9_name',"
                  "'label':'Player 9 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos9_ip',"
                  "'label':'Player 9 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos10_name',"
                  "'label':'Player 10 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos10_ip',"
                  "'label':'Player 10 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos11_name',"
                  "'label':'Player 11 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos11_ip',"
                  "'label':'Player 11 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos12_name',"
                  "'label':'Player 12 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos12_ip',"
                  "'label':'Player 12 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos13_name',"
                  "'label':'Player 13 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos13_ip',"
                  "'label':'Player 13 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos14_name',"
                  "'label':'Player 14 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos14_ip',"
                  "'label':'Player 14 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "},"

                  "{"
                  "'name':'sonos15_name',"
                  "'label':'Player 15 Name',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'none'"
                  "},"
                  "{"
                  "'name':'sonos15_ip',"
                  "'label':'Player 15 IP',"
                  "'type':" + String(INPUTTEXT) + ","
                  "'default':'192.168.1.x'"
                  "}"
                  "]";

  return params;
}

void handleRoot() {
  conf.handleFormRequest(&server);
  if (server.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Configuration ************");
    for (uint8_t i = 0; i < cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
  }
  getSetup();
}
