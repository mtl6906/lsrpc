#include "ls/rpc/Worker.h"
#include "ls/epoll/Tool.h"
#include "ls/rpc/main.h"
#include "ls/io/API.h"
#include "ls/Exception.h"
#include "ls/DefaultLogger.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Worker::Worker(int connectionNumber) : et(connectionNumber)
		{

		}
		
		void Worker::run(ConnectionManager *cm, ProtocolManager *pm)
		{
			for(;;)	
			{
				int n = et.wait(-1);
				LOGGER(ls::INFO) << n << " epoll event happened..." << ls::endl;
				for(int i=0;i<n;++i)
				{
					auto &event = et.event(i);
					Connection *connection;
				       	try
					{
						connection = cm -> get(event.data.fd);
					}
					catch(Exception &e)
					{
						continue;
					}
					if(event.events & EPOLLIN)
					{
						LOGGER(ls::INFO) << i << ": read event on " << event.data.fd << ls::endl;
						try
						{
							pm -> readContext(connection);
						}
						catch(Exception &e)
						{
							if(e.getCode() != Exception::LS_ENOCOMPLETE)
							{
								pm -> release(connection);
								cm -> recycle(event.data.fd);
							}
							continue;	
						}
						LOGGER(ls::INFO) << "exec protocol..." << ls::endl;
						pm -> exec(connection);
						try
						{	
							send(connection, pm);
						}
						catch(Exception &e)
						{
							if(e.getCode() == Exception::LS_EWOULDBLOCK)
							{
								et.mod(event.data.fd, EPOLLOUT | EPOLLET);
								continue;
							}
						}
						if(connection -> isRelease == true)
						{
							pm -> release(connection);
							cm -> recycle(event.data.fd);
						}
					}
					else if(event.events & EPOLLOUT)
					{
						LOGGER(ls::INFO) << i << ": write event on " << event.data.fd << ls::endl;
						try
						{
							send(connection, pm);
						}
						catch(Exception &e)
						{
							if(e.getCode() == Exception::LS_EWOULDBLOCK)
								continue;	
						}
						if(connection -> isRelease == true)
						{
							pm -> release(connection);
							cm -> recycle(event.data.fd);
						}
					}
					LOGGER(ls::INFO) << "fd " << event.data.fd << " has been released!" << ls::endl;
				}
			}
		}

		void Worker::add(int connfd, int type)
		{
			et.add(connfd, type);
		}

		void send(Connection *connection, ProtocolManager *pm)
		{
			if(connection -> responseType == "static")
			{
				file::File *file = pm -> getFile(connection);
				io::api.move(file -> getReader(), connection -> sock.getWriter(), connection -> staticSendBuffer, LS_IO_READ);
			}
			else if(connection -> responseType == "dynamic")
			{
				if(connection -> staticSendBuffer -> size() > 0)
				{
					auto sout = connection -> getStaticOutputStream();
					sout.tryWrite();
				}
				if(connection -> dynamicSendBuffer -> size() > 0)
				{
					auto dout = connection -> getDynamicOutputStream();
					dout.tryWrite();
				}
			}
		}
	}
}

