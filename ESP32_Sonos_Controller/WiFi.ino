// init wifi

boolean initWiFi() {
  //connects to the wifi network
  boolean wifiConnected = false;
  // attempt to connect using saved values for ssid and pwd
  // WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
  int wifiAttempts = 0;
  int maxAttempts = 7;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < maxAttempts) {   //loop until wifi is connected OR max attempts
    wifiAttempts ++;
    delay(750);
    // attempt to connect using saved values for ssid and pwd
    WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
    Serial.print(".");
  }
  // if we are connected return true
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    g_State = OPERATING;
  }
  else if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("************ Going into Setup Mode ****************");
    g_State = SETUP;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(conf.getApName(), "", 1);
  }
  return wifiConnected;
}

void wifiKeepAlive() {
  //checks to see if wifi is connected, if not it reconnects
  const long checkFreq = 5000;
  static long lastCheck = millis();
  if (millis() - lastCheck > checkFreq) {
    if (WiFi.status() != WL_CONNECTED) {
      // Reconnect
      int wifiAttempts = 0;
      int maxAttempts = 5;
      while (WiFi.status() != WL_CONNECTED && wifiAttempts < maxAttempts) {   //loop until wifi is connected OR max attempts
        WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
        wifiAttempts ++;
        delay(500);
        Serial.print(".");
      }
    }
    lastCheck = millis();
  }
}
