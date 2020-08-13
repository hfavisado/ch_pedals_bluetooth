#include <BleConnectionStatus.h>
#include "BleRudder.h"
#include "Arduino.h"

// #define DEBUG // uncomment this to enable logging

#ifdef DEBUG
#define BATTERYDEBUG
#define AXISDEBUG
#endif

#define SAMPLES_PER_READING 5          // Number of pot samples to take (to smooth the values)
#define DELAY_BETWEEN_SAMPLES_MS 4     // Delay in milliseconds between pot samples
#define DELAY_BETWEEN_HID_REPORTS_MS 5 // Additional delay in milliseconds between HID reports

// Battery readings depend on the values of the resistors in the voltage divider
#define BATTERY_FULL_ADC_READING 4095
#define BATTERY_EMPTY_ADC_READING 3790

#ifdef BATTERYDEBUG
#define BATTERY_UPDATE_INTERVAL_TICKS 100000
#else
#define BATTERY_UPDATE_INTERVAL_TICKS 1000000 * 2
#endif

typedef struct adcPin
{
  int pinNumber;
  long value;

  // Set default value to half of the maximum 12-bit ADC value
  int minValue = 4095 / 2;
  int maxValue = 4095 / 2;
};

#define NUM_AXES 3
enum axes
{
  X,
  Y,
  Rz
};

volatile bool checkBattery = false;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

BleRudder *bleRudder;

adcPin axisPins[NUM_AXES];
adcPin batteryLevelPin;

void setup()
{
  initPinNumbers();
  updateBatteryLevel();
  bleRudder = new BleRudder("Pro Pedals BT", "CH Products", batteryLevelPin.value);
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ; // wait for serial port to connect. Needed for native USB
  Serial.println("start");
#endif
  bleRudder->begin();
  setupTimerInterrupt();
}

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  checkBattery = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void loop()
{
  if (bleRudder->isConnected())
  {
    handleAxes();
    if (checkBattery)
    {
      updateBatteryLevel();
      bleRudder->setBatteryLevel(batteryLevelPin.value);
    }
    delay(DELAY_BETWEEN_HID_REPORTS_MS);
  }
}

void handleAxes()
{
  for (int pinIndex = 0; pinIndex < NUM_AXES; pinIndex++)
  {
    readAdc(&axisPins[pinIndex]);
    axisPins[pinIndex].value = map(axisPins[pinIndex].value, axisPins[pinIndex].minValue, axisPins[pinIndex].maxValue, 127, -127);
  }
  bleRudder->setAxes(axisPins[X].value, axisPins[Y].value, axisPins[Rz].value);
  printAxisReadings();
}

void updateBatteryLevel()
{
  int batteryLevel;
  readAdc(&batteryLevelPin);
  batteryLevelPin.value = map(batteryLevelPin.value, BATTERY_EMPTY_ADC_READING, BATTERY_FULL_ADC_READING, 0, 100);
  batteryLevelPin.value = max(0, (int)batteryLevelPin.value);
  printBatteryReadings();
  checkBattery = false;
}

void readAdc(adcPin *pin)
{
  pin->value = 0;
  for (int i = 0; i < SAMPLES_PER_READING; i++)
  {
    pin->value += analogRead(pin->pinNumber);
    delay(DELAY_BETWEEN_SAMPLES_MS);
  }
  pin->value /= SAMPLES_PER_READING;
  pin->minValue = min(pin->minValue, (int)pin->value);
  pin->maxValue = max(pin->maxValue, (int)pin->value);
}

void initPinNumbers()
{
  axisPins[X].pinNumber = 34;
  axisPins[Y].pinNumber = 36;
  axisPins[Rz].pinNumber = 39;
  batteryLevelPin.pinNumber = 33;
}

void setupTimerInterrupt()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, BATTERY_UPDATE_INTERVAL_TICKS, true);
  timerAlarmEnable(timer);
}

void printAxisReadings()
{
#ifdef AXISDEBUG
  Serial.print("Left brake: ");
  Serial.print((signed int)axisPins[X].value);
  Serial.print("\tmin: ");
  Serial.print(axisPins[X].minValue);
  Serial.print("\tmax: ");
  Serial.print(axisPins[X].maxValue);
  Serial.print("\tRight brake: ");
  Serial.print((signed int)axisPins[Y].value);
  Serial.print("\tmin: ");
  Serial.print(axisPins[Y].minValue);
  Serial.print("\tmax: ");
  Serial.print(axisPins[Y].maxValue);
  Serial.print("\tRudder: ");
  Serial.print((signed int)axisPins[Rz].value);
  Serial.print("\tmin: ");
  Serial.print(axisPins[Rz].minValue);
  Serial.print("\tmax: ");
  Serial.println(axisPins[Rz].maxValue);
#endif
}

void printBatteryReadings()
{
#ifdef BATTERYDEBUG
  Serial.print("\tBattery: ");
  Serial.println(batteryLevelPin.value);
#endif
}