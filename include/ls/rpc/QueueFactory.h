#ifndef LS_RPC_QUEUEFACTORY_H
#define LS_RPC_QUEUEFACTORY_H

#include "ls/Queue.h"
#include "ls/rpc/Connection.h"

namespace ls
{
	namespace rpc
	{
		class QueueFactory
		{
			public:	
				virtual ~QueueFactory() {}
				virtual Queue<Connection *> *makeQueue(int size) = 0;
		};
	}
}

#endif
