#include "ls/rpc/Connection.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Connection::Connection(int connfd) : sock(connfd), request(nullptr), response(nullptr), recvBuffer(nullptr), dynamicSendBuffer(nullptr), staticSendBuffer(nullptr)
		{
		}

		io::InputStream Connection::getInputStream()
		{
			return io::InputStream(sock.getReader(), recvBuffer);
		}

		io::OutputStream Connection::getStaticOutputStream()
		{
			return io::OutputStream(sock.getWriter(), staticSendBuffer);
		}

		io::OutputStream Connection::getDynamicOutputStream()
		{
			return io::OutputStream(sock.getWriter(), dynamicSendBuffer);
		}
	}
}
