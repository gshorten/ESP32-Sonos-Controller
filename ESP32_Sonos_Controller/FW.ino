/*
   Firmware update functions
   Revisions
   1.2  First working OTA
   1.3  added messages on display when OTA updating
   2.0  major update, moved all files to git repository
   2.1  increased time between wifi connect attempts to 1 second, 10 max attempts
   2.2  minor tweaks
   2.3  tweaking startup displays, sequence
   2.4  fixed weather update splash
   2.5  save and retrieve firmware version for startup display
*/

String FirmwareVer = {
  "2.5"                 //added Firmware version to start up splash
};

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      {
        Serial.println("HTTP_UPDATE_NO_UPDATES");
      }
      break;

    case HTTP_UPDATE_OK:
      {
        Serial.println("HTTP_UPDATE_OK");
        String splash[3] = {"Firmware Updated", "", ""};
        displayText(splash);
      }
      break;
  }
}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client)
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    HTTPClient https;

    if (https.begin( * client, fwurl))
    { // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }

  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    }
    else
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      String splash[3];
      splash[0]= "New Firmware found";
      splash[1] = "Please Wait";
      splash[2] = "Updating to latest Firmware, version: ";
      splash[2] +=  payload;
      displayText(splash);
      return 1;
    }
  }
  g_FirmwareVersion = payload;
  NVS.setString("Firmware", g_FirmwareVersion);
  return 0;
}
