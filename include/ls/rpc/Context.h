#ifndef LS_RPC_CONTEXT_H
#define LS_RPC_CONTEXT_H

namespace ls
{
	namespace rpc
	{
		class Context
		{
			public:
				virtual void read(Connection *connection);
				virtual void write(Connection *connection);
				virtual ~Context();
		};
	}
}

#endif
