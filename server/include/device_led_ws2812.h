#ifndef DEVICE_LED_WS2812_H
#define DEVICE_LED_WS2812_H

// led driver (note: original c library)
#include <stdint.h>
#include <unistd.h>
#include <cstdlib> 
#include "rpi_ws281x/ws2811.h"
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE              WS2811_STRIP_GBR

#include "device_led.h"



class DeviceLedWS2812: public DeviceLed
{
public:
	DeviceLedWS2812(int size);
	void setLeds(const Rgba *leds);
	void setLed(const int n, const Rgba &rgba);
private:
	void runnableNotifyChanged();
	const float BRIGHTNESS_MAX = 0.05;

	// led driver
	int m_size;
	ws2811_t ledstring;
	Rgba *m_leds;
	ws2811_channel_t channel0;
};

#endif
