#include "controller_led.h"
#include "fout.h"

#include <SimpleJSON/json.hpp>
#include <unistd.h>


ControllerLed::ControllerLed(int rows, int columns) :
    m_size{rows * columns}, m_rows{rows}, m_columns{columns}
{
    m_leds = new Rgba[m_size];
}


void ControllerLed::resetAll()
{
    for (auto i = 0; i < m_size; i++)
    {
        m_leds[i] = Rgba(Rgba::Color::OFF);
    }
    if (m_deviceLed) {
        m_deviceLed->setLeds(m_leds);
    }
}


void ControllerLed::setCommandAll(Commands command)
{
    switch (command)
    {
    case Commands::FLASH:
        break;
    case Commands::FADE:
        break;
    }
}


bool ControllerLed::setColorAll(Rgba::Color color)
{
    for (auto i = 0; i < m_size; i++)
    {
        m_leds[i] = color;
    }
    return sendBroadcastAll(true);
}


void ControllerLed::attachDeviceLed(DeviceLed *device)
{
    m_deviceLed = device;
    m_deviceLed->setLeds(m_leds);
}


void ControllerLed::addAnimation(string name, Animation *animation)
{
    m_animations[name] = animation;
}


bool ControllerLed::runAnimation(string name)
{
    // find animation and run it
    auto it = m_animations.find(name);
    if (it != m_animations.end())
    {
        Animation *animation = it->second;
        animation->runAnimation(m_leds, m_rows, m_columns, [this]() { sendBroadcastAll(false); });
        return true;
    }
    return false;
}


// receive broadcast from broadcast module
void ControllerLed::receiveBroadcast(string message)
{
    Fout{} << "ControllerLed::onReceiveBroadcast: " << message << endl;
    
    // analyze message
    json::JSON jsonRead = json::JSON::Load(message);

    // controls one or multiple leds
    // containing arbitrary number of pairs: index, color
    // i.e: {"leds":[19,"170c164",25,"110f464"]}
    if (jsonRead.hasKey("leds")) {
        json::JSON leds = jsonRead["leds"];

        // animation is running, forward led as command
        Animation* animation = nullptr;
        for (auto a : m_animations)
            if (a.second->isRunning())
                animation = a.second;
        if (animation != nullptr &&
            leds.length() &&
            // note: controlCommand returns true if command accepted
            //animation->controlCommand(std::to_string(leds[0].ToInt())) &&
            animation->controlLed(leds[0].ToInt(), Rgba(leds[1].ToString()))
            )
            ;
        else
        {
            // change leds
            for (int i = 0; i + 1 < leds.length(); i += 2) {
                int index = leds[0].ToInt();
                string colorString = leds[1].ToString();
                m_leds[index] = Rgba(colorString);
            }
            sendBroadcastAll(true);
        }
    }
    else if (jsonRead.hasKey("cmd")) {
        json::JSON leds = jsonRead["cmd"];
        string value = leds.ToString();

        // all led's off
        if (value == "off")
            setColorAll(Rgba::Color::OFF);
        // run new animation
        else if (runAnimation(value))
            ;
        // forward command to animation
        else
            for (auto a : m_animations)
                if (a.second->isRunning())
                    a.second->controlCommand(value);
    }
    else
        Fout{} << "ControllerLed: Error receiveBroadcast() not found: " << message << endl;
}


void ControllerLed::receiveClientAdded()
{
    sendBroadcastAll(false);
}


bool ControllerLed::sendBroadcastAll(bool stopAnimation)
{
    // stop animation if running
    (stopAnimation) && Animation::stopAnimation();

    // local, device led
    if (m_deviceLed) {
        m_deviceLed->setLeds(m_leds);
    }

    // network, create JSON-object and send broadcast
    json::JSON obj;
    json::JSON myArray = json::Array();
    for (auto i = 0; i < m_size; i++)
        myArray.append(i, m_leds[i].toRawHex());
    obj["leds"] = myArray;
    return BroadcastModule::sendBroadcast(obj.dump());
}
