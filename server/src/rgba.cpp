#include "rgba.h"


// i.e. b000114 -> rgba(11,0,1,0.2)
Rgba::Rgba(const string rgba)
{
    unsigned int num = std::stoul("0x" + rgba, nullptr, 16);
    m_red = num >> 24 & 0xff;
    m_green = num >> 16 & 0xff;
    m_blue = num >> 8 & 0xff;
    m_alpha = static_cast<float>(num & 0xff) / 100.0;
}

Rgba & Rgba::operator=(const Rgba & rgba)
{
    this->m_red = rgba.m_red;
    this->m_green = rgba.m_green;
    this->m_blue = rgba.m_blue;
    this->m_alpha = rgba.m_alpha;
    return *this;
}

bool Rgba::operator==(const Color &color)
{
    return toRawHex() == ColorDefs[(int)color];
}

bool Rgba::operator!=(const Color & color)
{
    return !operator==(color);
}

bool Rgba::operator==(const Rgba &color)
{
    return m_red == color.m_red &&
        m_green == color.m_green &&
        m_blue == color.m_blue &&
        m_alpha == color.m_alpha;
}

bool Rgba::operator!=(const Rgba & color)
{
    return !operator==(color);
}

string Rgba::toRawHex() const
{
    unsigned int num = static_cast<unsigned int>(m_alpha * 100) | (m_blue << 8) | (m_green << 16) | (m_red << 24);
    std::stringstream sstream;
    sstream << std::hex << num;
    return sstream.str();
}

std::ostream & operator<<(std::ostream & out, Rgba & obj)
{
    out << "rgba(" << (int)obj.m_red << ", " << (int)obj.m_green << ", " << (int)obj.m_blue << ", " << obj.m_alpha << ")" << endl;
    return out;
}
