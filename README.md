# TempestWxDisplay
Arduino/TFT INI9341 Display for local Tempest Weather Station

This sketch sniffs the local LAN (where the Tempest hub is connected) for the UDP packets containing the raw weather data from the Tempest.  Rather than using the Tempest station page (on tempestwx.com), you can view your local display for the current weather info.  The information on the display is updated at the same frequency as the UDP packets (usually Wind every 3 secs and other weather every minute).

The current display looks like this:

<image src="https://user-images.githubusercontent.com/101214995/209420422-6c1d8c93-b198-45ed-a663-33c2848cbdfd.png" width="500"/>



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


Wiring the display to the ESP8266 WEMOS is simple.  You can use this as a guide:
https://thesolaruniverse.wordpress.com/2021/05/02/wiring-an-ili9341-spi-tft-display-with-esp8266-based-microcontroller-boards-nodemcu-and-wemos-d1-mini/

I also printed a nice case for the Display and ESP8266.  You can find the case here:
https://www.thingiverse.com/thing:3461768/files


I use the Arduino IDE to upload directly to the ESP8266.  You can use the code as-is but you must add your WiFi Credentials to the WiFi_Credentials.h file so that the ESP8266 will connect to your WiFi network and find your Tempest hub.

