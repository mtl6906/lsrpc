#ifndef LS_RPC_TOOL_H
#define LS_RPC_TOOL_H

#include "ls/rpc/WorkerManager.h"
#include "ls/rpc/ProtocolManager.h"
#include "ls/rpc/ConnectionManager.h"
#include "ls/rpc/Config.h"
#include "string"

namespace ls
{
	namespace rpc
	{
		class Tool
		{
			public:
				Tool();
				void run();
				void push(Protocol *protocol);
			protected:
				std::unique_ptr<Config> config;
				std::unique_ptr<WorkerManager> wm;
				std::unique_ptr<ConnectionManager> cm;
				std::unique_ptr<ProtocolManager> pm;
		};
	}
}

#endif
