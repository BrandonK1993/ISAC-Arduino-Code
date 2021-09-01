/*
  ReadField
  
  Description: Demonstates reading from a public channel which requires no API key and reading from a private channel which requires a read API key.
               The value read from the public channel is the current outside temperature at MathWorks headquaters in Natick, MA.  The value from the
               private channel is an example counter that increments every 10 seconds.
  
  Hardware: Arduino MKR WiFi 1010
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires WiFiNINA library
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2020, The MathWorks, Inc.
*/

#include <WiFiNINA.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <AccelStepper.h>
#include <Servo.h>
#include "DHT.h"

#define DHTPIN 2                    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11               // DHT 11

#define motorPin1  7                // IN1 pin on the ULN2003A driver
#define motorPin2  8                // IN2 pin on the ULN2003A driver
#define motorPin3  9                // IN3 pin on the ULN2003A driver
#define motorPin4  10               // IN4 pin on the ULN2003A driver
#define PIN_A A0                    // Pin A0 for Proximity Sensor


int stepsPerRevolution = 64;        // steps per revolution
int degreePerRevolution = 5.625;    // degree per revolution
int flag_status = 0;
const int analogInPin = A2;
AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

char ssid[] = "Leefamily2.4";       // your network SSID (name) 
char pass[] = "67814455";           // your network password
int keyIndex = 0;                   // your network key Index number (needed only for WEP)
WiFiClient  client;

Servo servo;
DHT dht(DHTPIN, DHTTYPE);

// Thingspeak Channel Details
unsigned long ChannelNumber1 = 1480440;
unsigned long ChannelNumber2 = 1481771;
unsigned long ChannelNumber3 = 1489196;
unsigned int FieldNumber1 = 1;
unsigned int FieldNumber2 = 2;
unsigned int FieldNumber3 = 3;
unsigned int FieldNumber4 = 4;
unsigned int FieldNumber5 = 5;
unsigned int FieldNumber6 = 6;
unsigned int FieldNumber7 = 7;
unsigned int FieldNumber8 = 8;

const char * myWriteAPIKeyC1 = "HEM4RL1FUXVMJUIT";
const char * myWriteAPIKeyC2 = "DESJ20YPF57U50LY";
const char * myWriteAPIKeyC3 = "KDL4NPT1RS9HL6PF";

// Set default values for all sensor and actuator status check variables
int c1_f1_cover_control_check = 0;
int c1_f2_pump_control_check = 0;
int c1_f3_cleaner_control_check = 0;
int c1_f4_drain_control_check = 0;
int c1_f5_fan_control_check = 0;

int c2_f1_cover_status_check = 1;
int c2_f4_water_level_check = 30;
int c2_f8_humidity_level_check = 0;


void setup() 
{
  Serial.begin(9600);             // Initialize serial
  
  dht.begin();
  servo.attach(14);               // Pin 14 for Servo Motor Control (Lock Control)
  
  pinMode(6, OUTPUT);             // Pin 6 for Pump Control
  pinMode(5, OUTPUT);             // Pin 5 for Green LED Control (Ultrasonic Cleaner Control)
  pinMode(4, OUTPUT);             // Pin 4 for Right Drying Fan
  pinMode(3, OUTPUT);             // Pin 3 for Left Drying Fan
  pinMode(PIN_A, INPUT);          // Pin A0 for Proximity Sensor

  //Ensure all Pins are at LOW first
  digitalWrite(6, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
    
  stepper.setMaxSpeed(1000.0);     // set the Stepper max motor speed
  stepper.setAcceleration(50.0);   // set the Stepper acceleration
  stepper.setSpeed(200);           // set the Stepper current speed
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  
  ThingSpeak.begin(client);         //Initialize ThingSpeak
}

void loop() {
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(3000);     
    } 
    Serial.println("\nConnected");
  }

  int statusCode = 0;
  int Prox_Value = digitalRead(PIN_A);        //Proximity Sensor Value
  int Water_Value = analogRead(analogInPin);  //Water Level Sensor Value
  Water_Value = Water_Value*100/1024;
  float Humid_Value = dht.readHumidity();     //Humidity Sensor Value       

  //Print Values in Serial Monitor
  Serial.print("\nProximity Sensor Value = " ); 
  Serial.print(Prox_Value);
  
  Serial.print("\nWater Sensor Value = " );
  Serial.print(Water_Value); 
  Serial.print("%");

  Serial.print(F("\nHumidity Value "));
  Serial.print(Humid_Value);
  
  /*---------------------------------------------------------------------------------------------------------*/
  
  //Check if Proximity Sensor Value is same as default or same as previous
  if (Prox_Value != c2_f1_cover_status_check) 
  {
    //Send Prox_value to C2F1 "Status of Cover [1 for Closed | 0 for Opened]"
    int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber1, Prox_Value, myWriteAPIKeyC2);
    if(x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    
    //Set check variable as current value
    c2_f1_cover_status_check = Prox_Value;
  }
  
  /*---------------------------------------------------------------------------------------------------------*/
  
  //Check if Water Level Sensor Value is more than specified value
  if (Water_Value > c2_f4_water_level_check)
  {
    //Read and save C1F2 "Stop[0] / Start[1] Pumping water" value
    int c1_f2_pump_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber2);
    
    //Check if Water Pump is On
    if(c1_f2_pump_control_value == 1)
    {
      //Switch off Water Pump
      digitalWrite(6, LOW);
      Serial.println("\nPump has Stopped");
      
      //Send 0 to C1F2 "Stop[0] / Start[1] Pumping water" 
      int x = ThingSpeak.writeField(ChannelNumber1, FieldNumber2, 0, myWriteAPIKeyC1);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }

      //Send 0 to C2F3 "Status of Pump [0 for Off | 1 for On]"
      x = ThingSpeak.writeField(ChannelNumber2, FieldNumber3, 0, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      
      //Set check variable as current value
      c1_f2_pump_control_check = 0;
    }
  }
  
  /*---------------------------------------------------------------------------------------------------------*/
  
  //Read and save C1F1 "Unlock[0] / Lock[1] Cover" value
  int c1_f1_cover_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber1);
  statusCode = ThingSpeak.getLastReadStatus();

  //Check if successfully read value from Thingspeak
  if(statusCode == 200)
  {
    Serial.println("\nThingspeak value for Cover Control is: " + String(c1_f1_cover_control_value));
    
    //Check if Cover Control Value is same as default or same as previous
    if(c1_f1_cover_control_value != c1_f1_cover_control_check)
    {
      //Lock Cover if c1_f1_cover_control_value is 0
      if(c1_f1_cover_control_value == 0)
      {
        servo.write(90);
        Serial.println("\nCover is Unlocked");
      }

      //Unlock Cover if c1_f1_cover_control_value is 1
      else if(c1_f1_cover_control_value == 1)
      {
        servo.write(0);
        Serial.println("\nCover is Locked");
      }

      //Send c1_f1_cover_control_value to C2F1 "Status of Cover [1 for Closed | 0 for Opened]"
      int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber2, c1_f1_cover_control_value, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      
      //Set check variable as current value
      c1_f1_cover_control_check = c1_f1_cover_control_value;
    }
  }
  else
  {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  /*---------------------------------------------------------------------------------------------------------*/

  //Read and save C1F2 "Stop[0] / Start[1] Pumping water" value
  int c1_f2_pump_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber2);
  statusCode = ThingSpeak.getLastReadStatus();

  //Check if successfully read value from Thingspeak
  if(statusCode == 200)
  {
    Serial.println("Thingspeak value for Pump Control is: " + String(c1_f2_pump_control_value));
    
    //Check if Pump Control Value is same as default or same as previous
    if(c1_f2_pump_control_value != c1_f2_pump_control_check)
    {
      //Switch Off Pump if c1_f2_pump_control_value is 0
      if(c1_f2_pump_control_value == 0)
      {
        digitalWrite(6, LOW);
        Serial.println("\nPump has Stopped");
      }
      
      //Switch On Pump if c1_f2_pump_control_value is 1
      else if(c1_f2_pump_control_value == 1)
      {
        digitalWrite(6, HIGH);
        Serial.println("\nPump has Started");
      }
      
      //Send c1_f2_pump_control_value to C2F3 "Status of Pump [0 for Off | 1 for On]"
      int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber3, c1_f2_pump_control_value, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      
      //Set check variable as current value
      c1_f2_pump_control_check = c1_f2_pump_control_value;
    }
  }
  else
  {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  /*---------------------------------------------------------------------------------------------------------*/
  
  //Read and save C1F3 "Stop[0] / Start[1] Ultrasonic Cleaner" value
  int c1_f3_cleaner_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber3);
  statusCode = ThingSpeak.getLastReadStatus();

  //Check if successfully read value from Thingspeak
  if(statusCode == 200)
  {
    Serial.println("Thingspeak value for Cleaner Control is: " + String(c1_f3_cleaner_control_value));
    
    //Check if Ultrasonic Cleaner Control Value is same as default or same as previous
    if(c1_f3_cleaner_control_value != c1_f3_cleaner_control_check)
    {
      //Switch Off Cleaner if c1_f3_cleaner_control_value is 0
      if(c1_f3_cleaner_control_value == 0)
      {
        digitalWrite(5, LOW);
        Serial.println("\nCleaner has Stopped");
      }
      //Switch On Cleaner if c1_f3_cleaner_control_value is 1
      else if(c1_f3_cleaner_control_value == 1)
      {
        digitalWrite(5, HIGH);
        Serial.println("\nCleaner has Started");
      }
      
      //Send c1_f3_cleaner_control_value to C2F5 "Status of Ultrasonic Cleaner [0 for Off | 1 for On]"
      int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber5, c1_f3_cleaner_control_value, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      
      //Set check variable as current value
      c1_f3_cleaner_control_check = c1_f3_cleaner_control_value;
    }
  }
  else
  {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  /*---------------------------------------------------------------------------------------------------------*/

  //Read and save C1F4 "Close[0] / Open[1] Drainage Valve" value 
  int c1_f4_drain_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber4);
  statusCode = ThingSpeak.getLastReadStatus();

  //Check if successfully read value from Thingspeak
  if(statusCode == 200)
  {
    Serial.println("Thingspeak value for Drainage Control is: " + String(c1_f4_drain_control_value));
    
    //Check if Drain Valve Control Value is same as default or same as previous
    if(c1_f4_drain_control_value != c1_f4_drain_control_check)
    {
      //Close Drain Valve if c1_f4_drain_control_value is 0
      if(c1_f4_drain_control_value == 0)
      {
        if (!stepper.isRunning() && stepper.distanceToGo() == 0 && stepper.currentPosition()>0)
        {
        stepper.moveTo(degToSteps(0));
        }
        while (stepper.distanceToGo()!=0){
           stepper.run();    
        }
        Serial.println("\nDrain valve is Closed");
      }
      //Open Drain Valve if c1_f4_drain_control_value is 1
      else if(c1_f4_drain_control_value == 1)
      {
        stepper.runToNewPosition(degToSteps(90));
        Serial.println("\nDrain valve is Opened");
      }

      //Send c1_f4_drain_control_value to C2F6 "Status of Drainage Valve [0 for Closed | 1 for Opened]"
      int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber6, c1_f4_drain_control_value, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      
      //Set check variable as current value
      c1_f4_drain_control_check = c1_f4_drain_control_value;
    }
  }
  else
  {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  /*---------------------------------------------------------------------------------------------------------*/ 

  //Read and save C1F5 "Stop[0] / Start[1] Drying Fans" value
  int c1_f5_fan_control_value = ThingSpeak.readFloatField(ChannelNumber1, FieldNumber5);  
  statusCode = ThingSpeak.getLastReadStatus();

  //Check if successfully read value from Thingspeak
  if(statusCode == 200)
  {
    Serial.println("Thingspeak value for Fan Control is: " + String(c1_f5_fan_control_value));

    //Check if Fan Control Value is same as default or same as previous
    if(c1_f5_fan_control_value != c1_f5_fan_control_check)
    {
      //Switch Off Fans if c1_f5_fan_control_value is 0
      if(c1_f5_fan_control_value == 0)
      {
        digitalWrite(4, LOW);
        digitalWrite(3, LOW);
        Serial.println("\nDrying Fans has Stopped");
      }
      //Switch On Fans if c1_f5_fan_control_value is 1
      else if(c1_f5_fan_control_value == 1)
      {
        digitalWrite(4, HIGH);
        digitalWrite(3, HIGH);
        Serial.println("\nDrying Fans has Started");
      }

      //Send c1_f5_fan_control_value to C2F7 "Status of Mounted Fans [0 for Off | 1 for On]"
      int x = ThingSpeak.writeField(ChannelNumber2, FieldNumber7, c1_f5_fan_control_value, myWriteAPIKeyC2);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }

      //Send Humid_Value to C3F1 "Humidity Value"
      x = ThingSpeak.writeField(ChannelNumber3, FieldNumber1, Humid_Value, myWriteAPIKeyC3);
      if(x == 200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }

      //Set check variable as current value
      c1_f5_fan_control_check = c1_f5_fan_control_value;
    }
  }
  else
  {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  /*---------------------------------------------------------------------------------------------------------*/
        
}

// function to convert deg to steps for Stepper Motor Control
float degToSteps(float deg)
{
  return (stepsPerRevolution / degreePerRevolution) * deg;
}
