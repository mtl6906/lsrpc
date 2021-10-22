#ifndef LS_RPC_RPCCONFIG_H
#define LS_RPC_RPCCONFIG_H

#include "ls/Config.h"

namespace ls
{
	namespace rpc
	{
		class Config : public ls::Config
		{
			public:
				Config();
				void init() override;
				std::string getConfigPath() override;
				int connectionNumber;
				int buffersize;
				int workerNumber;
		};
	}
}


#endif
