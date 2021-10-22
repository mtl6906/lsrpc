#include "ls/rpc/Protocol.h"
#include "ls/net/Server.h"
#include "ls/rpc/main.h"
#include "ls/DefaultLogger.h"
#include "thread"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Protocol::Protocol(const string &tag, int port) : tag(tag), port(port)
		{
				
		}

		Protocol::~Protocol()
		{
		}

		void Protocol::run(ConnectionManager *cm, WorkerManager *wm)
		{
			net::Server server(port);
			for(;;)
			{
				int connfd = server.accept();
				cm -> assign(connfd, tag);
				wm -> nextWorker() -> add(connfd, EPOLLIN | EPOLLET);
				LOGGER(ls::INFO) << "accept fd " << connfd << " on port " << port << ls::endl;
			}
		}

		string &Protocol::getTag()
		{
			return tag;
		}
	}
}
