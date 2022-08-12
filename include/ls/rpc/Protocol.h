#ifndef LS_RPC_PROTOCOL_H
#define LS_RPC_PROTOCOL_H

#include "ls/rpc/QueueManager.h"
#include "ls/net/Server.h"
#include "ls/file/File.h"
#include "map"
#include "string"

namespace ls
{
	namespace rpc
	{
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
				std::string &getTag();
				int fd();
				net::Server &getServer()
				{
					return server;
				}
				void run(QueueManager *qm, int threadNumber);
			protected:
				std::string tag;
				net::Server server;
		};
	}
}

#endif
