#ifndef LS_RPC_TOOL_H
#define LS_RPC_TOOL_H

#include "ls/rpc/QueueFactory.h"
#include "ls/rpc/WorkerManager.h"
#include "ls/rpc/ProtocolManager.h"
#include "ls/rpc/QueueManager.h"
#include "ls/rpc/Config.h"
#include "string"

namespace ls
{
	namespace rpc
	{
		class Tool
		{
			public:
				Tool(QueueFactory &qf);
				void run();
				void push(Protocol *protocol);
			protected:
				Config config;
				QueueManager qm;
				WorkerManager wm;
				ProtocolManager pm;
		};
	}
}

#endif
