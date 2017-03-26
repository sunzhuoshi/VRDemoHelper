#include "stdafx.h"
#include "SteamVRDemoLogServer.h"

#include <list>
#include <iostream>
#include <log4cplus\log4cplus.h>

const char *PROPERTY_FILE = "hook.props";

namespace loggingserver
{
	typedef std::list<log4cplus::thread::AbstractThreadPtr> ThreadQueueType;

	class ReaperThread
		: public log4cplus::thread::AbstractThread
	{
	public:
		ReaperThread(log4cplus::thread::Mutex & mtx_,
			log4cplus::thread::ManualResetEvent & ev_,
			ThreadQueueType & queue_)
			: mtx(mtx_)
			, ev(ev_)
			, queue(queue_)
			, stop(false)
		{ }

		virtual
			~ReaperThread()
		{ }

		virtual void run();

		void signal_exit();

	private:
		log4cplus::thread::Mutex & mtx;
		log4cplus::thread::ManualResetEvent & ev;
		ThreadQueueType & queue;
		bool stop;
	};

	typedef log4cplus::helpers::SharedObjectPtr<ReaperThread> ReaperThreadPtr;

	void ReaperThread::signal_exit()
	{
		log4cplus::thread::MutexGuard guard(mtx);
		stop = true;
		ev.signal();
	}

	void ReaperThread::run()
	{
		ThreadQueueType q;

		while (true)
		{
			ev.timed_wait(30 * 1000);
			{
				log4cplus::thread::MutexGuard guard(mtx);

				// Check exit condition as the very first thing.
				if (stop)
				{
					std::cout << "Reaper thread is stopping..." << std::endl;
					return;
				}
				ev.reset();
				q.swap(queue);
			}
			if (!q.empty())
			{
				std::cout << "Reaper thread is reaping " << q.size() << " threads."
					<< std::endl;

				for (ThreadQueueType::iterator it = q.begin(), end_it = q.end();
					it != end_it; ++it)
				{
					AbstractThread & t = **it;
					t.join();
				}
				q.clear();
			}
		}
	}

	/**
	This class wraps ReaperThread thread and its queue.
	*/
	class Reaper
	{
	public:
		Reaper()
		{
			reaper_thread = ReaperThreadPtr(new ReaperThread(mtx, ev, queue));
			reaper_thread->start();
		}

		~Reaper()
		{
			reaper_thread->signal_exit();
			reaper_thread->join();
		}

		void visit(log4cplus::thread::AbstractThreadPtr const & thread_ptr);

	private:
		log4cplus::thread::Mutex mtx;
		log4cplus::thread::ManualResetEvent ev;
		ThreadQueueType queue;
		ReaperThreadPtr reaper_thread;
	};

	void Reaper::visit(log4cplus::thread::AbstractThreadPtr const & thread_ptr)
	{
		log4cplus::thread::MutexGuard guard(mtx);
		queue.push_back(thread_ptr);
		ev.signal();
	}

	class ClientThread
		: public log4cplus::thread::AbstractThread
	{
	public:
		ClientThread(log4cplus::helpers::Socket clientsock_, Reaper & reaper_)
			: self_reference(log4cplus::thread::AbstractThreadPtr(this))
			, clientsock(std::move(clientsock_))
			, reaper(reaper_)
		{
			std::cout << "Received a client connection!!!!" << std::endl;
		}

		~ClientThread()
		{
			std::cout << "Client connection closed." << std::endl;
		}

		virtual void run();

	private:
		log4cplus::thread::AbstractThreadPtr self_reference;
		log4cplus::helpers::Socket clientsock;
		Reaper & reaper;
	};


	void loggingserver::ClientThread::run()
	{
		try
		{
			while (1)
			{
				if (!clientsock.isOpen())
					break;

				log4cplus::helpers::SocketBuffer msgSizeBuffer(sizeof(unsigned int));
				if (!clientsock.read(msgSizeBuffer))
					break;

				unsigned int msgSize = msgSizeBuffer.readInt();

				log4cplus::helpers::SocketBuffer buffer(msgSize);
				if (!clientsock.read(buffer))
					break;

				log4cplus::spi::InternalLoggingEvent event
					= log4cplus::helpers::readFromBuffer(buffer);
				log4cplus::Logger logger
					= log4cplus::Logger::getInstance(event.getLoggerName());
				logger.callAppenders(event);
			}
		}
		catch (...)
		{
			reaper.visit(std::move(self_reference));
			throw;
		}
		reaper.visit(std::move(self_reference));
	}

} // namespace loggingserver


SteamVRDemoLogServer::SteamVRDemoLogServer() :
	m_serverSocket(nullptr),
	m_runFlag(true)
{
}


SteamVRDemoLogServer::~SteamVRDemoLogServer()
{
	delete m_serverSocket;
}

bool SteamVRDemoLogServer::start(unsigned short port)
{
	log4cplus::PropertyConfigurator config(PROPERTY_FILE);
	config.configure();
	log4cplus::Logger logger = log4cplus::Logger::getRoot();

	m_serverSocket = new log4cplus::helpers::ServerSocket(port, false, false, "127.0.0.1");
	if (!m_serverSocket->isOpen()) {
		LOG4CPLUS_ERROR(logger, "Could not open server socket, maybe port " << port << " is already in use." << std::endl);
		return false;
	}
	LOG4CPLUS_INFO(logger, "Log server is listening on port: " << port << std::endl);

	log4cplus::thread::AbstractThread::start();
	return true;
}

void SteamVRDemoLogServer::run()
{
	loggingserver::Reaper reaper;

	while (m_runFlag)
	{
		loggingserver::ClientThread *thr =
			new loggingserver::ClientThread(m_serverSocket->accept(), reaper);
		thr->start();
	}
}

void SteamVRDemoLogServer::stop()
{
	m_runFlag = false;
	
	m_serverSocket->interruptAccept();
	this->join();
}
