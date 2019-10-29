#include "webserver.h"
#include "fout.h"

Webserver::Webserver(unsigned short port)
{
	m_server.config.port = port;
	auto &echo_all = m_server.endpoint["^/ws/?$"];
	echo_all.on_message = [this](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {
		string messageString = message->string();
		auto send_stream = make_shared<WsServer::SendStream>();
		*send_stream << messageString;

		// track timeout
		if (messageString == "pong")
		{
			if (m_timeoutTracker.find(connection) != m_timeoutTracker.end())
			{
				chrono::steady_clock::time_point begin = m_timeoutTracker[connection];
				chrono::steady_clock::time_point end = chrono::steady_clock::now();
				m_showPingTimeout && Fout{ Fout::LOG_NOD } << "Elapsed time: " << (float)chrono::duration_cast<chrono::microseconds>(end - begin).count() / 1000.0 << "ms / " << connection << endl;
				m_timeoutTracker[connection] = end;
			}
		} else
		{
			// inform all broadcast modules
			Fout{ Fout::LOG_NOD } << messageString << endl;
			for (BroadcastModule *module : m_broadcastModules)
			{
				module->receiveBroadcast(messageString);
			}
		}
	};
	echo_all.on_open = [this](shared_ptr<WsServer::Connection> connection) {
		Fout{ Fout::LOG } << "Webserver: Opened connection " << connection.get() << endl;
		for (BroadcastModule *module : m_broadcastModules)
		{
			module->receiveClientAdded();
		}
	};
	echo_all.on_close = [](shared_ptr<WsServer::Connection> connection, int status, const string & /*reason*/) {
		Fout{ Fout::LOG } << "Webserver: Closed connection " << connection.get() << endl;
	};
	echo_all.on_error = [](shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
		Fout{ Fout::LOG } << "Webserver: Error in connection " << connection.get() << ". "
			<< "Error: " << ec << ", error message: " << ec.message() << endl;
	};
}


void Webserver::addBroadcastModule(BroadcastModule *module)
{
	module->attachBroadcastSender([this](string message) { sendBroadcast(message); });
	m_broadcastModules.push_back(module);
}


Webserver::~Webserver()
{
	delete m_serverThread;
}


void Webserver::sendBroadcast(string message)
{
	m_lastBroadcast = chrono::steady_clock::now();
	for (auto &a_connection : m_server.get_connections())
	{
		auto send_stream = make_shared<WsServer::SendStream>();
		*send_stream << message;
		a_connection->send(send_stream);
	}
}


void Webserver::startServer()
{
	m_serverThread = new thread ([this]() {
		m_server.start();
	});
	Fout{ Fout::LOG } << "Webserver: Started on port "<< m_server.config.port << endl;

	// ping-pong to make sure all connections are alive
	m_threadPingPong = new thread([this]() {
		const int TIMEOUT_MILLIS = 4000;
		m_timeoutTracker.clear();
		while (true)
		{
			// send ping broadcast
			sendPingBroadcast();
			this_thread::sleep_for(chrono::milliseconds(TIMEOUT_MILLIS));

			// make sure all connections are alive
			chrono::steady_clock::time_point now = chrono::steady_clock::now();
			for (auto it = m_timeoutTracker.begin(); it != m_timeoutTracker.end(); /*_*/)
			{
				if ((now - it->second) > chrono::milliseconds(2 * TIMEOUT_MILLIS))
				{
					it->first->send_close(1000);
					it = m_timeoutTracker.erase(it);
					Fout{ Fout::LOG_NOD } << "Webserver: Connection not responding " << it->first.get() << endl;
				}
				else
				{
					it++;
				}
			}
		}
	});
}


void Webserver::stopServer()
{
	m_serverThread->join();
}


void Webserver::sendPingBroadcast()
{
	for (auto &a_connection : m_server.get_connections())
	{
		m_timeoutTracker[a_connection] = chrono::steady_clock::now();
		auto send_stream = make_shared<WsServer::SendStream>();
		*send_stream << "ping";
		a_connection->send(send_stream);
	}
}
