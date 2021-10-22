#include "ls/rpc/ProtocolManager.h"
#include "ls/Exception.h"
#include "thread"

using namespace std;

namespace ls
{
	namespace rpc
	{
		ProtocolManager::~ProtocolManager()
		{
			for(auto &it : protocols)
				delete it.second;
		}

		void ProtocolManager::run(ConnectionManager *cm, WorkerManager *wm)
		{
			for(auto &it : protocols)
				thread(&Protocol::run, it.second, cm, wm).detach();		
		}

		void ProtocolManager::exec(Connection *connection)
		{
			auto it = protocols.find(connection -> protocol);
			if(it == protocols.end())
				throw Exception(Exception::LS_ENOCONTENT);
			it -> second -> exec(connection);
		}

		void ProtocolManager::push(Protocol *protocol)
		{
			protocols[protocol -> getTag()] = protocol;
		}

		void ProtocolManager::readContext(Connection *connection)
		{
			protocols[connection -> protocol] -> readContext(connection);
		}

		file::File *ProtocolManager::getFile(Connection *connection)
		{
			protocols[connection -> protocol] -> getFile(connection);
		}

		void ProtocolManager::release(Connection *connection)
		{
			protocols[connection -> protocol] -> release(connection);
		}
	}
}
