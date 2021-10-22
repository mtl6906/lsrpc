#ifndef LS_RPC_WORKER_H
#define LS_RPC_WORKER_H

#include "ls/epoll/Tool.h"
#include "ls/rpc/ConnectionManager.h"
#include "string"

namespace ls
{
	namespace rpc
	{
		class ProtocolManager;
		class Worker
		{
			public:
				Worker(int connectionNumber);
				void run(ConnectionManager *cm, ProtocolManager *pm);
				void add(int connfd, int type);
			protected:
				epoll::Tool et;
		};

		void send(Connection *connection, ProtocolManager *pm);
	}
}

#endif
