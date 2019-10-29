#ifndef ANIMATION_H
#define ANIMATION_H

#include "rgba.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>


//////////////////////////////////////////////////
// Animation
// note: Abstract Class, defines all animations
class Animation
{
public:
	// override member
	void runAnimation(Rgba *leds, int rows, int columns, std::function<void()> updateCallback);
	// return true if command accepted
	virtual bool controlCommand(string cmd) { return false; }
	virtual bool controlLed(int led, Rgba color) { return false; }
	
	// static
	static bool stopAnimation();
	bool isRunning();
	static thread *s_threadRunnable;
protected:
	Rgba *m_leds;
	int m_size;
	int m_sizeRows;
	int m_sizeColumns;
	std::function<void()> m_updateCallback;
	bool sleepInterruptable(int sleepMillis);

	// override member
	// returns fails when animation done, true if interrupted
	virtual bool runnable() = 0;
private:
	static void runnableWrapper();

	static bool s_animationRunning;
	static Animation *s_currentAnimation;
	static Animation *s_setCurrentAnimation;

	static once_flag spawnThreadFlag;
	static bool s_animationStartStopFlag;
	static bool s_notifyNewAnimation;
	static std::mutex s_mtxAnimationStartStop;
	static std::condition_variable s_cvAnimationStartStop;
};


//////////////////////////////////////////////////
// FadeAnimation
class FadeAnimation : public Animation
{
private:
	bool runnable();
};


//////////////////////////////////////////////////
// SnakeAnimation
class BlinkAnimation : public Animation
{
private:
	bool runnable();
};

#endif
