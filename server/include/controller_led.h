#ifndef CONTROLLER_LED_H
#define CONTROLLER_LED_H

#include "webserver.h"
#include "rgba.h"
#include "device_led.h"
#include "animation.h"

#include <string>
#include <map>

using namespace std;


class ControllerLed: public BroadcastModule
{
public:
    enum class Commands { FLASH, FADE };

    ControllerLed(int rows, int columns);
    void resetAll();
    void setCommandAll(Commands command);
    bool setColorAll(Rgba::Color color);
    Rgba getLed(const int n) { return m_leds[n]; }
    void setLed(const int n, const Rgba &rgba) { m_leds[n] = rgba; }
    void attachDeviceLed(DeviceLed* device);
    void addAnimation(string name, Animation *animation);
    bool runAnimation(string name);
private:
    void receiveBroadcast(string message);
    void receiveClientAdded();
    bool sendBroadcastAll(bool stopAnimation);
    int m_size;
    int m_rows;
    int m_columns;
    Rgba *m_leds;
    map<string, Animation*> m_animations;

    // local device led
    DeviceLed *m_deviceLed = nullptr;
};

#endif
