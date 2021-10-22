#include "ls/rpc/ConnectionManager.h"
#include "ls/Exception.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		ConnectionManager::ConnectionManager(int connectionNumber, int buffersize)
		{
			for(int i=0;i<connectionNumber;++i)
				bufferPool.put(new Buffer(buffersize));
		}

		void ConnectionManager::assign(int fd, const string &protocol)
		{
			Connection *connection = new Connection(fd);
			connection -> staticSendBuffer = bufferPool.get();
			connection -> recvBuffer = bufferPool.get();
			connection -> staticSendBuffer -> clear();
			connection -> recvBuffer -> clear();
			connection -> protocol = protocol;
			connectionMapper[fd] = connection;
		}

		void ConnectionManager::recycle(int fd)
		{
			auto connection = connectionMapper[fd];
			bufferPool.put(connection -> staticSendBuffer);
			bufferPool.put(connection -> recvBuffer);
			if(connection -> dynamicSendBuffer != nullptr)
				delete connection -> dynamicSendBuffer;
			delete connection;
			connectionMapper.erase(fd);
		}

		Connection *ConnectionManager::get(int fd)
		{
			auto it = connectionMapper.find(fd);
			if(it == connectionMapper.end())
				throw Exception(Exception::LS_ENOCONTENT);
			return it -> second;
		}
	}
}
