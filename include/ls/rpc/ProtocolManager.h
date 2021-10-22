#ifndef LS_RPC_PROTOCOLMANAGER_H
#define LS_RPC_PROTOCOLMANAGER_H

#include "ls/rpc/Protocol.h"
#include "ls/file/File.h"
#include "map"
#include "string"

namespace ls
{
	namespace rpc
	{
		class ProtocolManager
		{
			public:
				~ProtocolManager();
				void run(ConnectionManager *cm, WorkerManager *wm);
				void exec(Connection *connection);
				void readContext(Connection *connection);
				void push(Protocol *protocol);
				file::File* getFile(Connection *connection);
				void release(Connection *connection);
			protected:
				std::map<std::string, Protocol *> protocols;	
		};
	}
}

#endif
