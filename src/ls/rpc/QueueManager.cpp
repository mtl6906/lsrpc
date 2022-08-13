#include "ls/rpc/QueueManager.h"
#include "ls/DefaultLogger.h"
#include "ls/Exception.h"
#include "thread"
#include "unistd.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		QueueManager::QueueManager(int workerNumber, Queue<Connection*> *q) : q(q), workerNumber(workerNumber)
		{
		}

		QueueManager::~QueueManager()
		{
		//	delete waits;
			delete q;
		}

		int QueueManager::put(Connection *connection, int threadNumber)
		{
		
			int ec = q -> push(connection);
			if(ec < 0)
			{
		//		waits[threadNumber].store(true);
				waits[0].store(true);
				return ec;
			}
		}

		Connection *QueueManager::get(int &ec, int threadNumber)
		{
			auto connection = q -> pop(ec);
			LOGGER(ls::INFO) << threadNumber << ": over" << ls::endl;
			if(ec < 0)
			{
				if(ec == Exception::LS_ERESET)
					waits[1].store(true);
//					waits[threadNumber].store(true);
				return nullptr;
			}
			return connection;
		}
		bool QueueManager::iswait(int i)
		{
			return waits[i].load();
		}

		int QueueManager::size()
		{
			return q -> size();
		}

		void QueueManager::resetThread(int _protocolNumber)
		{
//			protocolNumber = _protocolNumber;
		//	waits = (std::atomic<bool> *)operator new (sizeof(std::atomic<bool>) * (workerNumber + protocolNumber));
                //       for(int i=0;i<workerNumber+protocolNumber;++i)
                //               new (waits + i)atomic<bool>(false);		
			for(;;)
			{
		/*		int cnt = 0;
				for(int i=0;i<workerNumber + protocolNumber;++i)
					if(waits[i].load() == true)
						++cnt;
				if(i < workerNumber + 1)
				{
					usleep(1000);
					continue;
				}
				for(int i=0;i<protocolNumber;++i)
					if(waits[i].load() == false)
						waits[i].store(true);
		*
		*/
				if(waits[0].load() == false || waits[1].load() == false)
				{
					usleep(1000);		
					continue;
				}
				LOGGER(ls::INFO) << "reset" << ls::endl;
				q -> reset();
//				for(int i=workerNumber + protocolNumber-1;i>=0;--i)
//					waits[i].store(false);
				waits[1].store(false);
				waits[0].store(false);
			}
		//	for(int i=0;i<workerNumber+protocolNumber;++i)
		//		((atomic<bool> *)(waits + i)) -> ~atomic();
			
		}

		void QueueManager::run(int protocolNumber)
		{
			thread(&QueueManager::resetThread, this, protocolNumber).detach();
		}
	}
}
