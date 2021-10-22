#ifndef LS_RPC_CONNECTIONMANAGER_H
#define LS_RPC_CONNECTIONMANAGER_H

#include "ls/Pool.h"
#include "ls/io/InputStream.h"
#include "ls/io/OutputStream.h"
#include "ls/rpc/Connection.h"
#include "map"

namespace ls
{
	namespace rpc
	{
		class ConnectionManager
		{
			public:
				ConnectionManager(int connectionNuber, int buffersize);
				void assign(int fd, const std::string &protocol);
				void recycle(int fd);
				Connection *get(int fd);
			protected:
				std::map<int, Connection *> connectionMapper;
				Pool<Buffer> bufferPool;
		};
	}
}

#endif
