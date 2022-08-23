#ifndef LS_RPC_CONNECTION_H
#define LS_RPC_CONNECTION_H

#include "ls/io/InputStream.h"
#include "ls/io/OutputStream.h"
#include "ls/net/Socket.h"
#include "ls/file/File.h"
#include "string"

#define READING 0
#define WRITING 1
#define ENDING 2

namespace ls
{
	namespace rpc
	{
		class Connection
		{
			public:
				Connection(int connfd, int buffersize);
				io::InputStream getInputStream();
				io::OutputStream getOutputStream();
				void clear();
				void reset(int fd, const std::string &tag);
				void reset(file::File *file);
				net::Socket sock;
				Buffer sendBuffer;
				Buffer recvBuffer;
				std::string protocol;
				std::unique_ptr<file::File> file;
				bool keepalive;
				int fd()
				{
					return sock.getFd();	
				}
		};
	}
}

#endif
