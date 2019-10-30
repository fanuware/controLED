#ifndef _FOUT_H
#define _FOUT_H

#include <iostream>
#include <sstream>
#include <string>
#include <string>
#include <mutex>
#include <functional>
#include <algorithm>

using namespace std;


// thread safe standard output class
// use tempoary object: Fout{} << "log message" << val << std::endl;
class Fout : public ostringstream
{
public:
    enum eMode {
        INTERACTIVE, // print as is, without pre/suffix
        LOG_NOD, // NOT DAEMON, print log message when
        LOG }; // ALWAYS, print log message

    Fout(eMode currentMode = INTERACTIVE);
    ~Fout();
    static void setDefaults(function<string(void)> prefix, function<string(void)> postfixFunc, eMode mode);

    static string currentDateTime() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
        return buf;
    }

private:
    static mutex s_mtxCout;
    static eMode s_processMode;
    static function<string(void)> s_prefixFunc;
    static function<string(void)> s_postfixFunc;
    eMode m_printMode;
};

#endif
