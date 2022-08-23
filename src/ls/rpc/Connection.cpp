#include "ls/rpc/Connection.h"

using namespace std;

namespace ls
{
	namespace rpc
	{
		Connection::Connection(int connfd, int buffersize) : sock(connfd), recvBuffer(buffersize), sendBuffer(buffersize), file(nullptr), keepalive(false)
		{
		}

		io::InputStream Connection::getInputStream()
		{
			return io::InputStream(sock.getReader(), &recvBuffer);
		}

		io::OutputStream Connection::getOutputStream()
		{
			return io::OutputStream(sock.getWriter(), &sendBuffer);
		}

		void Connection::clear()
		{
			recvBuffer.clear();
			sendBuffer.clear();
		}

		void Connection::reset(int fd, const string &tag)
		{
			sock.reset(fd);
			this -> protocol = tag;
			clear();
		}

		void Connection::reset(file::File *file)
		{
			this -> file.reset(file);
		}
	}
}
