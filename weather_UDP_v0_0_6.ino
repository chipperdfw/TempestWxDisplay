/*
 * ESP8266WiFi Tempest Weather Display
 * Created by Chip Coker KD4C
 * This sketch uses the ESP8266WiFi library, the WiFiUdp.h functions, and the ArduinoJson.h (Version 5) functions,
 * the Adafruit_GFX.h and the Adafruit_ILI9341.h graphics library (for display to the ILI9341 TFT display,
 * and relies on modified bar and meter functions from Bodmer
 *
 * Ver 0.0.1 : Basic capture of local UDP Data from Tempest and basic display on 2.8" TFT ILI9341 Display 
 * Ver 0.0.4 : Ring & Bar graphs for Temp & Humidity 
 * Ver 0.0.5 : Color code voltage and adjust font size for 100+ degrees
 * Ver 0.0.6 : 
 * 
 * To-Do:
 * Wind Gusts - formatting
 * Rain / Lightning - rain rate and lightning
 * Feels Like - under big temp
 * Forecast - top corners of temp?
 */
// Libraries
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>         // required to parse weather data - load highest version of v5.13.x
#include <Adafruit_GFX.h>        // include Adafruit graphics library
#include <Adafruit_ILI9341.h>    // include Adafruit ILI9341 TFT library

  
// Set WiFi credentials
const char ssid[] = "xxx";
const char pass[] = "xxx";
#define UDP_PORT 50222
 
// UDP
WiFiUDP UDP;
char packet[255];

//  Allocate JSON Buffer
//StaticJsonBuffer<2000> jsonBuffer;
//char json[255]; 

String windDir[] = {"NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW", "N"};

// TFT Display connection to ESP8266 Set-up
   #define TFT_CS    D2                                                         // TFT CS  pin is connected to NodeMCU pin D2
   #define TFT_RST   D3                                                         // TFT RST pin is connected to NodeMCU pin D3
   #define TFT_DC    D4                                                         // TFT DC  pin is connected to NodeMCU pin D4

// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)

   Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
  // Assign human-readable names to common 16-bit RGB 585 color values:
  #define BLACK       0x0000
  #define RED         0xF800
  #define GREEN       0x07E0
  #define BLUE        0x001F
  #define CYAN        0x07FF
  #define MAGENTA     0xF81F
  #define YELLOW      0xFFE0
  #define ORANGE      0xFBE0
  #define WHITE       0xFFFF
  #define GREY        0x5AEB

  #define CALLBKGD 0x001F  // Callsign background
  #define CKBKGD 0xFFFF  // Clock area background
  #define WXFGND 0x000A  // Weather area text
  #define WXBKGD 0xA7F4  // Weather area background 05E0
  #define FCBKGD 0xFFF9  // Forecast area background
  #define ALBKGD 0xFAE8  // Alert area background
  #define FC_AL_BKGD 0xFFF9 // Baseline background for forecasts & alerts

// Define regions
  const int centreX  = 274; // Location of the compass display on screen
  const int centreY  = 60;
  const int diameter = 41;  // Size of the compass


  
void setup() {
  // Setup serial port
  Serial.begin(9600);
  Serial.println();
  
// Initialize the TFT Display 
   tft.begin (); 
   tft.setRotation (3); //1,3 Landscape 0,2 Portrait
   tft.fillScreen(BLACK);
   tft.fillRoundRect(0, 0, 225, 25, 5, CALLBKGD);  //callsign area
   display_item(20, 5, "Tempest Weather", WHITE, 2);



// Connection Message   
   display_item(5, 230, ("WiFi: " + String(ssid)), WHITE, 1);


  // Begin WiFi
  WiFi.begin(ssid, pass);
  
  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  
  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
 
  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);

// After connecting display IP
   //IPAddress locIP;
   //tft.print("IP:");    tft.println(WiFi.localIP());
   //locIP = WiFi.localIP();
   tft.fillRect(5,230,100,10,0x0000);  // overwrite SSID
   display_item(5, 230, "WiFi:", WHITE, 1);
   tft.setTextSize(1);
   tft.print(WiFi.localIP());

  
}  // End Setup
 
void loop() {
 
   static int windLull;  // Make these persistent until updated
   static int windGust;

  // If packet received...
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    //Serial.print("Received packet! Size: ");
    //Serial.println(packetSize); 
    int len = UDP.read(packet, 255);
    if (len > 0)
    {
      packet[len] = '\0';
    }
    
    Serial.print("Packet: ");      Serial.println(packet);

    StaticJsonBuffer<500> jsonBuffer;

    JsonObject& doc = jsonBuffer.parseObject(packet);

    // Test if parsing succeeds.
    if (!doc.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    // Assign Values base on Packet Type
    const char *type = doc["type"];

    if (strcmp(type, "rapid_wind") == 0) {
      int windSpeed = int(2.236936 * float(doc["ob"][1])); //convert to MPH
      int windDeg = doc["ob"][2];

      String windCard = windDir[int(abs(windDeg - 11.25) / 22.5)];  // Convert wind dir to cardinal 16

      //  Serial.print("Wind: ");
      //  Serial.print(windCard);
      //  Serial.print(windSpeed); Serial.print("/"); Serial.println(windGust);

      Display_Wind (windSpeed, windDeg, windCard, windLull, windGust); // Display wind compass
    }

    if (strcmp(type, "obs_st") == 0) {
      //Serial.print("Type: ");    Serial.println(type);
      windLull = int(2.236936 * float(doc["obs"][0][1]));
      int windSpeed = int(2.236936 * float(doc["obs"][0][2]));
      windGust = int(2.236936 * float(doc["obs"][0][3]));
      int windDeg = doc["obs"][0][4];
      String windCard = windDir[int(abs(windDeg - 11.25) / 22.5)];  // Convert wind dir to cardinal 16
      
      float pressNow = (0.0302210 * float(doc["obs"][0][6]));
      float tempNow = (1.8 * float(doc["obs"][0][7])) + 32;
      float humidityNow = doc["obs"][0][8];
      int luxNow = int(doc["obs"][0][9]);
      float UVNow = doc["obs"][0][10];
      int solarNow = int(doc["obs"][0][11]);
      float rainNow = (60 * float(doc["obs"][0][12]))/25.4; // Convert mm/min to inches/hour
      int PrecipType = doc["obs"][0][13];

      float dewpointNow = tempNow - ((100 - humidityNow)/5); // Td = T - ((100 - RH)/5.)
  
      Serial.print("Observation: ");
      Serial.print("Temp; ");      Serial.println(tempNow);
      Serial.print("Hum; ");      Serial.println(humidityNow);
      Serial.print("Pressure; ");  Serial.println(pressNow); Serial.println(float(doc["obs"][0][6]));
      Serial.print("Solar; ");  Serial.println(solarNow); 
      Serial.print("Wind: "); Serial.print(windCard); Serial.print(windSpeed); Serial.print("/"); Serial.println(windGust);

      // Display screen elements
      Display_Wind (windSpeed, windDeg, windCard, windLull, windGust);
      Display_Temp (tempNow);
      Display_Humidity (humidityNow, dewpointNow);
      Display_Rain (rainNow, PrecipType);
      Display_Stats (humidityNow,pressNow, UVNow, solarNow, luxNow);
    }
    if (strcmp(type, "evt_precip") == 0) {
      Serial.print("Type: ");    Serial.println(type);
      Serial.print("Packet: ");      Serial.println(packet);
    }
    if (strcmp(type, "evt_strike") == 0) {
      int strike_distance = int(0.621371 * float(doc["evt"][1]));  // convert km to miles
      Serial.print("Type: ");    Serial.println(type);
      Serial.print("Distance: ");      Serial.println(strike_distance);
    }
    if (strcmp(type, "device_status") == 0) {
      Serial.print("Type: ");    Serial.println(type);
      //Serial.print("Packet: ");      Serial.println(packet);
      String stationId = doc["serial_number"];
      String voltage = doc["voltage"];
      int voltColor = WHITE;
      if (float(doc["voltage"])<2.445) {
        voltColor = YELLOW;        
      }
      if (float(doc["voltage"])<=2.39) {
        voltColor = ORANGE;        
      }
      if (float(doc["voltage"])<=2.355) {
        voltColor = RED;        
      }
      tft.fillRect(155,230,120,10,0x0000);  // overwrite station & voltage
      display_item(155, 230, stationId, WHITE, 1);
      display_item(225, 230, ("V:" + voltage), voltColor, 1);
    }
    if (strcmp(type, "hub_status") == 0) {
      Serial.print("Type: ");    Serial.println(type);
      //Serial.print("Packet: ");      Serial.println(packet);
      String stationId = doc["serial_number"];
      String rssi = doc["rssi"];
      tft.fillRect(127,230,20,10,0x0000);  // overwrite rssi
      display_item(130, 230, rssi, WHITE, 1);
      //display_item(150, 230, stationId, WHITE, 1);
    }


  } // End if(packetSize)
 
} // End Loop()

void Display_Wind(int windSpeed, int windDeg, String windCard, int windLull, int windGust) {  // Display wind compass and speed 
  tft.fillRoundRect(centreX - 45, centreY - 60, centreX + 65, (centreY + diameter / 2 + 70), 10, 0x000F); // Clear the compass
  tft.drawRoundRect((centreX - 45), (centreY - 60), (diameter + 50), (centreY + diameter / 2 + 70), 10, YELLOW); 
      Draw_Compass_Rose(); // Draw compass rose
      display_item((centreX - 10), (centreY - 55), "WIND", WHITE, 1);
      if (windSpeed < 10) display_item((centreX - 28), (centreY + 50), (String(windSpeed) + " MPH"), YELLOW, 2);
      else {
        display_item((centreX - 35), (centreY + 50), (String(windSpeed) + " MPH"), YELLOW, 2);
        if (windSpeed >= 18) display_item((centreX - 35), (centreY + 50), (String(windSpeed) + " MPH"), RED, 2);
      }
      if (windGust !=0) display_item((centreX - 20), (centreY + 68), (String(windLull) + "/" + String(windGust)), YELLOW, 2);
//      if (temp_strA.indexOf('G') >= 0) {
//        tft.fillRect(centreX - 40, centreY + 48, 82, 18, BLACK);
//        display_item((centreX - 40), (centreY + 50), String(wind_speedKTS) + "g" + temp_strA.substring(temp_strA.indexOf('G') + 1, temp_strA.indexOf('G') + 3) + temp_strB, YELLOW, 2);
//      }
//      int wind_direction = 0;
//      if (temp_strA.substring(0, 3) == "VRB") {
//        display_item((centreX - 15), (centreY - 5), "VRB", YELLOW, 2);
//      }
//      else {
//        wind_direction = temp_strA.substring(0, 3).toInt() - 90;
        int dx = (diameter * cos((windDeg - 90) * 0.017444)) + centreX;
        int dy = (diameter * sin((windDeg - 90) * 0.017444)) + centreY;
        arrow(dx, dy, centreX, centreY, 5, 5, YELLOW); // u8g.drawLine(centreX,centreY,dx,dy); would be the u8g drawing equivalent
//      }

}

void Display_Temp(float tempNow) {
  tft.fillRect(0, 30, 225, 165, 0x0000); // Clear the area
  tft.drawRoundRect(0, 30, 225, 165, 10, GREEN); // (x0,y0,w,h,r,color)
  display_item(85, 35, "TEMPERATURE", WHITE, 1);
  //display_item(centreX - 42, 155, ("Hum: " + String(humidityNow) + "%"), YELLOW, 1);
  //display_item(centreX - 42, 165, ("Bar: " + String(pressNow) + "in"), YELLOW, 1);
  //    display_item(5, 200, ("Temp: " + String(tempNow) + char(247)), WHITE, 2);
  ringMeter(tempNow, 0, 100, 35, 45, 75, "deg", 3); // Draw analog meter (val,min,max,x,y,r,unit,scheme)
}
void Display_Humidity(float humidityNow, float dewpointNow) {
  tft.fillRect(0, 197, 150, 30, 0x0000); // Clear the area
  tft.drawRoundRect(0, 197, 150, 30, 10, YELLOW); // (x0,y0,w,h,r,color)
  display_item(55, 200, "HUMIDITY", WHITE, 1);
  display_item(10, 205, (String(humidityNow) + "%"), YELLOW, 1);
  display_item(110, 205, (String(dewpointNow) + char(247)), YELLOW, 1);
  // horiz bar  
  linearMeter(int(humidityNow*1.3), 10, 214, 1, 10, 0, 130, 3);  //int val, int x, int y, int w, int h, int g, int n, byte s
}
void Display_Rain(float rainNow, int PrecipType) {
  switch (PrecipType) {  // Change fill color based on precip type
    case 0:
    tft.fillRoundRect(155, 197, 70, 30, 10, BLACK);
    break;
    case 1:
    tft.fillRoundRect(155, 197, 70, 30, 10, BLUE);
    break;
    case 2:
    tft.fillRoundRect(155, 197, 70, 30, 10, MAGENTA);
    break;
  }
  //if (PrecipType == 0) tft.fillRoundRect(155, 197, 70, 30, 10, BLACK);
  //else tft.fillRoundRect(155, 197, 70, 30, 10, BLUE); // Clear/set the background
  tft.drawRoundRect(155, 197, 70, 30, 10, YELLOW); // (x0,y0,w,h,r,color)
  display_item(170, 200, "PRECIP", WHITE, 1);
  display_item(165, 215, (String(rainNow)), YELLOW, 1);
  // fill for rain_event status
}
void Display_Stats(float humidityNow,float pressNow, float UVNow, int solarNow, int luxNow) {
  tft.fillRoundRect(centreX - 45, 155, diameter + 50, 68, 10, 0x0100); // Clear the background
  tft.drawRoundRect((centreX - 45), (155), (diameter + 50), 68, 10, GREEN); // (x0,y0,w,h,r,color)
  display_item((centreX - 15), (160), "STATS", WHITE, 1);
  display_item(centreX - 42, 175, ("Bar: " + String(pressNow) + "in"), YELLOW, 1);
  display_item(centreX - 42, 185, ("UV: " + String(UVNow)), YELLOW, 1);
  display_item(centreX - 42, 195, ("Sol: " + String(solarNow) + "W/m2"), YELLOW, 1);
  display_item(centreX - 42, 205, ("Lux: " + String(luxNow)), YELLOW, 1);
}

void display_item(int x, int y, String token, int txt_colour, int txt_size) {
  tft.setCursor(x, y);
  tft.setTextColor(txt_colour);
  tft.setTextSize(txt_size);
  tft.print(token);
  tft.setTextSize(2); // Back to default text size
}

void draw_veering_arrow(int a_direction) {
  int dx = (diameter * 0.75 * cos((a_direction - 90) * 3.14 / 180)) + centreX;
  int dy = (diameter * 0.75 * sin((a_direction - 90) * 3.14 / 180)) + centreY;
  arrow(centreX, centreY, dx, dy, 2, 5, RED);   // u8g.drawLine(centreX,centreY,dx,dy); would be the u8g drawing equivalent
}

void Draw_Compass_Rose() {
  int dxo, dyo, dxi, dyi;
  tft.drawCircle(centreX, centreY, diameter, GREEN); // Draw compass circle
  // tft.fillCircle(centreX,centreY,diameter,GREY);  // Draw compass circle
//  tft.drawLine(0, 105, 228, 105, YELLOW); // Seperating line for relative-humidity, temp, windchill, temp-index and dewpoint
//  tft.drawLine(132, 105, 132, 185, YELLOW); // Seperating vertical line for relative-humidity, temp, windchill, temp-index and dewpoint
  for (float i = 0; i < 360; i = i + 22.5) {
    dxo = diameter * cos((i - 90) * 3.14 / 180);
    dyo = diameter * sin((i - 90) * 3.14 / 180);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    tft.drawLine(dxo + centreX, dyo + centreY, dxi + centreX, dyi + centreY, YELLOW); // u8g.drawLine(centreX,centreY,dx,dy); would be the u8g drawing equivalent
    dxo = dxo * 0.5;
    dyo = dyo * 0.5;
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    tft.drawLine(dxo + centreX, dyo + centreY, dxi + centreX, dyi + centreY, YELLOW); // u8g.drawLine(centreX,centreY,dx,dy); would be the u8g drawing equivalent
  }
  display_item((centreX - 2), (centreY - 33), "N", GREEN, 1);
  display_item((centreX - 2), (centreY + 26), "S", GREEN, 1);
  display_item((centreX + 30), (centreY - 3), "E", GREEN, 1);
  display_item((centreX - 32), (centreY - 3), "W", GREEN, 1);
}

void arrow(int x1, int y1, int x2, int y2, int alength, int awidth, int colour) {
  float distance;
  int dx, dy, x2o, y2o, x3, y3, x4, y4, k;
  distance = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  //
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  tft.drawLine(x1, y1, x2, y2, colour);
  tft.drawLine(x1, y1, dx, dy, colour);
  tft.drawLine(x3, y3, x4, y4, colour);
  tft.drawLine(x3, y3, x2, y2, colour);
  tft.drawLine(x2, y2, x4, y4, colour);
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = ILI9341_RED; break; // Fixed colour
      case 1: colour = ILI9341_GREEN; break; // Fixed colour
      case 2: colour = ILI9341_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = ILI9341_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  //tft.setTextColor(WHITE, BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(text_colour, BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 70) display_item((x - 40), (y - 20), String(value), text_colour, 6);
//  if (r > 84) tft.drawCentreString(buf, x - 5, y - 20, 6); // Value in middle
  else display_item((x - 40), (y - 20), String(value), text_colour, 4); // Value in middle

  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(WHITE, BLACK);
  if (r > 70) display_item((x + 30), (y - 20), String(char(247)), text_colour, 4); // Units display
  else display_item((x + 30), (y - 20), String(char(247)), text_colour, 3); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
//  Draw the linear meter
// #########################################################################
// val =  reading to show (range is 0 to n)
// x, y = position of top left corner
// w, h = width and height of a single bar
// g    = pixel gap to next bar (can be 0)
// n    = number of segments
// s    = colour scheme
void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s)
{
  // Variable to save "value" text colour from scheme and set default
  int colour = BLUE;
  // Draw n colour blocks
  for (int b = 1; b <= n; b++) {
    if (val > 0 && b <= val) { // Fill in coloured blocks
      switch (s) {
        case 0: colour = RED; break; // Fixed colour
        case 1: colour = GREEN; break; // Fixed colour
        case 2: colour = BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(b, 0, n, 127,   0)); break; // Red to Blue
        case 4: colour = rainbow(map(b, 0, n, 0,   127)); break; // Blue to red
        case 5: colour = rainbow(map(b, 0, n,  63,   0)); break; // Green to red
        case 6: colour = rainbow(map(b, 0, n,   0,  63)); break; // Red to green
        case 7: colour = rainbow(map(b, 0, n,   0, 159)); break; // Rainbow (red to violet)
      }
      tft.fillRect(x + b*(w+g), y, w, h, colour);
    }
    else // Fill in blank segments
    {
      tft.fillRect(x + b*(w+g), y, w, h, GREY);
    }
  }
}


unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}
