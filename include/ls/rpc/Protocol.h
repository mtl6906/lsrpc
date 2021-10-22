#ifndef LS_RPC_PROTOCOL_H
#define LS_RPC_PROTOCOL_H

#include "ls/rpc/ConnectionManager.h"
#include "ls/file/File.h"
#include "map"
#include "string"

namespace ls
{
	namespace rpc
	{
		class WorkerManager;
		class Protocol
		{
			public:
				Protocol(const std::string &tag, int port);
				virtual ~Protocol();
				virtual void exec(Connection *connection) = 0;
				virtual void readContext(Connection *connection) = 0;
				virtual void putString(Connection *connection) = 0;
				virtual void putFile(Connection *connection) = 0;
				virtual file::File* getFile(Connection *connection) = 0;
				virtual void release(Connection *connection) = 0;
				void run(ConnectionManager *cm, WorkerManager *wm);
				std::string &getTag();
			protected:
				std::string tag;
				int port;
		};
	}
}

#endif
