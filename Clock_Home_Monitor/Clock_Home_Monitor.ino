/*
   Arduino IoT Clock and Home Monitor
   Author: Toglefritz
*/
/*  /////////////////////////
    ///    SKETCH INFO    ///
    /////////////////////////
*/
/*
  DESCRIPTION: 
    This sketch for the Arduino Uno controls a clock and IoT dashboard that is used as a quick and cnovenient way to monitor the environmental 
    conditions in and around your home. The dashboard (freeboard.io) displays information about outside weather conditions, inside climitalogical 
    conditions, inside noise levels, and inside sound levels. For an example of a working dashboard, visit [Freeboard URL].
    
    For detailed project instructions and bill of materials, visit the project page on Instructables:
    [Instructables URL]  

  LICENSE: 
    This code is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
    < https://creativecommons.org/licenses/by-nc-sa/4.0/ >
  
  PARTS:
    * Arduino Uno R3: < https://www.sparkfun.com/products/11021 >
    * SparkFun Wifi Shield (CC3000):  < https://www.sparkfun.com/products/12071 >
    * SparkFun Humidity and Temperature Sensor:  < https://www.sparkfun.com/products/12064 >
    * SparkFun Ambient Light Sensor:  < https://www.sparkfun.com/products/8688 >  
    * SparkFun Electret Microphone: < https://www.sparkfun.com/products/9964 > 
    * Project Enclosure:  < https://www.sparkfun.com/products/10088 >
    * Jumpter Wires
    * Nylon standoffs 
  
  HARDWARE CONNECTIONS:
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
*/

/*
    /////////////////////////
    ///   LOAD LIBRARIES  ///
    /////////////////////////
*/

// Load the CC3000 libraries from SparkFun, < https://github.com/sparkfun/SFE_CC3000_Library >. These libraries 
// provide methods that make it easier to use the CC3000 WiFi shield and facilitate communication between the 
// Arduino and a web server.
#include <SPI.h> 
#include <common.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Callbacks.h>
#include <SFE_CC3000_Client.h>
#include <SFE_CC3000_SPI.h>

// Load the library for communicating with the HTU21D temperature/humidity sensor ( < https://github.com/sparkfun/HTU21D_Breakout > )
#include <Wire.h>
#include <SparkFunHTU21D.h>

/* 
    //////////////////////////
    ///    CONFIGURATION   ///
    //////////////////////////

  The variables in this section will need to be changed to contain your own information.
*/
/*
    WiFi connection settings
*/
char ssid[] = "yourSSID";  // The name of your home WiFi
char password[] = "yourPassword";  // Your WiFi passphrase
unsigned int apSecurity = WLAN_SEC_WPA2; // Security of your home WiFi. The security mode is 
                                          // defined by one of the following:
                                          // WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2

/* 
    Unit Settings 
*/
    String tempUnit = "C";   // Units for temperature readings. Choose C for Celsius or F for Fahrenheit
/*
    Dweet.io parameters
*/
/* 
   Dweet.io is a service enabling IoT devices to store information online
   using an extremely simple API. Dweet.io does not use username/password credentials, rather, 
   it uses objects called "things" to store information. You must use a unique thing name for
   your home monitor. If you are using multiple Arduinos to monitor different areas
   of your home, each must have a unique thing name. For more inforamtion, visit < https://dweet.io/ >.
   By default, your thing will be publiclly accessible. We are not displaying any sensitive data but
   you might still want to give your thing a complicated name so nobody else can find it. For example, 
   you could use a thing name like "YourUsername_6Trz9OBjoVmM". If you want your data to be private, it
   only costs $0.99 per month for a "lock" < https://dweet.io/locks >.
*/
char thingName[] = "yourThingName_randomString";

/*
   Note:  Using Freeboard.io to display data
   
   We will be storing our sensor data online using Dweet.io, but we want a nice way to display the data.
   Freeboard is and IoT dashboard that takes data from various soruces, and displays the data in a 
   very beautiful, and extremely easy-to-read interface.
   Our Arduino doesn't need to know anything about Freeboard.io, as Freeboard.io mearly displays the data
   from Dweet.io. For more information on setting up Freeboard.io to display information from your Arduino
   home monitor, visit [Instructables URL]. Like with Dweet.io, your dashboard on Freeboard.io will be publically
   viewable by default. If you want it private, you can sign up for an account for $12 per month at 
   < https://freeboard.io/#pricing >.  
*/

/*  
   ////////////////////
   ///    SKETCH    ///
   ////////////////////
*/
// Define pins used for the CC3000 shield
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// Initialize the CC3000 objects (shield and client):
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

// Other WiFi connection settings
unsigned int timeout = 30000;   // Time to wait while attempting WiFi connection (ms)
char server[] = "www.dweet.io";   // Remote host site
// Caution:  Do not use http:// or https:// in the server variable. Not only will it not
// work with the CC3000, but also it might screw up your WiFi router (for some reason that I don't
// understand) requiring you to reboot the router. 

// Define sensor input pins
#define soundSensor A2
#define lightSensor A3
#define HTU21D_SDA A4
#define HTU21D_SCL A5

// Create an instance of the HTU21D object
HTU21D HTU21DSensor;

void setup() {
  // Set the sensor pins as inputs
  pinMode(soundSensor, INPUT);
  pinMode(lightSensor, INPUT);
  pinMode(HTU21D_SDA, INPUT);
  pinMode(HTU21D_SCL, INPUT);
  
  // Begin serial communication over the for reporting program status when the Arduino 
  // is connected to a computer 
  Serial.begin(115200);
  
  // Initialize CC3000 (configure SPI communications)
  ConnectionInfo connection_info;
  int i;
  Serial.println("Initializing CC3000");
  if (wifi.init()) {
    Serial.println("CC3000 initialization complete");
  } else {
    Serial.println("Something went wrong during CC3000 init!");
  }
  
  // Connect to the access point using the settings in the configuration section
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  if(wifi.connect(ssid, apSecurity, password, timeout)) {
    Serial.println("AP connection Established");
  }
  else { 
    Serial.println("Error: Could not connect to AP");
  }

  // Gather connection details and print IP address
  if ( !wifi.getConnectionInfo(connection_info) ) {
    Serial.println("Error: Could not obtain connection details");
  } else {
    Serial.print("Arduino IP Address: ");
    for (i = 0; i < IP_ADDR_LEN; i++) {
      Serial.print(connection_info.ip_address[i]);
      if ( i < IP_ADDR_LEN - 1 ) {
        Serial.print(".");
      }
    }
  }

  // Make a TCP connection to Dweet.io to send sensor readings
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(server);
  if (client.connect(server, 80)) {
    Serial.println("Connected to remote server");
  }
  else {
    Serial.println("Error: Could not make a TCP connection");
  }

  // Initialize the HTU21D
  HTU21DSensor.begin();

  Serial.println("Setup complete");
}

void loop() {    
  // Only take readings and Dweet sensor data every 20 seconds
  if(millis() % 20000 == 0) {       
    // Read temperature and humidity using the HTU21D sensor
    int humd = round(HTU21DSensor.readHumidity());
    int temp = round(HTU21DSensor.readTemperature());
    // If temperature units should be in F, convert the default C value
    if(tempUnit == "F") {
      temp = temp * 1.8 + 32;  
    }

    // Read ambient light level
    int lightLevel = analogRead(lightSensor);

    // Read from sound sensor
    int soundLevel = analogRead(soundSensor);
    
    // Report sensor data
    Serial.println();
    Serial.println("Dweeting sensor data:");
    Serial.print("  Temperature = ");
    Serial.print(temp);
    Serial.println(tempUnit);
    Serial.print("  Humidity = ");
    Serial.print(humd);
    Serial.println("%");
    Serial.print("  Light Level = ");
    Serial.println(lightLevel);
    Serial.print("  Sound Level = ");
    Serial.println(soundLevel);
  
    // Send sensor readings to Dweet,io via an HTTP POST request  
    // The URI for the POST request should look something like
    // /dweet/for/my-thing-name?hello=world
    // See < http://code.tutsplus.com/tutorials/http-headers-for-dummies--net-8039 > for 
    // understanding HTTP requests
    client.print("POST /dweet/for/");   // Beginning of URI to send Dweets
    client.print(thingName);            // Unique thing name (defined above)
    client.print("?temperature=");      // Temperature
    client.print(temp);
    client.print("&humidity=");         // Humidity
    client.print(humd);
    client.print("&lightlevel=");        // Light level
    client.print(lightLevel);
    client.print("&soundlevel=");        // Sound level
    client.print(soundLevel);
    client.println(" HTTP/1.1");        // This is the protocol for the request
    client.print("Host: ");             // The host field is required for all HTTP requests 
    client.println(server);
    client.println();                   // A blank line signals the end of the requests
  }    
}

/* 
  Note:  Adding custom sensors
  You can add just about any sensor you want to your Arduino clock/home monitor. All you need to do to make it
  work is (1) within loop() take a reading from the sensor, and (2) add the reading to the Dweet POST request
  by adding something like:

  client.print(&customSensorName=);
  client.println(customSensorReadingVar);

  Add this after the similar lines for the exisitng sensors, but before the client.println(" HTTP/1.1"); line.
*/

