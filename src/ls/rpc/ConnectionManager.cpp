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
				bufferPool.push(new Buffer(buffersize));
		}

		ConnectionManager::~ConnectionManager()
		{
			while(bufferPool.empty() == false)
			{
				delete bufferPool.front();
				bufferPool.pop();
			}
		}

		bool ConnectionManager::empty()
		{
			return bufferPool.empty();
		}

		int ConnectionManager::assign(int fd, const std::string &tag)
		{
			if(bufferPool.empty())
				return Exception::LS_ENOCONTENT;
			Connection *connection = new Connection(fd);
			connection -> protocol = tag;
			connection -> staticSendBuffer = bufferPool.front();
			bufferPool.pop();
			connection -> recvBuffer = bufferPool.front();
			bufferPool.pop();
			connection -> staticSendBuffer -> clear();
			connection -> recvBuffer -> clear();
			connectionMapper[connection -> fd()] = connection;
		}

		void ConnectionManager::recycle(Connection *connection)
		{
			LOGGER(ls::INFO) << "recycle" << ls::endl;
			if(connection -> staticSendBuffer != nullptr)
			{
				bufferPool.push(connection -> staticSendBuffer);
				connection -> staticSendBuffer = nullptr;
			}
			if(connection -> recvBuffer != nullptr)
			{
				bufferPool.push(connection -> recvBuffer);
				connection -> recvBuffer = nullptr;
			}
			if(connection -> dynamicSendBuffer != nullptr)
			{
				delete connection -> dynamicSendBuffer;
				connection -> dynamicSendBuffer = nullptr;
			}
			int fd = connection -> fd();
			connectionMapper.erase(fd);
			delete connection;
		}

		void ConnectionManager::clear(Connection *connection)
		{
			connection -> staticSendBuffer -> clear();
			connection -> recvBuffer -> clear();
			if(connection -> dynamicSendBuffer != nullptr)
			{
				delete connection -> dynamicSendBuffer;
				connection -> dynamicSendBuffer = nullptr;
			}
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
