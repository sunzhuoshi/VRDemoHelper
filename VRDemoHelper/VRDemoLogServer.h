#pragma once
#include <log4cplus\thread\threads.h>

namespace log4cplus {
	namespace helpers {
		class ServerSocket;
	}
}

class VRDemoLogServer : public log4cplus::thread::AbstractThread
{
public:
	typedef log4cplus::helpers::SharedObjectPtr<VRDemoLogServer> VRDemoLogServerPtr;
	VRDemoLogServer();
	~VRDemoLogServer();
	bool start(unsigned short port);
	void run();
	void stop();
private:
	int m_port;
	bool m_runFlag;
	log4cplus::helpers::ServerSocket *m_serverSocket;
};

