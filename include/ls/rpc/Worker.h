#ifndef LS_RPC_WORKER_H
#define LS_RPC_WORKER_H

#include "ls/epoll/Tool.h"
#include "ls/rpc/ConnectionManager.h"
#include "ls/rpc/QueueManager.h"
#include "string"

namespace ls
{
	namespace rpc
	{
		class ProtocolManager;
		class Worker
		{
			public:
				Worker(int connectionNumber, int buffersize);
				void run(ProtocolManager *pm, QueueManager *qm, int threadNumber);
			protected:
				ConnectionManager cm;
				epoll::Tool et;
		};

		int send(Connection *connection, ProtocolManager *pm);
	}
}

#endif
