#include "ls/rpc/Tool.h"
#include "ls/http/Response.h"
#include "ls/http/Request.h"
#include "ls/http/QueryString.h"
#include "ls/http/Url.h"
#include "ls/http/StringBody.h"
#include "ls/json/API.h"
#include "ls/DefaultLogger.h"

using namespace std;
using namespace ls;

class JsonProtocol : public rpc::Protocol
{
	public:
		JsonProtocol(const string &tag, int port) : Protocol(tag, port)
		{
		
		}
		~JsonProtocol() override
		{
		
		}

		void readContext(rpc::Connection *connection) override
		{
			auto in = connection -> getInputStream();
			try
			{
				in.tryRead();
			}
			catch(Exception &e)
			{
				if(errno != EAGAIN && errno != EWOULDBLOCK)
					throw e;
			}
			string text;
			try
			{
				text = in.splitOf("", 1);
			}
			catch(Exception &e)
			{
				throw Exception(Exception::LS_ENOCOMPLETE);
			}
			json::Object *request = new json::Object();
			*request = json::api.decode(text);
			connection -> request = request;
		}

		void exec(rpc::Connection *connection) override
		{
			auto request = (json::Object *)connection -> request;
			string cmd;
			json::api.get(*request, "cmd", cmd);
			connection -> response = methodMapper[cmd](request);
			putString(connection);
		}

		void putFile(rpc::Connection *connection) override
		{
			auto response = (json::Object *)connection -> response;
			string filename;
			json::api.get(*response, "file", filename);
			delete response;
			connection -> response = new file::File(filename);
			connection -> responseType = "static";
		}

		void putString(rpc::Connection *connection)
		{
			auto response = (json::Object *)connection -> response;
			string text = json::api.encode(*response);
			connection -> dynamicSendBuffer = new Buffer(text);
			connection -> responseType = "dynamic";
		}

		file::File *getFile(rpc::Connection *connection)
		{
			return (file::File *)connection -> response;
		}

		void release(rpc::Connection *connection)
		{
			if(connection -> request != nullptr)
				delete (json::Object *)connection -> request;
			if(connection -> response != nullptr && connection -> responseType == "static")
				delete (file::File *)connection -> response;
			else if(connection -> response != nullptr)
				delete (json::Object *)connection -> response;
		}

		void add(const string &key, json::Object *(*func)(json::Object*))
		{
			methodMapper[key] = func;
		}
	protected:
		map<string, json::Object*(*)(json::Object *)> methodMapper;
};

class HttpProtocol : public rpc::Protocol
{
	public:
		HttpProtocol(const string &tag, int port) : Protocol(tag, port)
		{
		
		}

		~HttpProtocol() override
		{
			
		}

		void readContext(rpc::Connection *connection)
		{
			auto in = connection -> getInputStream();
			try
			{
				in.tryRead();
			}
			catch(Exception &e)
			{
				if(errno != EAGAIN && errno != EWOULDBLOCK)
					throw e;
			}
		//	throw
			string text;
			try
			{
				text = in.split("\r\n\r\n", true);
			}
			catch(Exception &e)
			{
				throw Exception(Exception::LS_ENOCOMPLETE);
			}
			if(connection -> request == nullptr)
				connection -> request = new http::Request();
			http::Request *request = (http::Request *)connection -> request;
		//	throw 
			try
			{
				if(request -> getMethod() == "")
					request -> parseFrom(text);
			}
			catch(Exception &e)
			{
				LOGGER(ls::INFO) << "parse failed..." << ls::endl;
				delete request;
				throw e;
			}
			if(request -> getMethod() == "GET")
				return;
			LOGGER(ls::INFO) << "request with body..." << ls::endl;
			int contentLength;
			try
			{
				contentLength = stoi(request -> getAttribute("Content-Length"));
			}
			catch(Exception &e)
			{
				request -> setDefaultHeader();
				request -> getMethod() = "GET";
				request -> getURL() = "/error?code=411";
				return ;
			}
		//	throw
			try
			{
				auto text = in.split(contentLength);
				request -> setBody(new http::StringBody(text, ""));
			}
			catch(Exception &e)
			{
				throw Exception(Exception::LS_ENOCOMPLETE);
			}
		}

		void exec(rpc::Connection *connection) override
		{
			LOGGER(ls::INFO) << "map method" << ls::endl;
			auto request = (http::Request*)connection -> request;
			auto uri = http::Url(request -> getURL()).uri;
			auto method = methodMapper[request -> getMethod()].find(uri);
			if(method != methodMapper[request -> getMethod()].end())
			{
				connection -> response = method -> second(request);
				putString(connection);
				return ;
			}
			request -> getMethod() = "GET";
			request -> getURL() = "/error?code=404";
			connection -> response = methodMapper["GET"]["/error"](request);
			putString(connection);
		}

		void putString(rpc::Connection *connection)
		{
			auto response = (http::Response *)connection -> response;
			auto header = response -> toString();
			connection -> staticSendBuffer -> push(header);
			auto body = response -> getBody();
			string text;
			body -> getData(&text);
			LOGGER(ls::INFO) << text << ls::endl;
			connection -> dynamicSendBuffer = new Buffer(text);
			connection -> responseType = "dynamic";
			LOGGER(ls::INFO) << connection -> dynamicSendBuffer -> size() << ls::endl;
		}

		void putFile(rpc::Connection *connection)
		{
			auto response = (http::Response *)connection -> response;
			auto header = response -> toString();
			connection -> staticSendBuffer -> push(header);
			connection -> responseType = "static";
		}

		file::File *getFile(rpc::Connection *connection)
		{
			auto response = (http::Response *)connection -> response;
			auto body = response -> getBody();
			file::File *file;
			body -> getData(&file);
			return file;
		}

		void release(rpc::Connection *connection) override
		{
			if(connection -> response)
				delete (http::Response *)connection -> response;
			if(connection -> request)
				delete (http::Request *)connection -> request;
		}
			
		void add(const string &method, const string &key, http::Response *(*func)(http::Request *request))
		{
			methodMapper[method][key] = func;
		}
	protected:
		map<string, map<string, http::Response *(*)(http::Request *)>> methodMapper;
};

json::Object* hello_json(json::Object *request)
{
	string name;
	int cmdId;
	auto &root = *request;
	json::api.get(root, "name", name);
	json::api.get(root, "cmdId", cmdId);
	json::Object &result = *new json::Object();
	json::api.push(result, "name", name);
	json::api.push(result, "cmdId", cmdId);
	return &result;
}

http::Response* post_hello_http(http::Request *request)
{
	auto *body = request -> getBody();
	string text;
	body -> getData(&text);
	json::Object root = json::api.decode(text);
	http::Response *response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode("200");
	unique_ptr<json::Object> jsonResponse(hello_json(&root));
	response -> setBody(new http::StringBody(jsonResponse -> toString(), "application/json"));
	return response;
}

http::Response* get_hello_http(http::Request *request)
{
	http::QueryString qs;
	auto queryText = http::Url(request -> getURL()).queryText;
	string name = "none";
	if(queryText != "")
	{
		qs.parseFrom(queryText);
		name = qs.getParameter("name");
	}
	json::Object root;
	json::api.push(root, "name", name);
	auto response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode("200");
	response -> setBody(new http::StringBody(root.toString(), "application/json"));
	LOGGER(ls::INFO) << "over" << ls::endl;
	return response;
}

http::Response *error(http::Request *request)
{
	http::QueryString qs;
	qs.parseFrom(http::Url(request -> getURL()).queryText);
	auto code = qs.getParameter("code");
	auto response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode(code);
	response -> setBody(new http::StringBody(code, "text/plain"));
	return response;
}

int main()
{
	JsonProtocol *jp = new JsonProtocol("json", 8081);
	HttpProtocol *hp = new HttpProtocol("http", 8082);
	jp -> add("hello_json", hello_json);
	hp -> add("POST", "/hello_http", post_hello_http);
	hp -> add("GET", "/hello_http", get_hello_http);
	hp -> add("GET", "/error", error);
	rpc::Tool tool;
	tool.push(jp);
	tool.push(hp);
	tool.run();
	return 0;
}
