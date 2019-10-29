#include "device_led_74hc595.h"


DeviceLed74HC595::DeviceLed74HC595(int size) :
	DeviceLed(50)
{
	m_size = size;
	m_leds = new Rgba[m_size];

	// init led driver
	wiringPiSetup();
	wiringPiSPISetup(0, 500000);
}


void DeviceLed74HC595::setLeds(const Rgba *leds)
{
	for (int i = 0; i < m_size; i++)
	{
		m_leds[i] = leds[i];
	}
	notifyChanged();
}


void DeviceLed74HC595::setLed(const int n, const Rgba & rgba)
{
	m_leds[n] = rgba;
	notifyChanged();
}


void DeviceLed74HC595::runnableNotifyChanged()
{
	bool anyLedOn;
	do
	{
		anyLedOn = false; // assume

		// generate rgb data
		const int RED = 0, GREEN = 2, BLUE = 1;
		uint8_t colorSet[3][8];
		for (int i = 0; i < 8; i++)
		{
			colorSet[RED][i] = 0xff; // red
			colorSet[GREEN][i] = 0xff; // green
			colorSet[BLUE][i] = 0xff; // blue

			// row by row
			for (int j = 0; j < 8; j++)
			{
				int n = (i * 8) + j;
				float average = (
					(float)m_leds[n].red() * m_leds[n].alpha() +
					(float)m_leds[n].green() * m_leds[n].alpha() +
					(float)m_leds[n].blue() * m_leds[n].alpha()) /
					3.0;
				bool redOn = false;
				bool greenOn = false;
				bool blueOn = false;
				if (average > 20.0) {
					float compRange = (average - 1.0);
					redOn = (float)m_leds[n].red() * m_leds[n].alpha() >= compRange;
					greenOn = (float)m_leds[n].green() * m_leds[n].alpha() >= compRange;
					blueOn = (float)m_leds[n].blue() * m_leds[n].alpha() >= compRange;
				}
				anyLedOn |= redOn | greenOn | blueOn;
				colorSet[RED][i] = colorSet[RED][i] & ~(redOn << 7 - j);
				colorSet[GREEN][i] = colorSet[GREEN][i] & ~(greenOn << 7 - j);
				colorSet[BLUE][i] = colorSet[BLUE][i] & ~(blueOn << 7 - j);
			}
		}

		// write red
		for (int i = 0; i < 8; i++)
		{
			ledData[0] = 0xff;
			ledData[2] = 0xff;
			ledData[1] = 0xff;

			// red, green and blue
			for (int c = 0; c < sizeof(colorSet); c++)
			{
				ledData[0] = 0xff;
				ledData[2] = 0xff;
				ledData[1] = 0xff;
				ledData[c] = colorSet[c][i]; // override color
				ledData[3] = 1 << (7 - i);
				wiringPiSPIDataRW(0, ledData, sizeof(ledData));
				//this_thread::sleep_for(chrono::microseconds(100));
			}
		}

	} while (anyLedOn);
}
