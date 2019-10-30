#include "labrinth_animation.h"
#include "fout.h"

#include <queue>
#include <vector>

// constants
static const Rgba COLOR_BG(Rgba::Color::OFF);
static const Rgba COLOR_FG(Rgba::Color::BLUE);
static const Rgba COLOR_TARGET(Rgba::Color::RED);
static const Rgba COLOR_VISITED(Rgba::Color::YELLOW);
static const Rgba COLOR_PATH(Rgba::Color::GREEN);

bool LabrinthAnimation::runnable()
{
    const int DIRECTION[4][2]{
        { 0, -1 }, // up
        { 1, 0 }, // right
        { 0, 1 }, // down
        { -1, 0 } // left
    };

    // clear screen
    for (int i = 0; i < m_size; i++)
        m_leds[i] = COLOR_BG;
    m_leds[0] = COLOR_VISITED;
    m_updateCallback();

    // wait for start
    state = eState::INIT_LABRINTH;
    unique_lock<mutex> lock(mxWaitSolve);
    //cvWaitSolve.wait(lock, [&]() { return state != eState::INIT_LABRINTH; });
    while (state == eState::INIT_LABRINTH) {
        if (sleepInterruptable(250))
            return true;
    }

    // solve labrinth (dijkstra)
    queue<int> qu;
    vector<bool> vis(m_size, false);
    vector<int> parent(m_size);
    vis[0] = true;
    parent[0] = -1;
    qu.push(0);
    int targetFound = false;
    while (!qu.empty())
    {
        if (sleepInterruptable(250))
            return true;
        int cur = qu.front();
        qu.pop();
        int col = cur % m_sizeColumns;
        int row = cur / m_sizeColumns;

        // verify target found
        if (cur == target) {
            targetFound = true;
            break;
        }
        m_leds[cur] = COLOR_VISITED;
        m_updateCallback();

        // continue search unvisited and unblocked
        for (int i = 0; i < 4; i++)
        {
            int nextCol = col + DIRECTION[i][0];
            int nextRow = row + DIRECTION[i][1];
            int next = nextRow * m_sizeColumns + nextCol;
            if (0 <= nextCol && nextCol < m_sizeColumns &&
                0 <= nextRow && nextRow < m_sizeRows &&
                !vis[next] &&
                m_leds[next] != COLOR_FG ) {
                vis[next] = true;
                parent[next] = cur;
                qu.push(next);
            }
        }
    }
    if (targetFound) {
        int cur = target;
        while (cur != -1)
        {
            cur = parent[cur];
            m_leds[cur] = COLOR_PATH;
            m_updateCallback();
            if (sleepInterruptable(100))
                return true;
        }
    }
    return false;
}


bool LabrinthAnimation::controlLed(int led, Rgba color)
{
    Fout{} << "Led" << endl;
    if (state == eState::INIT_LABRINTH)
    {
        if (led == 0) {
            state = eState::SOLVE_LABRINTH;
            cvWaitSolve.notify_one();
        }
        else {
            if (m_leds[target] != Rgba::Color::OFF)
                m_leds[target] = COLOR_FG;
            target = led;
            m_leds[target] = COLOR_TARGET;
            m_updateCallback();
        }
    }
    else
        return false;
    return true;
}
