#include "ls/rpc/WorkerManager.h"
#include "thread"

using namespace std;

namespace ls
{
	namespace rpc
	{
		WorkerManager::WorkerManager(int workerNumber, int connectionNumber) : workers(workerNumber)
		{
			for(int i=0;i<workerNumber;++i)
				workers[i] = new Worker(connectionNumber);
		}

		WorkerManager::~WorkerManager()
		{
			for(auto &worker : workers)
				delete worker;
		}

		void WorkerManager::run(ConnectionManager *cm, ProtocolManager *pm)
		{
			for(auto worker : workers)
				thread(&Worker::run, worker, cm, pm).detach();
		}
	
		Worker *WorkerManager::nextWorker()
		{
			currentWorker = (currentWorker + 1)%workers.size();
			return workers[currentWorker];
		}
	}
}
