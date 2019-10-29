#include "device_led.h"


DeviceLed::DeviceLed(int refreshRateMillis) :
	REFRESH_RATE_MILLIS(refreshRateMillis)
{
	lastRunTime = chrono::steady_clock::now() - chrono::milliseconds(REFRESH_RATE_MILLIS);
	threadWorker = new thread(&DeviceLed::runnableWrapper, this);
}

void DeviceLed::notifyChanged()
{
	{
		std::unique_lock<std::mutex> lock(mtxNotifyChanged);
		notifyChangedFlag = true;
	}
	cvNotifyChanged.notify_one();
}


void DeviceLed::runnableWrapper()
{
	while (true)
	{
		// consider refresh rate
		if ((chrono::steady_clock::now() - lastRunTime) < chrono::milliseconds(REFRESH_RATE_MILLIS))
			this_thread::sleep_until(lastRunTime + chrono::milliseconds(REFRESH_RATE_MILLIS));
		lastRunTime = chrono::steady_clock::now();

		// wait for notification
		{
			std::unique_lock<std::mutex> lock(mtxNotifyChanged);
			if (!notifyChangedFlag)
				cvNotifyChanged.wait(lock, [&]() { return notifyChangedFlag; });
			notifyChangedFlag = false;
		}
		
		// start runnable worker
		runnableNotifyChanged();
	}
}
