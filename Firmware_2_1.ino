/*
  Firmware for the Aircraft covering Iron version 2.0
  Last Edited 4-21-2018
  Firmware Version 2.1
  Author: Tyler Doupe
*/

//Import Libraries

#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_MAX31855.h>
#include <LiquidCrystal.h>

//These 3 variables are the pins the thermocouple reader is connected to
#define MAXDO   12
#define MAXCS   11
#define MAXCLK  13
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

LiquidCrystal lcd(9, A1, 10, A2, A3, A4, A5, 7, 6, 5, 4);

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;
boolean encoderIncreased;
boolean encoderDecreased;
int lastMSB = 0;
int lastLSB = 0;


int mode = 1; //The current selction mode ex. temp or gain set
boolean gainSet; //If true we are in gainset mode, if false we are in temp set mode
long buttonPushed = 0; //A counter for how long the button has been held for in milliseconds
boolean buttonHeld = false; //Is the button currently being held?
double targetTemp; //The temperature we want the iron to be, obviously
double Kp = 2.12; // Proportional Gain
double Ki = 0.0031; //Integral Gain
double Kd = 0.015; //Derivative Gain
double Tadjust = 0.06; //Temperature adjustment factor
int tempRange = 400; //Max temprature that can be reached in free temp mode
double x = 0; //The current error, that is, actual temperature minus target temperature
double x1 = 0; //The error at the last time step, used for approximating the derivative
unsigned long lastTime = 0; //The time of the previous timestep in milliseconds
unsigned long T = 0; //The current time in milliseconds
double dt; //The width of one timestep used by the PID controller
double y = 0; //The duty cycle to be passed to the relay. 0-100
//double rawY; //The raw duty cycle, can be over or lower than 100%
double I; //Integral of error
double preset1 = 225;
double preset2 = 250;
double preset3 = 350;
int cycle = 2000; //The period of the PWM cycle in milliseconds
unsigned long Tstart = 0; //The time that a cycle started
boolean calc = true; //Recalculate duty cycle required?

int encoderPin1 = 2; //these pins can not be changed 2/3 are special pins
int encoderPin2 = 3;
int encoderButton = 14;
int outputPin = 8; //The pin that outputs a signal to the relay
int disp = 0;
double temp;
int count = 0; //General use counter
boolean higher = false;
boolean error = false;

void setup()
{
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Starting..."); //The message the iron controller displays when starting
  lcd.setCursor(0, 1);
  lcd.print("Firmware V 2.1");

  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  pinMode(encoderButton, INPUT);
  pinMode(outputPin, OUTPUT);


  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  digitalWrite(encoderButton, HIGH); //turn pullup resistor on


  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  //Retrieve settings from EEPROM
  EEPROM.get(64, Kp);
  EEPROM.get(128, Ki);
  EEPROM.get(192, Kd);
  EEPROM.get(255, Tadjust);
  EEPROM.get(320, preset1);
  EEPROM.get(384, preset2);
  EEPROM.get(448, preset3);
  //If there are no values stored in EEPROM use the default values
  if (isnan(Kp))
  {
    Kp = 2.00;
  }
  if (isnan(Ki))
  {
    Ki = 0.0025;
  }
  if (isnan(Kd))
  {
    Kd = 0.100;
  }
  if (isnan(Tadjust))
  {
    Tadjust = 0.0;
  }
  if (isnan(preset1))
  {
    preset1 = 225;
  }
  if (isnan(preset2))
  {
    preset2 = 250;
  }
  if (isnan(preset3))
  {
    preset3 = 350;
  }

  temp = thermocouple.readFarenheit(); //Read the iron temperature
  temp = temp - Tadjust * temp; //Adjusted temperature, approximates surface temp
  if (!isnan(temp))
  {
    targetTemp = temp;
  }
  delay(2000);
}

void loop()
{
  temp = thermocouple.readFarenheit(); //Read the iron temperature
  temp = temp - Tadjust * temp; //Adjusted temperature, approximates surface temp
  if (isnan(temp))
  {
    temp = targetTemp;
    error = true;
  }

  if (temp >= 380)
  {
    error = true;
  }

  //The actual PID algorithm
  x1 = x; //x1 = the error one timestep ago
  x = targetTemp - temp; //Calculate the current error
  lastTime = T;
  T = millis(); //The current time in milliseconds
  dt = T - lastTime; //The length of one timestep. (maybe doesn't need to be calculated every time?)
  I += x; //The integral of error since the program started. A future version may change this to only taking the integral over the last few seconds

  if ( higher && temp <= targetTemp - 3 )
  {
    I = 0;
    higher = !higher;
  }
  if ( !higher && temp >= targetTemp + 3)
  {
    I = 0;
    higher = !higher;
  }

  if (calc)
  {
    y = Kd * ((x - x1) / dt) + Kp * x + Ki * I; //This is the actual control algorithm, the heart of the program


    //rawY = y;
    if (temp < 100)
    {
      y = constrain(y, 0, 100); //If y is outside the possible range of values, limit to between 0 and 100% duty cycle
    }
    if (temp >= 100 && temp < 200)
    {
      y = constrain(y, 0, 50);
    }
    if (temp >= 200)
    {
      y = constrain(y, 0, 30);
    }


    //Turn on voltage to the relay for y percent of the cycle (PWM)
    //PWM must have a low frequency because the solid state relay this program was designed for takes ~8 ms to respond
    if (isnan(y))
    {
      y = 0;
    }

    Tstart = T;
    if (y > 1)
    {
      digitalWrite(outputPin, HIGH);
    }
    calc = false;
  }
  else
  {
    if (T >= Tstart + cycle * (y / 100) || temp > targetTemp)
    {
      digitalWrite(outputPin, 0);
    }
    if (T >= Tstart + cycle)
    {
      calc = true;
    }
  }

  //If there is no error, activate the relay
  /*
    if (!error)
    {
    digitalWrite(outputPin, HIGH);
    }
    delay(y/100*cycle);
    if (y < 100)
    {
    digitalWrite(outputPin, 0);
    delay((100 - y)/100*cycle);
    }
    else
    {
    digitalWrite(outputPin, 0);
    }
  */

  readEncoderButton();

  if (!gainSet)
  {
    switch (mode)
    {
      case 2:
        targetTemp = preset1;
        break;
      case 3:
        targetTemp = preset2;
        break;
      case 4:
        targetTemp = preset3;
        break;
    }
  }

  //Refresh the LCD every nth loop (currently every loop)
  if (disp == 1)
  {
    refreshScreen();
    disp = 0;
  }
  ++disp;
  error = false;
  Serial.println(temp);
}


