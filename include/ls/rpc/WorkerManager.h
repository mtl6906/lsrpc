#ifndef LS_RPC_WORKERMANAGER_H
#define LS_RPC_WORKERMANAGER_H

#include "ls/rpc/Worker.h"
#include "vector"

namespace ls
{
	namespace rpc
	{
		class WorkerManager
		{
			public:
				WorkerManager(int workerNumber, int connectionNumber);
				~WorkerManager();
				void run(ConnectionManager *cm, ProtocolManager *pm);
				Worker *nextWorker();
			protected:
				std::vector<Worker *> workers;
				int currentWorker;
		};
	}
}

#endif
