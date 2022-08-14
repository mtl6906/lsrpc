#include "ls/rpc/ProtocolManager.h"
#include "ls/rpc/QueueManager.h"
#include "ls/DefaultLogger.h"
#include "ls/Exception.h"
#include "thread"
#include "unistd.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		ProtocolManager::ProtocolManager()
		{
		}

		ProtocolManager::~ProtocolManager()
		{
			for(auto &it : protocols)
				delete it.second;
		}

		void ProtocolManager::run(QueueManager *qm)
		{
			int threadNumber = 0;
			for(auto &it : protocols)
				thread(&Protocol::run, it.second, qm, threadNumber++).detach();
		}
		
		int ProtocolManager::size()
		{
			return protocols.size();
		}		

		int ProtocolManager::exec(Connection *connection)
		{
			auto it = protocols.find(connection -> protocol);
			if(it == protocols.end())
				return Exception::LS_ENOCONTENT;
			it -> second -> exec(connection);
			return Exception::LS_OK;
		}

		void ProtocolManager::push(Protocol *protocol)
		{
			protocols[protocol -> getTag()] = protocol;
			_protocols.push_back(protocol);
		}

		int ProtocolManager::readContext(Connection *connection)
		{
			return protocols[connection -> protocol] -> readContext(connection);
		}

		file::File *ProtocolManager::getFile(Connection *connection)
		{
			protocols[connection -> protocol] -> getFile(connection);
		}

		void ProtocolManager::release(Connection *connection)
		{
			protocols[connection -> protocol] -> release(connection);
		}

		Protocol *ProtocolManager::get(int i)
		{
			return _protocols[i];
		}
	}
}
