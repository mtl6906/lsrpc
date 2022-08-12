#include "ls/rpc/WorkerManager.h"
#include "thread"

using namespace std;

namespace ls
{
	namespace rpc
	{
		WorkerManager::WorkerManager(int workerNumber, int connectionNumber, int buffersize) : workers(workerNumber)
		{
			for(int i=0;i<workerNumber;++i)
				workers[i] = new Worker(connectionNumber, buffersize);
		}

		WorkerManager::~WorkerManager()
		{
			for(auto &worker : workers)
				delete worker;
		}

		void WorkerManager::run(ProtocolManager *pm, QueueManager *qm)
		{
			for(int i=0;i<workers.size();++i)
				thread(&Worker::run, workers[i], pm, qm, i).detach();
		}
	}
}
