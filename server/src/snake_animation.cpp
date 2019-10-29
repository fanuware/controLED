#include "snake_animation.h"
#include "fout.h"

#include <cstdlib>
#include <ctime>


//////////////////////////////////////////////////
// Square
Rgba SnakeAnimation::Square::s_inactiveColor = Rgba::Color::OFF;
Rgba SnakeAnimation::Square::s_activeColor = Rgba::Color::BLUE;
Rgba SnakeAnimation::Square::s_targetColor = Rgba::Color::RED;


//////////////////////////////////////////////////
// SnakeAnimation

bool SnakeAnimation::controlCommand(string cmd)
{
	Fout{} << "SnakeAnimation cmd: " << cmd << endl;
	if (cmd == "up")
		this->setDirection(eDirection::UP);
	else if (cmd == "down")
		this->setDirection(eDirection::DOWN);
	else if (cmd == "left")
		this->setDirection(eDirection::LEFT);
	else if (cmd == "right")
		this->setDirection(eDirection::RIGHT);
	else
		return false;
	return true;
}

bool SnakeAnimation::controlLed(int led, Rgba color)
{
	switch (led) {
	case 1: case 2: case 3: case 4: case 5: case 6: case 10: case 11: case 12: case 13: case 19: case 20:
		this->setDirection(eDirection::UP);
		break;
	case 43: case 44: case 50: case 51: case 52: case 53: case 57: case 58: case 59: case 60: case 61: case 62:
		this->setDirection(eDirection::DOWN);
		break;
	case 8: case 16: case 17: case 24: case 25: case 26: case 32: case 33: case 34: case 40: case 41: case 48:
		this->setDirection(eDirection::LEFT);
		break;
	case 15: case 22: case 23: case 29: case 30: case 31: case 37: case 38: case 39: case 46: case 47: case 55:
		this->setDirection(eDirection::RIGHT);
		break;
	default:
		return false;
	}
	return true;
}


bool SnakeAnimation::runnable()
{

	// velocity defaults
	m_minTimeCycleEvent = 50;
	m_maxTimeCycleEvent = 800;
	m_velocityFixedPercent = 65;
	m_velocityMode = eVelocityMode::FIXED;

	// initialize game
	m_gameStateCmd = eGameState::INITIALIZE;
	while (m_gameStateCmd != eGameState::PAUSE) {
		cycleEvent();
		m_updateCallback();
		if (sleepInterruptable(m_timeCycleEvent))
			return true;
	}
	return false;
}


SnakeAnimation::SnakeAnimation()
{
	std::srand(std::time(nullptr));
}

void SnakeAnimation::setPlayField()
{
	// create all square
	if (m_grid.size() == 0)
	{
		for (int row = 0; row < m_sizeRows; row++) {
			vector<Square*> colVec;
			for (int col = 0; col < m_sizeColumns; col++) {
				colVec.push_back(new Square(row, col, &m_leds[(row * m_sizeRows) + col]));
			}
			m_grid.push_back(colVec);
		}
	} else {
		for (auto r : m_grid) {
			for (auto s : r) {
				s->setState(eState::Inactive);
			}
		}
	}

	// choose random snake color
	switch (std::rand() % 5) {
	case 0:
		Square::s_activeColor = Rgba::Color::BLUE;
		break;
	case 1:
		Square::s_activeColor = Rgba::Color::GREEN;
		break;
	case 2:
		Square::s_activeColor = Rgba::Color::ORANGE;
		break;
	case 3:
		Square::s_activeColor = Rgba::Color::WHITE;
		break;
	case 4:
		Square::s_activeColor = Rgba::Color::PURPLE;
		break;
	}

	// initialize game default
	m_snakeSize = 3;
	m_actualDirection = eDirection::RIGHT;
	m_futureDirection = m_actualDirection;
	m_snake.clear();
	setSquare(m_grid[1][1], eDirection::REF);

	// place food
	setFood();
}


void SnakeAnimation::cycleEvent()
{
	if (m_gameStateCmd == eGameState::INITIALIZE) {
		setPlayField();
		m_gameStateCmd = eGameState::RUNNING;
	}
	else if (m_gameStateCmd == eGameState::RUNNING) {
		// check direction changed
		getDirection();

		// move snake head
		setSquare(m_snake.front(), m_actualDirection);

		// check target
		checkTarget();

		// remove snake back
		removeSquare();
	}
}


void SnakeAnimation::gameStart()
{
	m_gameStateCmd = eGameState::RUNNING;
}


void SnakeAnimation::gamePause()
{
	m_gameStateCmd = eGameState::PAUSE;
}


void SnakeAnimation::gameRestart()
{
	m_gameStateCmd = eGameState::INITIALIZE;
}


void SnakeAnimation::setSquare(Square *ref, eDirection dir) {
	Square *actSquare;
	switch (dir) {
	case eDirection::LEFT:
		actSquare = m_grid[ref->getRow() % m_sizeRows][(ref->getCol() + m_sizeColumns - 1) % m_sizeColumns];
		break;
	case eDirection::RIGHT:
		actSquare = m_grid[ref->getRow() % m_sizeRows][(ref->getCol() + 1) % m_sizeColumns];
		break;
	case eDirection::UP:
		actSquare = m_grid[(ref->getRow() + m_sizeRows - 1) % m_sizeRows][ref->getCol() % m_sizeColumns];
		break;
	case eDirection::DOWN:
		actSquare = m_grid[(ref->getRow() + 1) % m_sizeRows][ref->getCol() % m_sizeColumns];
		break;
	case eDirection::REF:
	default:
		actSquare = ref;
	}

	// collision check
	bool isCollision = false;
	for (Square *sq : m_snake) {
		if (sq == actSquare) {
			isCollision = true;
		}
	}
	if (isCollision) {
		gamePause();
	}
	else {
		actSquare->setState(eState::Active);
		m_snake.push_front(actSquare);
	}
}


void SnakeAnimation::removeSquare() {
	while (m_snake.size() > (size_t)m_snakeSize) {
		m_snake.back()->setState(eState::Inactive);
		m_snake.pop_back();
	}
}


void SnakeAnimation::checkTarget() {
	for (Square *square : m_snake) {

		// target found
		if (square == m_target) {
			m_snakeSize++;
			setFood();
		}
	}
}


void SnakeAnimation::setFood() {
	bool isSearchSquare;
	Square *randomSquare;
	do {
		isSearchSquare = false;
		randomSquare = m_grid[std::rand() % m_sizeRows][std::rand() % m_sizeColumns];
		for (Square *square : m_snake) {
			if (square == randomSquare) {
				isSearchSquare = true;
				continue;
			}
		}
	} while (isSearchSquare);
	m_target = randomSquare;
	m_target->setState(eState::Target);
	setSnakeVelocity();
}


void SnakeAnimation::setDirection(eDirection dir) {
	switch (dir) {
	case eDirection::LEFT:
		if (m_actualDirection != eDirection::RIGHT)
			m_futureDirection = dir;
		break;
	case eDirection::RIGHT:
		if (m_actualDirection != eDirection::LEFT)
			m_futureDirection = dir;
		break;
	case eDirection::UP:
		if (m_actualDirection != eDirection::DOWN)
			m_futureDirection = dir;
		break;
	case eDirection::DOWN:
		if (m_actualDirection != eDirection::UP)
			m_futureDirection = dir;
		break;
	default:
		break;
	}
}


void SnakeAnimation::getDirection() {
	m_actualDirection = m_futureDirection;
}


// set snake velocity
void SnakeAnimation::setSnakeVelocity() {

	// fixed velocity
	if (m_velocityMode == eVelocityMode::FIXED) {
		m_timeCycleEvent = m_maxTimeCycleEvent - ((m_maxTimeCycleEvent - m_minTimeCycleEvent) * m_velocityFixedPercent / 100);
		if (m_timeCycleEvent < m_minTimeCycleEvent)
			m_timeCycleEvent = m_minTimeCycleEvent;
		if (m_timeCycleEvent > m_maxTimeCycleEvent)
			m_timeCycleEvent = m_maxTimeCycleEvent;
	}

	// dynamic velocity
	else if (m_velocityMode == eVelocityMode::DYNAMIC) {
		m_timeCycleEvent = m_minTimeCycleEvent + (std::rand() % (m_maxTimeCycleEvent - m_minTimeCycleEvent));
	}
}


int SnakeAnimation::getVelocity() {
	return m_velocityFixedPercent;
}


// set velocity [%]
void SnakeAnimation::setVelocity(int velocityFixedPercent) {
    m_velocityFixedPercent = velocityFixedPercent;
    setSnakeVelocity();
}


// set velocity mode [%]
void SnakeAnimation::setVelocityMode(eVelocityMode velocityMode) {
    m_velocityMode = velocityMode;
    setSnakeVelocity();
}


void SnakeAnimation::Square::setState(eState s)
{
	switch (s) {
	case eState::Inactive:
		*m_led = s_inactiveColor;
		break;
	case eState::Active:
		*m_led = s_activeColor;
		break;
	case eState::Target:
		*m_led = s_targetColor;
		break;
	}
}
