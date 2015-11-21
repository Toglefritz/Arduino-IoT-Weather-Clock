# ARDUINO IOT WEATHER CLOCK
This sketch for the Arduino Uno controls a clock and IoT dashboard that is used as a quick and convenient way to monitor the environmental conditions in and around your home.

Arduino IoT Clock and Home Monitor
Author: Toglefritz

## SKETCH INFO
This sketch for the Arduino Uno controls a clock and IoT dashboard that is used as a quick and cnovenient way to monitor the environmental 
conditions in and around your home. The dashboard (freeboard.io) displays information about outside weather conditions, inside climitalogical 
conditions, inside noise levels, and inside sound levels. For an example of a working dashboard, visit [Freeboard URL].
    
For detailed project instructions and bill of materials, visit the project page on Instructables:
[Instructables URL]

## LICENSE  

This code is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
< https://creativecommons.org/licenses/by-nc-sa/4.0/ >
    
## PARTS
 * Arduino Uno R3: < https://www.sparkfun.com/products/11021 >
* SparkFun Wifi Shield (CC3000):  < https://www.sparkfun.com/products/12071 >
* SparkFun Humidity and Temperature Sensor:  < https://www.sparkfun.com/products/12064 >
* SparkFun Ambient Light Sensor:  < https://www.sparkfun.com/products/8688 >  
* SparkFun Electret Microphone: < https://www.sparkfun.com/products/9964 > 
* Project Enclosure:  < https://www.sparkfun.com/products/10088 >
* Jumpter Wires
* Nylon standoffs
    
## HARDWARE CONNECTIONS
The circuit for this project basically consists of an Arduino Uno board with a CC3000 WiFi shield (< https://www.sparkfun.com/products/12071 >) 
connected to a number of environmental sensors, plus a tricolor LED breakout for simple status indication.

Signals
-------
* Microphone AUD --> A2
* Ambient Light Sensor SIG --> A3
* HTU21D SDA --> A4
* HTU21D SCL --> A5

Power
-----
* Microphone GND --> GND
* Microphone VCC --> 3.3V
* Ambient Light Sensor GND --> GND
* Ambient Light Sensor VCC --> 5V
* HTU21D - --> GND
* HTU21D + --> 3.3V       // Make sure the temperature/humdity sensor is connected to 3.3V, not 5V

The CC3000 WiFi shield obviously just plugs into the Arduino Uno the same way as any other shield.
    
## REQUIRED LIBRARIES
* CC3000 libraries from SparkFun, < https://github.com/sparkfun/SFE_CC3000_Library >
* HTU21D temperature/humidity sensor library ( < https://github.com/sparkfun/HTU21D_Breakout > )
    
# CONFIGURATION
Before upload the sketch to an Arduino Uno board, some values in the sketch must be adjusted to match 
your unique details:
    
  85   char ssid[] = "yourSSID";  // The name of your home WiFi
  86   char password[] = "yourPassword";  // Your WiFi passphrase
  
  94   String tempUnit = "C";   // Units for temperature readings. Choose C for Celsius or F for Fahrenheit
  
  109  char thingName[] = "yourThingName_randomString";
  
### Adding custom sensors
You can add just about any sensor you want to your Arduino clock/home monitor. All you need to do to make it
work is (1) within loop() take a reading from the sensor, and (2) add the reading to the Dweet POST request
by adding something like:

  client.print(&customSensorName=);
  client.println(customSensorReadingVar);

Add this after the similar lines for the exisitng sensors, but before the client.println(" HTTP/1.1"); line.
