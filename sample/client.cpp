#include "ls/net/Client.h"
#include "ls/net/Socket.h"
#include "ls/io/InputStream.h"
#include "ls/io/OutputStream.h"
#include "ls/json/API.h"
#include "ls/Buffer.h"
#include "iostream"

using namespace std;
using namespace ls;

int main(int argc, char **argv)
{
	net::Client client(argv[1], 8081);
	net::Socket sock(client.connect());
	io::OutputStream out(sock.getWriter(), new Buffer());
	io::InputStream in(sock.getReader(), new Buffer());

	json::Object root;
	json::api.push(root, "cmd", string("hello_json"));
	json::api.push(root, "cmdId", int(1));
	json::Object parameter;
	json::api.push(parameter, "name", string("lx"));
	json::api.push(root, "parameter", parameter);

	out.append(json::api.encode(root));
	out.append("", 1);
	out.write();

	try
	{
		in.read();
	}
	catch(Exception &e)
	{
		cout << "read failed" << endl;
		return 0;
	}
	cout << in.split() << endl;
	return 0;
}
