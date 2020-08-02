#include <BleConnectionStatus.h>
#include "BleRudder.h"
#include "Arduino.h"

//#define DEBUG // uncomment this to enable logging

#define NUM_AXES 3
enum axes {leftBrake, rightBrake, rudder};
const int pins[] = {34, 36, 39};      // left brake on pin 34, right brake on pin 36, rudder on pin 39

#define MIDDLE_THRESHOLD              {2048, 2048, 2048}
#define SAMPLES_PER_READING           5 // Number of pot samples to take (to smooth the values)
#define DELAY_BETWEEN_SAMPLES_MS      4 // Delay in milliseconds between pot samples
#define DELAY_BETWEEN_HID_REPORTS_MS  5 // Additional delay in milliseconds between HID reports

int rawAnalogValue[NUM_AXES][SAMPLES_PER_READING];
int maxValue[NUM_AXES] = MIDDLE_THRESHOLD;
int minValue[NUM_AXES] = MIDDLE_THRESHOLD;
char filteredValue[NUM_AXES];


BleRudder bleRudder("Pro Pedals BT", "CH Products");

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
    printValues();

    bleRudder.setAxes(filteredValue[leftBrake], filteredValue[rightBrake], filteredValue[rudder]);
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
  for (int axisIndex = 0; axisIndex < NUM_AXES; axisIndex++) {
    int reading = 0;
    int average = 0;
    for (int i = 0; i < SAMPLES_PER_READING ; i++)
    {
      reading += rawAnalogValue[axisIndex][i];
    }
    average = reading / SAMPLES_PER_READING;
    minValue[axisIndex] = min(minValue[axisIndex], average);
    maxValue[axisIndex] = max(maxValue[axisIndex], average);
    filteredValue[axisIndex] = map(average, minValue[axisIndex], maxValue[axisIndex], 127, -127);
  }
}

void printValues() {
#ifdef DEBUG
  Serial.print("Left brake: ");
  Serial.print((signed int)filteredValue[leftBrake]);
  Serial.print("\tmin: "); Serial.print(minValue[leftBrake]);
  Serial.print("\tmax: "); Serial.print(maxValue[leftBrake]);
  Serial.print("\tRight brake: ");
  Serial.print((signed int)filteredValue[rightBrake]);
  Serial.print("\tmin: "); Serial.print(minValue[rightBrake]);
  Serial.print("\tmax: "); Serial.print(maxValue[rightBrake]);
  Serial.print("\tRudder: ");
  Serial.print((signed int)filteredValue[rudder]);
  Serial.print("\tmin: "); Serial.print(minValue[rudder]);
  Serial.print("\tmax: "); Serial.println(maxValue[rudder]);
#endif
}
