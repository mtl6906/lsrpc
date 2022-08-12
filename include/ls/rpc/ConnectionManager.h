#ifndef LS_RPC_CONNECTIONMANAGER_H
#define LS_RPC_CONNECTIONMANAGER_H

#include "ls/Pool.h"
#include "ls/io/InputStream.h"
#include "ls/io/OutputStream.h"
#include "ls/rpc/Connection.h"
#include "map"
#include "queue"

namespace ls
{
	namespace rpc
	{
		class ConnectionManager
		{
			public:
				ConnectionManager(int connectionNumber, int buffersize);
				~ConnectionManager();
				void assign(int fd, const std::string &tag);
				void recycle(Connection *connection);
				void clear(Connection *connection);
				Connection *get(int fd);
				bool empty();
			protected:
				std::map<int, Connection *> connectionMapper;
				std::queue<Buffer *> bufferPool;
		};
	}
}

#endif
