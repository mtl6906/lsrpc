#ifndef LS_RPC_QUEUEMANAGER_H
#define LS_RPC_QUEUEMANAGER_H

#include "ls/Queue.h"
#include "ls/rpc/Connection.h"
#include "atomic"

namespace ls
{
	namespace rpc
	{
		class QueueManager
		{	
			public:
				QueueManager(int workerNumber, Queue<Connection *> *q);
				~QueueManager();
				int put(Connection *connection, int threadNumber);
				Connection* get(int &ec, int threadNumber);
				bool iswait(int i);
				void run(int protocolNumber);
				void resetThread(int protocolNumber);
				int size();
			protected:
				int workerNumber;
				Queue<Connection *> *q;
				std::atomic<bool> waits[2];
		};
	}
}

#endif
