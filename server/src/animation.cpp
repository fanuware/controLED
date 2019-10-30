#include "animation.h"


//////////////////////////////////////////////////
// Animation
thread *Animation::s_threadRunnable = nullptr;

once_flag Animation::spawnThreadFlag;
bool Animation::s_animationStartStopFlag = false;
bool Animation::s_notifyNewAnimation = false;
std::mutex Animation::s_mtxAnimationStartStop;
std::condition_variable Animation::s_cvAnimationStartStop;

bool Animation::s_animationRunning = false;
Animation *Animation::s_currentAnimation = nullptr;
Animation *Animation::s_setCurrentAnimation = nullptr;


bool Animation::stopAnimation()
{
    {
        std::unique_lock<std::mutex> lock(s_mtxAnimationStartStop);
        s_animationStartStopFlag = false;
    }
    s_cvAnimationStartStop.notify_one();
    // note: do NOT wait until !isRunning because outside-thread should not be slowed down
    return true;
}


bool Animation::isRunning()
{
    return s_animationRunning && s_currentAnimation == this;
}


bool Animation::sleepInterruptable(int sleepMillis)
{
    {
        std::unique_lock<std::mutex> lock(s_mtxAnimationStartStop);
        s_cvAnimationStartStop.wait_for(lock, std::chrono::milliseconds(sleepMillis), []() {
            return !s_animationStartStopFlag || s_notifyNewAnimation; });
        return !s_animationStartStopFlag || s_notifyNewAnimation;
    }
}


void Animation::runnableWrapper()
{
    while (true)
    {
        // wait for notification
        {
            s_animationRunning = false;
            std::unique_lock<std::mutex> lock(s_mtxAnimationStartStop);
            s_cvAnimationStartStop.wait(lock, [&]() { return s_animationStartStopFlag; });
            s_currentAnimation = s_setCurrentAnimation;
            s_notifyNewAnimation = false;
            s_animationRunning = true;
        }

        // start runnable worker
        if (!s_currentAnimation->runnable())
            stopAnimation();
    }
}


void Animation::runAnimation(Rgba *leds, int rows, int columns, std::function<void()> updateCallback)
{
    std::unique_lock<std::mutex> lock(s_mtxAnimationStartStop);
    m_leds = leds;
    m_size = rows * columns;
    m_sizeRows = rows;
    m_sizeColumns = columns;
    m_updateCallback = updateCallback;

    // spawn animation thread, once
    call_once(spawnThreadFlag, [&]() {
        s_threadRunnable = new thread(&Animation::runnableWrapper);
    });

    // notify start
    s_setCurrentAnimation = this;
    s_animationStartStopFlag = true;
    s_notifyNewAnimation = true;
    s_cvAnimationStartStop.notify_one();
}


//////////////////////////////////////////////////
// FadeAnimation
bool FadeAnimation::runnable()
{
    // constants
    const int DURATION_MILLIS = 5000;
    const int REPETITIONS = 3600000 / DURATION_MILLIS; // 1 hour
    const int SLEEP_MILLIS = 100;
    const int COLOR_COUNT = 6;

    // initialize colors and states of each led
    const Rgba colorSet[COLOR_COUNT] = {
        (Rgba(Rgba::Color::RED)),
        (Rgba(Rgba::Color::YELLOW)),
        (Rgba(Rgba::Color::GREEN)),
        (Rgba(Rgba::Color::TURQUOISE)),
        (Rgba(Rgba::Color::BLUE)),
        (Rgba(Rgba::Color::PURPLE)) };
    int ledState[m_size];
    for (int i = 0; i < m_size; i++)
    {
        ledState[i] = i * DURATION_MILLIS / m_size;
    }

    // run animation
    for (int j = 0, r = 0; j < DURATION_MILLIS || (j = 0) || r++ < REPETITIONS; j += SLEEP_MILLIS)
    {
        for (int i = 0; i < m_size; i++)
        {
            // choose colors where current led is in between
            int currentState = (ledState[i] + j) % DURATION_MILLIS;
            int currentWindow = currentState * COLOR_COUNT / DURATION_MILLIS;
            Rgba colorBefore = colorSet[currentWindow % COLOR_COUNT];
            Rgba colorAfter = colorSet[(currentWindow + 1) % COLOR_COUNT];

            // set color
            int windowLenght = DURATION_MILLIS / COLOR_COUNT;
            float weight = (float)(currentState % windowLenght) / (float)windowLenght;
            m_leds[i] = Rgba(
                (int8_t)colorBefore.red() - (int8_t)(((float)colorBefore.red() - (float)colorAfter.red()) * weight),
                (int8_t)colorBefore.green() - (int8_t)(((float)colorBefore.green() - (float)colorAfter.green()) * weight),
                (int8_t)colorBefore.blue() - (int8_t)(((float)colorBefore.blue() - (float)colorAfter.blue()) * weight),
                1.0);
        }
        m_updateCallback();

        // sleep or wait until waked up
        if (sleepInterruptable(SLEEP_MILLIS))
            return true;
    }
    return false;
}


//////////////////////////////////////////////////
// BlinkAnimation
bool BlinkAnimation::runnable()
{
    //return;
    // constants
    const int DURATION_MILLIS = 5000;
    const int REPETITIONS = 3600000 / DURATION_MILLIS; // 1 hour
    const int SLEEP_MILLIS = 100;
    const int COLOR_COUNT = 10;

    // initialize colors and states of each led
    const Rgba colorSet[COLOR_COUNT] = {
        (Rgba(Rgba::Color::BLUE)),
        (Rgba(10,10,10,1.0)),
        (Rgba(Rgba::Color::RED)),
        (Rgba(10,10,10,1.0)),
        (Rgba(Rgba::Color::GREEN)),
        (Rgba(10,10,10,1.0)),
        (Rgba(Rgba::Color::YELLOW)),
        (Rgba(10,10,10,1.0)),
        (Rgba(Rgba::Color::ORANGE)),
        (Rgba(10,10,10,1.0)) };

    // run animation
    for (int j = 0, r = 0; j < DURATION_MILLIS || (j = 0) || r++ < REPETITIONS; j += SLEEP_MILLIS)
    {
        // choose colors where current led is in between
        int currentState = j % DURATION_MILLIS;
        int currentWindow = currentState * COLOR_COUNT / DURATION_MILLIS;
        Rgba colorBefore = colorSet[currentWindow % COLOR_COUNT];
        Rgba colorAfter = colorSet[(currentWindow + 1) % COLOR_COUNT];

        // set color
        int windowLenght = DURATION_MILLIS / COLOR_COUNT;
        float weight = (float)(currentState % windowLenght) / (float)windowLenght;
        for (int i = 0; i < m_size; i++)
        {
            m_leds[i] = Rgba(
                (int8_t)colorBefore.red() - (int8_t)(((float)colorBefore.red() - (float)colorAfter.red()) * weight),
                (int8_t)colorBefore.green() - (int8_t)(((float)colorBefore.green() - (float)colorAfter.green()) * weight),
                (int8_t)colorBefore.blue() - (int8_t)(((float)colorBefore.blue() - (float)colorAfter.blue()) * weight),
                1.0);
        }
        m_updateCallback();

        // sleep or wait until waked up
        if (sleepInterruptable(SLEEP_MILLIS))
            return true;
    }
    return false;
}