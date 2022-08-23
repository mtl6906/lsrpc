#include "ls/rpc/ConnectionManager.h"
#include "ls/Exception.h"
#include "ls/DefaultLogger.h"
#include "unistd.h"
#include "thread"
#include "atomic"


using namespace std;

namespace ls
{
	namespace rpc
	{
		ConnectionManager::ConnectionManager(int connectionNumber, int buffersize)
		{
			for(int i=0;i<connectionNumber * 2;++i)
				connectionPool.push(new Connection(-1, buffersize));
		}

		ConnectionManager::~ConnectionManager()
		{
			while(connectionPool.empty() == false)
			{
				delete connectionPool.front();
				connectionPool.pop();
			}
		}

		bool ConnectionManager::empty()
		{
			return connectionPool.empty();
		}

		int ConnectionManager::assign(int fd, const std::string &tag)
		{
			auto connection = connectionPool.front();
			connectionPool.pop();
			connection -> reset(fd, tag);
			connectionMapper[fd] = connection;
		}

		void ConnectionManager::recycle(Connection *connection)
		{
			LOGGER(ls::INFO) << "recycle" << ls::endl;
			connectionMapper.erase(connection -> fd());
			connectionPool.push(connection);
		}

		Connection *ConnectionManager::get(int fd)
		{
			auto it = connectionMapper.find(fd);
			if(it == connectionMapper.end())
				return nullptr;
			return it -> second;
		}
	}
}
