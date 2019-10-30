#ifndef DEVICE_LED_H
#define DEVICE_LED_H

#include "rgba.h"

#include <string>
#include <array>
#include <chrono>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>


class DeviceLed
{
public:
    virtual void setLeds(const Rgba *leds) = 0;
    virtual void setLed(const int n, const Rgba &rgba) = 0;
protected:
    DeviceLed(int refreshRateMillis);
    const int REFRESH_RATE_MILLIS;
    virtual void notifyChanged();
    virtual void runnableNotifyChanged() {};
private:
    void runnableWrapper();
    thread *threadWorker = nullptr;
    

    chrono::steady_clock::time_point lastRunTime;
    std::mutex mtxStartStop;

    bool notifyChangedFlag = false;
    std::mutex mtxNotifyChanged;
    std::condition_variable cvNotifyChanged;
};

#endif
