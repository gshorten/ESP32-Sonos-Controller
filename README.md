# ESP32 based portable Sonos music system controller
This is a battery powered Sonos music system controller using a ESP32 module with integrated OLED display, bluetooth, wifi, and battery management. https://heltec.org/project/wifi-kit-32/

It has a rotary encoder with a pushbutton.  You can select sonos units to control, change the volume, pause, play, and skip tracks.  It also gets local weather and time, and displays track information and battery status on the TFT display.  The battery provides aproximately 30 hours of use.  

I'll be adding additional documentation, but for now the primary purpose of this repository is to store firmware binary file for automatic OTA updates of the firmware over a WiFi connection.  For now read the code for documentation. 

This uses a fork of a library by https://github.com/tmittet/sonos.  The fork adapts the library to use the ESP32 processor. The library is here: https://github.com/gshorten/Sonos-ESP32. 

When a controller is started it checks this repository for the latest version.  If a later version exists
it does an Over the Air (OTA) update.

Instructions for use:
https://sites.google.com/shortens.ca/sonoscontroller/home 
