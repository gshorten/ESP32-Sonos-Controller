/*
   Sonos controller using the Heltec Wifi Kit 32, an esp32 based board with wifi and on board 128 x 64
   OLED display, and battery charger (uses JST 1.25 connector).
   Uses this fork of SonosUPnP: https://github.com/gshorten/Sonos-ESP32
   TODO:
    Fixes / bugs / features:
      - Add an "about" screen that shows information - ip of the control, firmware version.
      - Use ReactESP library to improve responsiveness of the pushbutton and volume control.  Pretty major change though. Fork it.
      - On startup, if cannot connnect to active sonos unit, go to menu to pick a new one ,
      - change OLED display function to display either 3 or 4 lines, centered or not, diff. sizes
      - add method to  SonosUPnP to play sonos favorite  (difficult)
      - add method to  SonosUPnP to get meta data so that title
                      for radio stations and Siriusxm can be extracted (difficult)

  TODO Completed:
     - don't change volume of a  unit  until volume control has not been turned for 250ms ?
                      display value that it will change to for feedback.
      - Error handling to switch modes (running or setup) if sonos unit cant be found or wifi cant connect
      - main menu to go into setup mode (ap mode), maybe click 3 times?
       - add web based configuration option, ie go to a web page to define wifi connection, sonos  units, etc.
          program will look for and load these on startup.
       - cache sonos status to improve responsiveness of the volume control and pause/ play.  See ReactESP alternative in TODO

        - put scheduling for the status display update in a seperate function, this will
                      enable the status display to be shown on demand.
      - add function to select weather city

       - implement flag to stop sonos status display refresh or weather check,
                      display update while we are changing volume or in a menu
      - scan for available wifi networks
      - save and retrieve settings to NVS (Non Volatile Storage)
      - get track information ( at least for spotify, sonos playlists, tracks)
      - modify SonosUPnP fork to work with Heltec wifi Kit32
      - add battery % indicator?

  Apologies for all the global variables and constants, a consequence of not using classes :-) , but there's
  not actually any more code, it's just all in the .ino file.  I will clean up and make OOP later!
  Conventions:
      global and local constants    ALL_UPPER_CASE
      global variables              g_camelCase
      local variables               regularCamelCase
      function names                regularCamelCase
      functions are seperated with /////////////////////////////////////////////////////////
*/

#include <MicroXPath.h>         // used by SonosUPnP
#include <SonosUPnP.h>          // Sonos control library (modified GS version)
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include "heltec.h"             //Heltec WiFi Kit32 library for ESP32 processor`  
#include <WiFiUdp.h>
#include <ArduinoNvs.h>         // for storing config data in nvs
#include <ArduinoJson.h>        // for decoding weather data
#include <NTPClient.h>          //gets time from ntp server
#include <AceButton.h>          // for the encoder button
#include "font.h"               // extra fonts for OLED display
#include <ESP32Encoder.h>       // rotary encoder
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebConfig.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

// setup encoder and button
using namespace ace_button;
ESP32Encoder sonosEnc;
const int     LEFT_PIN = 17;
const int     RIGHT_PIN = 5;
const uint8_t BUTTON_PIN = 18;
AceButton     encButton(BUTTON_PIN);            // make button
const int     LED_PIN = 25;                     // on board LED
const int     CW = 1;             //clockwise turn of encoder returns 1
const int     CCW = 255;         //counterclockwise turn of encoder returns 255

// configuration settings webserver
WebServer server;
WebConfig conf;
WiFiClient webClient;

// Firmware update variables
#define URL_fw_Version "https://raw.githubusercontent.com/gshorten/GSCUpdates/master/ESP32_Sonos_Controller/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/gshorten/GSCUpdates/master/ESP32_Sonos_Controller/ESP32_Sonos_Controller.ino.wifi_kit_32.bin"

// Global Variables and Constants
boolean StatusDisplayOn = true;                 // flag to turn updating of the status display on or off
//boolean SelectionMade = false;                // when not in pause play mode indicates a selection has been made
long g_TimeDisplayStarted;
// ie when changing volume, other tasks, we want to stop updating the status display, otherwise do it automatically.
boolean SHOW_SPLASH = true;
boolean g_firstTime = true;           // flag so first time we run function we update the weather
boolean UPDATE_NOW = true;            // force update of sonos information
int g_State = 0;                      // Operating state, normal operating or setup, switches wifi between ap and client
const int OPERATING = 0;
const int SETUP = 1;
boolean g_TrackInfoAvailable = false;
long g_EncoderEvent = millis();                  // time that an encoder event started
boolean g_EncoderInUse = false;
boolean g_LowBattery = false;
const int BATT_PIN = 37;
String g_FirmwareVersion = "0.0";
boolean g_WeatherUpdateOn = true;

// struct to hold  track and playstate information on the active unit
typedef struct {
  String playState;
  int volume;
  String source;
  String title;
  String creator;
  String album;
} SonosState;
SonosState g_SonosInfo;

// state constants for the encoder and button functions
const int STATE_PLAYING = 0;
const int STATE_CHANGE_UNIT = 1;
int g_EncInputMode = STATE_PLAYING;           // default for the encoder is to change the volume
int g_SingleClickAction = STATE_PLAYING;     // default action when single button is pressed (pause - play)

// Date Time constants
const byte LONG_DATETIME = 0;
const byte LONG_DATE = 1;
const byte CURR_TIME = 2;
const byte SHORT_DATE = 3;

// struct to store sonos unit data
typedef struct sonosInf
{
  IPAddress UnitIPAddress;
  String  UnitName;
} SonosUnit;

const int NUM_SONOS_UNITS = 15;           // number of sonos units
SonosUnit g_SonosUnits[NUM_SONOS_UNITS];  // array of sonos unit ipaddresses and names
int g_CurrentUnitNum = 0;                 //when selecting the unit with volume control, tracks the g_SonosUnits selected
int g_numOfActiveUnits;                   //number of active sonos units
IPAddress NULL_IP = (0, 0, 0, 0);
int g_SonosMenuLength; // string array for scrolling through the list has space for additional options
String g_SonosMenuList[NUM_SONOS_UNITS + 2];     // String array for the list of sonos  units + options for scrolling

// struct to save configuration data
typedef struct {
  byte configSaved;                      // flag to indicate if the Configuration has been saved to EEPROM (is 25 if it is)
  String locationName;                    // city to show weather for
  String locationNameID;                  // open weather map.org location id for weather
  String locationUTC;                     //utc offset stored as a string (is converted to long before use)
  String wifiSSID;                       // wifi ssid
  String wifiPassword;                   // wifi pwd
  IPAddress currentSonosUnit;            // default sonos ip
  String currentSonosUnitName;
} configData;

configData CurrentConfig;                // current configuration

// struct to store weather data
typedef struct {
  int currTemp;             // current temperature
  int fcstTemp;             //forecast temperature( 4 hours out)
  int currWind;             //current wind
  int fcstWind;             // forecast wind
  String currShortDesc;     //current desc
  String fcstShortDesc;
  String currLongDesc;
  String fcstLongDesc;
} weatherInfo ;

weatherInfo g_Weather;  //global to hold weather information

// 2 dimensional array to hold location name, weather ID, and UTC offset
String g_Locations[10][3] = {
  // UTC offset is stored as a string, converted to long*-1 before use .
  {"Calgary", "5913490", "25200"},
  {"Almonte", "8125781", "18000"},
  {"Radium", "6115713", "25200"},
  {"Ottawa", "6094817", "18000"},
  {"Vancouver", "6173331", "28800"},
  {"New Toronto", "6087824", "18000"},
  {"Osoyoos", "6094665", "28800"},
  {"Montreal", "6077246", "18000"},
  {"Caledon", "5913449", "18000"},
  {"Downtown Toronto", "6167863", "18000"}
};

int NUM_LOCATIONS = 10;
int g_currSelectedLocation;        // currently selected city when scrolling through the location list

// SonosUPNP variables:
const int SONOS_STATUS_POLL_DELAY_MS = 10000;
const int SERIAL_DATA_THRESHOLD_MS = 500;
#define SERIAL_ERROR_TIMEOUT "E: Serial"
#define ETHERNET_ERROR_DHCP "E: DHCP"
#define ETHERNET_ERROR_CONNECT "E: Connect"
// unsigned long g_sonosLastStateUpdate = 0;
// unsigned long g_serialDataReceived_ms;

IPAddress g_ActiveUnit;       // the current sonos unit we are controlling.
String g_ActiveUnitName;      // active unit name

// error handler for sonos wifi, see below.
void ethConnectError() {
  Serial.println("wifi error, cannot connect to Sonos");
}

SonosUPnP g_sonos = SonosUPnP(webClient, ethConnectError); // make sonos object
FullTrackInfo ArtistTitle;      // for creator, title, artist
String CurrentTrackInfo;        // will hold source if radio or say sirius instead of artist, title.

String g_utcString;
int utcOffsetInSeconds = -25200;
WiFiUDP ntpUDP;
// NTPClient timeClient (ntpUDP, "pool.ntp.org", utcOffsetInSeconds, timeUpdateFreq); // Define NTP Client to get the time
NTPClient timeClient(ntpUDP);

//************ Function Prototypes **********************************
// only necessary if default arguments are needed, they must be defined in the function prototypes

void buttonEvent(AceButton* /*encButton*/, uint8_t eventType,
                 uint8_t buttonState);
weatherInfo getWeather();
void pausePlay();
void checkEncoder();
void displayText(String lines[3], int numLines = 3);
void scanNetworks(int numToScan);
boolean checkSSID(String wifiSSID);
void getSonosUnits(boolean showSplash = false);
void SaveConfig();
void RetrieveConfig();
void unitSelectClick();
void saveSonos();
// void exitMainMenu(boolean displaySplash = true);
void showStatus(int seconds = 2);
void updateSonosStatus(long updateInterval = 3000, boolean doNow = false);
String configString();
void firmwareUpdate();
int FirmwareVersionCheck();

// ***********************  Main Program ***************************************
//                             Setup
void setup() {
  Serial.begin(115200);   // start serial monitor
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  NVS.begin();            // start non volatile storage
  //NVS.eraseAll();      // only use for debugging

  // set up OLED display:
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setBrightness(255);

  String textToDisplay[3];
  textToDisplay[0] = "Portable Sonos Control";
  textToDisplay[1] = " Geoff Shorten 2020";
  textToDisplay[2] = "Checking Firmware, Getting WiFi";
  displayText(textToDisplay);

  // battery voltage detection settngs
  adcAttachPin(BATT_PIN);
  analogSetCycles(8);                   // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
  analogSetSamples(1);                  // Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
  analogSetClockDiv(1);                 // Set the divider for the ADC clock, default is 1, range is 1 - 255
  analogSetAttenuation(ADC_11db);       // Sets the input attenuation for ALL ADC inputs, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
  analogSetPinAttenuation(37, ADC_11db);

  // Configure the button handler
  // set the encoder button to use the built-in pull up register.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Configure the ButtonConfig with the event handler, and enable all higher level events.
  ButtonConfig* buttonConfig = encButton.getButtonConfig();
  buttonConfig->setEventHandler(buttonEvent);               // the handler for button events
  // enable button features:
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);          // enable single click
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);    // enable double click
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);  // makes both single and double click work
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);    // probably don't need this

  // set up the encoder
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = UP;
  // Attach pins for use as encoder pins, set encoder mode.
  sonosEnc.attachHalfQuad(LEFT_PIN, RIGHT_PIN);   // this seems to generate 2 events for each click, with modified encoder function works well
  // set starting count value after attaching
  sonosEnc.setCount(1000);        // could probably be anything, but can't go negative (its a uint16)

  ledcSetup(0, 5000, 8);
  ledcAttachPin(LED_PIN, 0);

  // get configuration
  // make configuration string (json file)
  String params = configString();
  conf.setDescription(params);
  // read configuration file
  conf.readConfig();
  // put configuration settings in config struct
  initWiFi();       // start wifi client and server

  if (g_State == OPERATING) {
    // we are in operating state, do normal init
    getSetup();       // populate CurrentConfig
    char dns[30];
    sprintf(dns, "%s.local", conf.getApName());
    Serial.print("dns :"); Serial.println(dns);
    if (MDNS.begin(dns)) {
      Serial.println("MDNS Responder Started");
    }
    // check for updates
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
    // Intro splash
    makeSonosIPList();
    printOutSonosList();

    // get weather
    // String weatherUpdate[3] = {"Getting current ", "weather conditions", "    for "};
    //weatherUpdate[2] += CurrentConfig.locationName;
    //displayText(weatherUpdate);

    // set up the NTP time client to get current date and time
    timeClient.begin();        // start time client
    //g_utcString = CurrentConfig.locationUTC;      // get current UTC offset
    g_utcString = "25200";
    utcOffsetInSeconds = g_utcString.toInt();      // convert to long
    utcOffsetInSeconds = utcOffsetInSeconds * -1;
    Serial.print("UTC String offset is: "); Serial.println(g_utcString);
    Serial.print("UTC offset is: "); Serial.println(utcOffsetInSeconds);
    timeClient.setTimeOffset(utcOffsetInSeconds);
    timeClient.setUpdateInterval(6000);         // update every 60 seconds
    timeClient.setPoolServerName("pool.ntp.org");

    // check that active unit in NVS is valid (not all 0.0.0.0)
    if (CurrentConfig.currentSonosUnit == NULL_IP) {
      // make the first unit in g_SonosUnits list the active unit
      g_ActiveUnit = g_SonosUnits[0].UnitIPAddress;
      g_ActiveUnitName = g_SonosUnits[0].UnitName;
    }
    else {
      g_ActiveUnit = CurrentConfig.currentSonosUnit;
      g_ActiveUnitName = CurrentConfig.currentSonosUnitName;
    }
    Serial.print("Active Sonos Unit is: "); Serial.println(g_ActiveUnit);

    //updateSonosStatus(2000, true);
    StatusDisplayOn = true;
    statusDisplay();
  }
  else {
    // state is setup
    Serial.println(" no wifi, bypassed boot - go to setup");
    String splash[3];
    splash[0] = "No WiFi or Sonos connection";
    splash[1] = "Use web browser on";
    splash[2] = "phone or tablet to setup WiFi & Sonos Players";
    displayText(splash);
    server.on("/", handleRoot);
    server.begin(80);
    StatusDisplayOn = false;
    delay(8000);
    displaySetupInstructions();
  }
}

void loop() {
  if ( g_State == SETUP) {
    server.handleClient();
  }
  else if (g_State == OPERATING) {
    if (!g_EncoderInUse) {             // only get weather, time, update display if rotary contoller is not active
      g_Weather = getWeather();        // get the weather, saves info in struct g_Weather
      updateSonosStatus(4000, !UPDATE_NOW);
      timeClient.update();             // checks for current time (used in status display)
      showStatus(3);                   // show the status display
    }
    checkEncoder();                  // check to see if encoder has turned
    encButton.check();               // checks for button presses
    DisplayTimeout(1);               // times out display so OLED does not fade. timeout parameter is in minutes
    encoderTimer();                  // check to see if encoder is in use
    pwrLED();                        // check to see if power LED should blink ( low power)
  }
}
