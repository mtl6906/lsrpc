#ifndef LS_RPC_WORKERMANAGER_H
#define LS_RPC_WORKERMANAGER_H

#include "ls/rpc/Worker.h"
#include "ls/rpc/ProtocolManager.h"
#include "ls/Pool.h"
#include "vector"

namespace ls
{
	namespace rpc
	{
		class WorkerManager
		{
			public:
				WorkerManager(int workerNumber, int connectionNumber, int buffersize);
				~WorkerManager();
				void run(ProtocolManager *pm, QueueManager *qm);
				Worker *nextWorker();
			protected:
				std::vector<Worker *> workers;
		};
	}
}

#endif
