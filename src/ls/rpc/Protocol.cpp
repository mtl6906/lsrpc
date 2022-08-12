#include "ls/rpc/Protocol.h"
#include "ls/net/Server.h"
#include "ls/rpc/main.h"
#include "ls/DefaultLogger.h"
#include "thread"
#include "fcntl.h"
#include "unistd.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Protocol::Protocol(const string &tag, int port) : tag(tag), server(port)
		{
//			fcntl(server.getFd(), F_SETFL, O_NONBLOCK);
		}

		Protocol::~Protocol()
		{
		
		}

		string &Protocol::getTag()
		{
			return tag;
		}

		int Protocol::fd()
		{
			return server.getFd();
		}

		void Protocol::run(QueueManager *qm, int threadNumber)
		{
			for(;;)
			{
				if(qm -> iswait(0))
				{
					usleep(1000);
					continue;
				}
				int connfd;
				try
				{
			//		LOGGER(ls::INFO) << tag << ": accepting" << ls::endl;
					connfd = server.accept();
				}
				catch(Exception &e)
				{
			//		LOGGER(ls::INFO) << tag << ": " << e.what() << ls::endl;
					usleep(1000);
					continue;
				}
				Connection *connection = nullptr;
				try
				{
					LOGGER(ls::INFO) << tag << " accept on fd " << connfd << ls::endl;
					connection = new Connection(connfd);
					connection -> protocol = getTag();
					qm -> put(connection, threadNumber);
					LOGGER(ls::INFO) << qm -> size() << ls::endl;
					LOGGER(ls::INFO) << "put ok" << ls::endl;
				}
				catch(Exception &e)
				{
					LOGGER(ls::INFO) << "full" << ls::endl;
					delete connection;
				}
			}
		}
	}
}
