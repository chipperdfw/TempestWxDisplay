# TempestWxDisplay
Arduino/TFT INI9341 Display for local Tempest Weather Station

Uses the Wemos D1 Mini Pro ESP8266 and a ILI9341 TFT display to show real-time data from a Tempest Weather Station on the same LAN.  It sniffs the UDP Packets sent by the Tempest and translates the data to the TFT display.  It does not require an internet connection or use the Tempest API feed.

Version History Description:
 * Ver 0.0.1 : Basic capture of local UDP Data from Tempest and basic display on 2.8" TFT ILI9341 Display 
 * Ver 0.0.4 : Ring & Bar graphs for Temp & Humidity 
 * Ver 0.0.5 : Color code voltage and adjust font size for 100+ degrees
 * Ver 0.0.6 : Separated WiFi into separate WiFi_Credentials.h file
 * 
 * To-Do:
 * Wind Gusts - formatting
 * Rain / Lightning - rain rate and lightning
 * Feels Like - under big temp
 * Forecast - top corners of temp?


