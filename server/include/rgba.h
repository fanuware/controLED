#ifndef RGBA_H
#define RGBA_H

#include <string>
#include <ostream>
#include <sstream>
#include <iostream>

using namespace std;

const string ColorDefs[] = {
	"170c164", "f5ff0064", "ff000064",
	"af5064", "ffa70064", "b500ff64",
	"959f9b64", "ffffff64", "e2ff64",
	"0"
};


class Rgba
{
public:
	enum class Color {
		BLUE, YELLOW, RED,
		GREEN, ORANGE, PURPLE,
		GRAY, WHITE, TURQUOISE,
		OFF };

	Rgba(const uint8_t red = 255, const uint8_t green = 255, const uint8_t blue = 255, const float alpha = 0.0)
		: m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) {}
	Rgba(const string rgba);
	Rgba(const Color &color)
		: Rgba(ColorDefs[(int)color]) {}
	Rgba& operator=(const Rgba &rgba);
	bool operator==(const Color &color);
	bool operator!=(const Color &color);
	bool operator==(const Rgba &color);
	bool operator!=(const Rgba &color);

	friend std::ostream& operator<<(std::ostream& out, Rgba& obj);

	string toRawHex() const;
	uint8_t red() const { return m_red; }
	uint8_t green() const { return m_green; }
	uint8_t blue() const { return m_blue; }
	float alpha() const { return m_alpha; }
private:
	uint8_t m_red, m_green, m_blue;
	float m_alpha;
};

#endif
