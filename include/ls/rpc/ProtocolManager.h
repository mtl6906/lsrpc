#ifndef LS_RPC_PROTOCOLMANAGER_H
#define LS_RPC_PROTOCOLMANAGER_H

#include "ls/rpc/Protocol.h"
#include "ls/epoll/Tool.h"
#include "ls/file/File.h"
#include "ls/rpc/QueueManager.h"
#include "map"
#include "string"
#include "atomic"

namespace ls
{
	namespace rpc
	{

		class Protocol;
		class ProtocolManager
		{
			public:
				ProtocolManager();
				~ProtocolManager();
				void run(QueueManager *qm);
				int exec(Connection *connection);
				int readContext(Connection *connection);
				void push(Protocol *protocol);
				file::File* getFile(Connection *connection);
				void release(Connection *connection);
				int size();
				Protocol *get(int i);
			protected:
				std::map<std::string, Protocol *> protocols;
				std::vector<Protocol *> _protocols;
		};
	}
}

#endif
