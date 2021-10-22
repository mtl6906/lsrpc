#include "ls/rpc/Config.h"
#include "ls/DefaultLogger.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Config::Config()
		{
			load();	
		}
		
		string Config::getConfigPath()
		{
			char *path = getenv("LS_RPC_CONFIG_PATH");
			if(nullptr == path)
				return "config.json";
			return path;
		}

		void Config::init()
		{
			json::api.get(root, "buffersize", buffersize);
			json::api.get(root, "connectionNumber", connectionNumber);
			json::api.get(root, "workerNumber", workerNumber);

			LOGGER(ls::INFO) << "buffersize: " << buffersize << ls::endl;
			LOGGER(ls::INFO) << "connectionNumber: " << connectionNumber << ls::endl;
			LOGGER(ls::INFO) << "workerNumber: " << workerNumber << ls::endl;

		}	
	}
}
