#include "fout.h"
#include "webserver.h"
#include "controller_led.h"

#ifdef RASPBERRY_PI
#include "device_led_ws2812.h"
#include "device_led_74hc595.h"
#endif

#include "animation.h"
#include "snake_animation.h"
#include "labrinth_animation.h"
#include "rgba.h"

#include <string>
#include <future>
#include <cstdlib>
#include <signal.h>
#include <chrono>
#include <thread>

#define LED_ROWS 8
#define LED_COLUMNS 8

using namespace std;

// default settings, see command line arguments parser
bool isDaemonProcess = false;
unsigned short port = 8080;
std::string ledDevice = "PWM";

// signal handler, CTRL-C,..
auto terminationPromise = promise<bool>();

void signalHandler(int s)
{
    if (isDaemonProcess)
        terminationPromise.set_value(true);
    else
        exit(1);
}


// main
int main(int argc, char *argv[]) {

    // command line arguments
    int c = 0;
    while ((c = getopt(argc, argv, "dp:l:h")) != -1)
        switch (c)
        {
        case 'd':
            isDaemonProcess = true;
            break;
        case 'p':
            port = (unsigned short)stoi(optarg);
            break;
        case 'l':
            ledDevice = std::string(optarg);
            break;
        case 'h':
            Fout{} <<
                "Usage: controLed [OPTION]" <<
                "\nDescription : Control LED Matrix on Raspberry Pi," <<
                "\nWebserver allows multiple connections at the same time" <<
                "\n\nControl :" <<
                "\n-d : daemon, without standard input" <<
                "\n-p : port number" <<

                "\n-l : local LED device" <<
                "\n\t <PWM> = PWM-driver for WS2812: (default)" <<
                "\n\t\t$ sudo vi / etc / modprobe.d / snd - blacklist.conf" <<
                "\n\t\tblacklist snd_bcm2835" <<
                "\n\t\t$ sudo vi /boot/ config.txt" <<
                "\n\t\t# Enable audio(loads snd_bcm2835)" <<
                "\n\t\t#dtparam=audio=on" <<

                "\n\t <SPI> = spi-driver for 74HC595:" <<
                "\n\t\t$ sudo vi /boot/config.txt" <<
                "\n\t\tdevice_tree=bcm2710-rpi-3-b.dtb" <<
                "\n\t\tdtparam=spi=on" <<

                "\n\t <*> = no local device:" <<

                "\n-h : displays help" << endl;
            exit(0);
        }

    // set defaults for output stream wrapper (thread safe)
    if (isDaemonProcess)
    {
        Fout::setDefaults(
            []() { return ""; }, // prefix not needed => sysdemD
            []() { return " (" + Fout::currentDateTime() + ")"; }, // postfix
            Fout::LOG // running mode
        );
    }
    else
    {
        Fout::setDefaults(
            []() { return "ControLed - "; }, // prefix
            []() { return " (" + Fout::currentDateTime() + ")"; }, // postfix
            Fout::LOG_NOD // running mode
        );
    }

    // control all LED's (local and external)
    ControllerLed controller(LED_ROWS, LED_COLUMNS);
    for (auto & c : ledDevice) c = toupper(c);

#ifdef RASPBERRY_PI
    if (ledDevice == "PWM")
    {
        controller.attachDeviceLed((DeviceLed*) new DeviceLedWS2812(LED_ROWS * LED_COLUMNS));
        Fout{} << "Local Led device: " << ledDevice << endl;
    }
    else if (ledDevice == "SPI")
    {
        controller.attachDeviceLed((DeviceLed*) new DeviceLed74HC595(LED_ROWS * LED_COLUMNS));
        Fout{} << "Local Led device: " << ledDevice << endl;

    }
#endif

    controller.addAnimation("fade", new FadeAnimation());
    controller.addAnimation("blink", new BlinkAnimation());
    controller.addAnimation("snake", new SnakeAnimation());
    controller.addAnimation("labr", new LabrinthAnimation());

    // start webserver
    Webserver webserver(port);
    webserver.addBroadcastModule(&controller);
    webserver.startServer();

    // register signal handler
    signal(SIGINT, signalHandler);

    // distunguish between background and foreground process
    if (isDaemonProcess) {
        // wait for termination, otherwhile wait/run endless
        terminationPromise.get_future().wait();
    } else
    {
        while (true)
        {
            Fout{} << "Command $ ";
            int command;

            // read entered numbers
            string userInput;
            std::getline(std::cin, userInput);
            std::stringstream stream(userInput);
            if (stream >> command)
            {
                switch (command)
                {
                case 0:
                    controller.setColorAll(Rgba::Color::OFF);
                    break;
                case 1:
                    controller.runAnimation("fade");
                    break;
                case 2:
                    controller.runAnimation("blink");
                    break;
                case 3:
                    controller.runAnimation("snake");
                    break;
                case 4:
                    controller.setColorAll(Rgba::Color::BLUE);
                    break;
                case 5:
                    controller.setColorAll(Rgba::Color::GRAY);
                    break;
                case 6:
                    controller.setColorAll(Rgba::Color::PURPLE);
                    break;
                case 7:
                    controller.setColorAll(Rgba::Color::ORANGE);
                    break;
                case 8:
                    controller.setColorAll(Rgba::Color::YELLOW);
                    break;
                case 9:
                    controller.setColorAll(Rgba::Color::TURQUOISE);
                    break;
                case 10:
                    controller.setColorAll(Rgba::Color::WHITE);
                    break;
                case 11:
                    controller.setColorAll(Rgba::Color::GREEN);
                    break;
                case 44:
                    webserver.m_showPingTimeout = !webserver.m_showPingTimeout;
                    break;
                default:
                    controller.setColorAll(Rgba::Color::OFF);
                    break;
                }
            }
            else
                Fout{} << "Command unknown" << endl;
        }
    }

    // turn led's off (note: only when daemon)
    controller.resetAll();
}
