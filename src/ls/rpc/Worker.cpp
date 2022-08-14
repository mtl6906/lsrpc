#include "ls/rpc/Worker.h"

#include "ls/epoll/Tool.h"
#include "ls/rpc/main.h"
#include "ls/io/API.h"
#include "ls/Exception.h"
#include "ls/DefaultLogger.h"
#include "unistd.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Worker::Worker(int connectionNumber, int buffersize) : cm(connectionNumber, buffersize), et(connectionNumber)
		{

		}
		
		void Worker::run(ProtocolManager *pm, QueueManager *qm, int threadNumber)
		{
			Protocol *protocol = pm -> get(threadNumber);
			net::Server &server = protocol -> getServer();
			et.add(protocol -> fd(), EPOLLIN);
			for(;;)
			{
			/*
				if(qm -> iswait(1) == false)
					for(;;)
					{
						if(cm.empty() == true)
						{
							break;
						}
						Connection *connection = nullptr;
						try
						{
							connection = qm -> get(threadNumber);
							LOGGER(ls::INFO) << threadNumber << ": get ok" << ls::endl;
							cm.assign(connection);
							et.add(connection -> fd(), EPOLLIN | EPOLLET);
							LOGGER(ls::INFO) << threadNumber << ": add new event on " << (connection -> fd()) << ls::endl;
						}
						catch(Exception &e)
						{
							break;
						}
					}
			*/
				int n = et.wait(-1);
				if(n > 0)
					LOGGER(ls::INFO) << threadNumber << ": "<< n << " event trigger" << ls::endl;
				
				for(int i=0;i<n;++i)
				{
					auto &event = et.event(i);
					LOGGER(ls::INFO) << "event on " << event.data.fd << ls::endl;
					if(event.data.fd == protocol -> fd())
					{
						if(cm.empty())
							continue;	
						int connfd = server.accept();
						cm.assign(connfd, protocol -> getTag());
						et.add(connfd, EPOLLIN | EPOLLET | EPOLLRDHUP);
						continue;
					}
					auto connection = cm.get(event.data.fd);
					if(connection == nullptr)
						continue;
					if(event.events & EPOLLRDHUP)
					{
						pm -> release(connection);
						cm.recycle(connection);
						continue;
					}
					else if(event.events & EPOLLIN)
					{
						LOGGER(ls::INFO) << threadNumber << ": epollin trigger" << ls::endl;
						int ec = pm -> readContext(connection);
						if(ec < 0)
						{
							if(ec == Exception::LS_ENOCOMPLETE)
							{
								LOGGER(ls::INFO) << threadNumber << ": read no complete" << ls::endl;
								continue;
							}
							pm -> release(connection);
							if(connection -> keepalive == false)
								cm.recycle(connection);
							else
								cm.clear(connection);
							LOGGER(ls::INFO) << "read failed" << ls::endl;
							continue;
						}
						LOGGER(ls::INFO) << threadNumber << ": read ok" << ls::endl;
						ec = pm -> exec(connection);
						LOGGER(ls::INFO) << threadNumber << ": exec ok" << ls::endl;
						
						ec = send(connection, pm);
						LOGGER(ls::INFO) << threadNumber << ": send ok" << ls::endl;
						if(ec < 0)
						{
							if(ec == Exception::LS_EWOULDBLOCK)
							{
								LOGGER(ls::INFO) << threadNumber << ": write no complete" << ls::endl;
								et.mod(event.data.fd, EPOLLOUT | EPOLLET);
								continue;
							}
							LOGGER(ls::INFO) << threadNumber << ": send failed" << ls::endl;
						}
					}
					else if(event.events & EPOLLOUT)
					{
						LOGGER(ls::INFO) << threadNumber <<  ": epollout trigger" << ls::endl;
						
						int ec = send(connection, pm);			
						LOGGER(ls::INFO) << threadNumber <<  ": send ok" << ls::endl;
						if(ec < 0)
						{
							if(ec == Exception::LS_EWOULDBLOCK)
							{
								LOGGER(ls::INFO) << threadNumber <<  ": write no complete" << ls::endl;
								continue;
							}
							LOGGER(ls::INFO) << threadNumber <<  ": send failed" << ls::endl;
						}
					}
					pm -> release(connection);
					if(connection -> keepalive == false)
						cm.recycle(connection);
					else
						cm.clear(connection);
					LOGGER(ls::INFO) << threadNumber << ": release ok" << ls::endl;
				}

			}
		}

		int send(Connection *connection, ProtocolManager *pm)
		{
			int ec;
			if(connection -> responseType == "static")
			{
				file::File *file = pm -> getFile(connection);
				return io::api.move(file -> getReader(), connection -> sock.getWriter(), connection -> staticSendBuffer, LS_IO_READ);
			}
			else if(connection -> responseType == "dynamic")
			{
				if(connection -> staticSendBuffer -> size() > 0)
				{
					auto sout = connection -> getStaticOutputStream();
					ec = sout.tryWrite();
					if(ec < 0)
						return ec;
				}
				if(connection -> dynamicSendBuffer && connection -> dynamicSendBuffer -> size() > 0)
				{
					auto dout = connection -> getDynamicOutputStream();
					ec = dout.tryWrite();
					if(ec < 0)
						return ec;
				}
			}
			return Exception::LS_OK;
		}
	}
}

