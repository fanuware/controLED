#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <vector>
#include <functional>
#include <map>
#include "Simple-WebSocket-Server/client_ws.hpp"
#include "Simple-WebSocket-Server/server_ws.hpp"

using namespace std;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;


class BroadcastModule
{
public:
    virtual void receiveBroadcast(string message) = 0;
    virtual void receiveClientAdded() = 0;
    bool sendBroadcast(string message) { m_broadcastSenderFunc(message); return true; };
    void attachBroadcastSender(function<void(string)> sender) { m_broadcastSenderFunc = sender; };
private:
    function<void(string)> m_broadcastSenderFunc;
};

class Webserver
{
public:
    Webserver(unsigned short port);
    void addBroadcastModule(BroadcastModule *listener);
    void startServer();
    void stopServer();
    ~Webserver();
    bool m_showPingTimeout = false;
private:
    WsServer m_server;
    thread *m_serverThread;
    vector<BroadcastModule*> m_broadcastModules;
    void sendBroadcast(string message);
    map<shared_ptr<WsServer::Connection>, chrono::steady_clock::time_point> m_timeoutTracker;
    chrono::steady_clock::time_point m_lastBroadcast;

    // ping-pong to make sure connections stay alive
    thread *m_threadPingPong;
    void sendPingBroadcast();
};

#endif