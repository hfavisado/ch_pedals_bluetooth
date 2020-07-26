#ifndef ESP32_BLE_GAMEPAD_H
#define ESP32_BLE_GAMEPAD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BleConnectionStatus.h"
#include "BLEHIDDevice.h"
#include "BLECharacteristic.h"

class BleRudder {
  private:
    uint16_t _buttons;
    BleConnectionStatus* connectionStatus;
    BLEHIDDevice* hid;
    BLECharacteristic* inputGamepad;
    void buttons(uint16_t b);
    void rawAction(uint8_t msg[], char msgSize);
    static void taskServer(void* pvParameter);
  public:
    BleRudder(std::string deviceName = "ESP32 BLE Gamepad", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
    void begin(void);
    void end(void);
    void setAxes(signed char x, signed char y, signed char z);
    bool isConnected(void);
    void setBatteryLevel(uint8_t level);
    uint8_t batteryLevel;
    std::string deviceManufacturer;
    std::string deviceName;
  protected:
    virtual void onStarted(BLEServer *pServer) { };
};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_GAMEPAD_H
