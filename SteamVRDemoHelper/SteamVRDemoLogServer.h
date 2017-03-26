#pragma once
class SteamVRDemoLogServer
{
public:
	SteamVRDemoLogServer();
	~SteamVRDemoLogServer();
	bool start(unsigned short port);
	void stop();
private:
	int m_port;
};

