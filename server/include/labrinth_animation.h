#ifndef LABRINTH_ANIMATION_H
#define LABRINTH_ANIMATION_H

#include "animation.h"
#include <mutex>


class LabrinthAnimation : public Animation
{
	enum class eState {
		INIT_LABRINTH,
		SOLVE_LABRINTH
	};
	
	// override: animation
	bool runnable();
	bool controlLed(int led, Rgba color);
	eState state;
	mutex mxWaitSolve;
	condition_variable cvWaitSolve;
	int target;
public:
	LabrinthAnimation() : Animation() {
		target = m_size - 1;
	}
};

#endif