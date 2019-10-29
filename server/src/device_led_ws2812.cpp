#include "device_led_ws2812.h"
#include "fout.h"


DeviceLedWS2812::DeviceLedWS2812(int size) :
	DeviceLed(50)
{
	m_size = size;
	m_leds = new Rgba[m_size];

	// init led driver
	ledstring.freq = TARGET_FREQ;
	ledstring.dmanum = DMA;
	channel0.gpionum = GPIO_PIN;
	channel0.count = m_size;
	channel0.invert = 0;
	channel0.brightness = 255;
	channel0.strip_type = STRIP_TYPE;
	ledstring.channel[0] = channel0;
	ws2811_return_t ret;
	if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
	{
		Fout{ Fout::LOG } << "DeviceLedWS2812: init failed: " << ws2811_get_return_t_str(ret) << endl;
	}
}


void DeviceLedWS2812::setLeds(const Rgba *leds)
{
	for (int i = 0; i < m_size; i++)
	{
		m_leds[i] = leds[i];
	}
	notifyChanged();
}


void DeviceLedWS2812::setLed(const int n, const Rgba & rgba)
{
	m_leds[n] = rgba;
	notifyChanged();
}


void DeviceLedWS2812::runnableNotifyChanged()
{
	ws2811_return_t ret;
	for (int i = 0; i < m_size; i++)
	{
		ledstring.channel[0].leds[m_size - 1 - i] =
			(int)((float)m_leds[i].blue() * m_leds[i].alpha() * BRIGHTNESS_MAX) << 16 |
			(int)((float)m_leds[i].green() * m_leds[i].alpha() * BRIGHTNESS_MAX) << 8 |
			(int)((float)m_leds[i].red() * m_leds[i].alpha() * BRIGHTNESS_MAX);
	}
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
	{
		Fout{ Fout::LOG } << "DeviceLedWS2812: render failed: " << ws2811_get_return_t_str(ret) << endl;
	}

	// 15 frames /sec
	usleep(1000000 / 15);
}
