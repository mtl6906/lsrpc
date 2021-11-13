#ifndef LS_RPC_CONNECTION_H
#define LS_RPC_CONNECTION_H

#include "ls/io/InputStream.h"
#include "ls/io/OutputStream.h"
#include "ls/net/Socket.h"
#include "string"

namespace ls
{
	namespace rpc
	{
		class Connection
		{
			public:
				Connection(int connfd);
				io::InputStream getInputStream();
				io::OutputStream getStaticOutputStream();
				io::OutputStream getDynamicOutputStream();
				net::Socket sock;
				Buffer *staticSendBuffer;
				Buffer *dynamicSendBuffer;
				Buffer *recvBuffer;
				std::string protocol;
				std::string responseType;
				void *request;
				void *response;
				bool isRelease;

		};
	}
}

#endif
