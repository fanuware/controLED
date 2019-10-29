#ifndef DEVICE_LED_74HC595_H
#define DEVICE_LED_74HC595_H

#include "device_led.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>


class DeviceLed74HC595: public DeviceLed
{
public:
	DeviceLed74HC595(int size);
	void setLeds(const Rgba *leds);
	void setLed(const int n, const Rgba &rgba);
private:
	enum class colorSet { RED, BLUE, MAGENTA, GREEN, YELLOW, CYAN, WHITE, OFF };
	void runnableNotifyChanged();
	const float BRIGHTNESS_MAX = 0.05;

	// led driver
	int m_size;
	Rgba *m_leds;
	uint8_t ledData[4]; // 0=red, 1=blue, 2=green, 3=rowMask
};

#endif
