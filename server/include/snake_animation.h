#ifndef SNAKE_ANIMATION_H
#define SNAKE_ANIMATION_H

#include "animation.h"

#include <deque>
#include <vector>


class SnakeAnimation: public Animation
{
public:
    enum class eDirection {
        LEFT,
        RIGHT,
        UP,
        DOWN,
        REF // no direction
    };
    enum class eVelocityMode {
        FIXED,
        DYNAMIC
    };
    enum class eState {
        Inactive,
        Active,
        Target
    };
    enum class eGameState {
        INITIALIZE,
        RUNNING,
        PAUSE
    };


    // one square of the matrix
    // has the ability to control the led directly
    class Square
    {
    public:
        Square(int positionRow, int positionColumn, Rgba *const led) :
            m_positionRow(positionRow),
            m_positionColumn(positionColumn),
            m_led(led)
        {
            m_state = eState::Inactive;
            setState(m_state);
        }
        void setState(eState s);
        int getRow() { return m_positionRow; };
        int getCol() { return m_positionColumn; };
        static Rgba s_inactiveColor;
        static Rgba s_activeColor;
        static Rgba s_targetColor;
    private:
        int m_positionRow;
        int m_positionColumn;
        eState m_state;
        Rgba *const m_led;
    };

    SnakeAnimation();
    void setPlayField();
    void cycleEvent();
    void gameStart();
    void gamePause();
    void gameRestart();
    void setDirection(eDirection dir);
    void setVelocity(int velocityFixedPercent);
    void setVelocityMode(eVelocityMode velocityMode);

    // override: animation
    bool controlCommand(string cmd);
    bool controlLed(int led, Rgba color);

private:
    void setSquare(Square *ref, eDirection dir);
    void removeSquare();
    void checkTarget();
    void setFood();
    void getDirection();
    void setSnakeVelocity();
    int getVelocity();

    // fields
    eVelocityMode m_velocityMode;
    int m_velocityFixedPercent;
    int m_timeCycleEvent, m_minTimeCycleEvent, m_maxTimeCycleEvent;
    eGameState m_gameStateCmd;
    int m_snakeSize;
    eDirection m_actualDirection;
    eDirection m_futureDirection;
    vector<vector<Square*>> m_grid;
    deque<Square*> m_snake;
    Square* m_target;

    // override: animation
    bool runnable();
};

#endif