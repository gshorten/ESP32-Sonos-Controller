# ESP32 based portable Sonos music system controller
This is a battery powered Sonos music system controller using a TTGO ESP32 module ( http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1126&FId=t3:50033:3 )

I'll be adding additional documentation, but for now the primary purpose of this repository is to store firmware binary file for automatic OTA updates of the firmware over a WiFi connection.

When a controller is started it checks this repository for the latest version.  If a later version exists
it does an Over the Air (OTA) update.
