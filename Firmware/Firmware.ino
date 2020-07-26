#include <BleConnectionStatus.h>
#include "BleRudder.h"
#include "Arduino.h"

//#define DEBUG // uncomment this to enable logging

#define NUM_AXES 3
enum axes {rudder, leftBrake, rightBrake};
const int pins[] = {34, 36, 39};      // rudder on pin 34, left brake on pin 36, right brake on pin 39

#define SAMPLES_PER_READING           5 // Number of pot samples to take (to smooth the values)
#define DELAY_BETWEEN_SAMPLES_MS      4 // Delay in milliseconds between pot samples
#define DELAY_BETWEEN_HID_REPORTS_MS  5 // Additional delay in milliseconds between HID reports

int rawAnalogValue[NUM_AXES][SAMPLES_PER_READING];
char filteredValue[NUM_AXES];


BleRudder bleRudder;

void setup()
{

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB
  Serial.println("start");
#endif

  bleRudder.begin();
}

void loop()
{
  handleAxes();
  delay(DELAY_BETWEEN_HID_REPORTS_MS);
}

void handleAxes() {
  if (bleRudder.isConnected())
  {
    readAdc();
    filterValues();
    printValues();  // Print readings to serial port, remove if not debugging

    bleRudder.setAxes(filteredValue[rudder], 0, 0, 0, 0, 0, DPAD_CENTERED); // TODO: modify this to send all axis values
  }
}

void readAdc() {
  for (int i = 0 ; i < SAMPLES_PER_READING ; i++)
  {
    for (int pinIndex = 0; pinIndex < NUM_AXES; pinIndex++) {
      rawAnalogValue[pinIndex][i] = analogRead(pins[pinIndex]);
    }
    delay(DELAY_BETWEEN_SAMPLES_MS);
  }
}

void filterValues() {
  for (int pinIndex = 0; pinIndex < NUM_AXES; pinIndex++) {
    int reading = 0;
    for (int i = 0; i < SAMPLES_PER_READING ; i++)
    {
      reading += rawAnalogValue[pinIndex][i];
    }
    filteredValue[pinIndex] = map(reading / SAMPLES_PER_READING, 0, 4095, 127, -127);
  }
}

void printValues() {
#ifdef DEBUG
  Serial.print("Sent: ");
  Serial.print(filteredValue);
  Serial.print("\tRaw Avg: ");
  Serial.print(potValue);
  Serial.print("\tRaw: {");

  // Iterate through raw pot values, printing them to the serial port
  for (int i = 0 ; i < SAMPLES_PER_READING ; i++)
  {
    //      Serial.print(potValues[i]);

    // Format the values into a comma seperated list
    if (i == SAMPLES_PER_READING - 1)
    {
      Serial.println("}");
    }
    else
    {
      Serial.print(", ");
    }
  }
#endif
}
