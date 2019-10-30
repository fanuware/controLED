#include "fout.h"

mutex Fout::s_mtxCout{};
Fout::eMode Fout::s_processMode = Fout::INTERACTIVE;
function<string(void)> Fout::s_prefixFunc;
function<string(void)> Fout::s_postfixFunc;

Fout::Fout(eMode printMode)
    : m_printMode{ printMode } {}

Fout::~Fout() 
{
    lock_guard<mutex> guard(s_mtxCout);

    // print as is, without pre/suffix
    if (m_printMode == Fout::INTERACTIVE)
    {
        if (s_processMode != Fout::LOG)
            cout << this->str();
    }
    // print log message
    else if (s_processMode <= m_printMode)
    {
        // remove last endl
        string s = this->str();
        s.erase(std::remove(s.end() - 1, s.end(), '\n'), s.end());

        // print: prefix - content - suffix
        (s_prefixFunc) && cout << s_prefixFunc();
        cout << s;
        (s_postfixFunc) && cout << s_postfixFunc();
        cout << endl;
    }
}

void Fout::setDefaults(function<string(void)> prefix, function<string(void)> postfixFunc, eMode processMode)
{
    s_prefixFunc = prefix;
    s_postfixFunc = postfixFunc;
    s_processMode = processMode;
};