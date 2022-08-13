#include "ls/rpc/Tool.h"
#include "CASQueueFactory.h"
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

		int readContext(rpc::Connection *connection) override
		{
			auto in = connection -> getInputStream();
			
			LOGGER(ls::INFO) << "ok " << in.getBuffer() -> restSize() << ls::endl;
			int ec = in.tryRead();
			LOGGER(ls::INFO) << "success" << ls::endl;
			if(ec < 0)
			{
//				LOGGER(ls::INFO) << in.getBuffer() -> begin() << " " << in.getBuffer() -> size()<< ls::endl;
				LOGGER(ls::INFO) << "error" << ls::endl;
				if(errno != EAGAIN && errno != EWOULDBLOCK)
					return ec;
			}
			string text;
			text = in.splitOf(ec, "", 1);
			if(ec < 0)
			{
				LOGGER(ls::INFO) << in.getBuffer() -> begin() << ls::endl;
				return Exception::LS_ENOCOMPLETE;
			}
			json::Object *request = new json::Object();
			*request = json::api.decode(ec, text);
			if(ec < 0)
				return ec;
			connection -> request = (void *)request;
			return Exception::LS_OK;
		}

		int exec(rpc::Connection *connection) override
		{
			auto request = (json::Object *)connection -> request;
			string cmd;
			int ec = json::api.get(*request, "cmd", cmd);
			if(ec < 0)
				return ec;
			connection -> response = methodMapper[cmd](request);
			putString(connection);
			return ec;
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
			LOGGER(ls::INFO) << ls::endl;
			if(connection -> request != nullptr)
			{
				delete (json::Object *)connection -> request;
				connection -> request = nullptr;
			}
			LOGGER(ls::INFO) << ls::endl;
			if(connection -> response != nullptr && connection -> responseType == "static")
			{
				delete (file::File *)connection -> response;
				connection -> response = nullptr;
			}
			LOGGER(ls::INFO) << ls::endl;
			else if(connection -> response != nullptr)
			{
				delete (json::Object *)connection -> response;
				connection -> response = nullptr;
			}
			LOGGER(ls::INFO) << ls::endl;
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

		int readContext(rpc::Connection *connection)
		{
			auto in = connection -> getInputStream();
			int ec = in.tryRead();
			if(ec < 0)
			{
				if(errno != EAGAIN && errno != EWOULDBLOCK)
					return ec;
			}
		//	throw
			string text = in.split(ec, "\r\n\r\n", true);
			if(ec < 0)
				return Exception::LS_ENOCOMPLETE;
			if(connection -> request == nullptr)
				connection -> request = new http::Request();
			http::Request *request = (http::Request *)connection -> request;
		//	throw 
			if(request -> getMethod() == "")
			{
				int ec = request -> parseFrom(text);
				if(ec < 0)
				{
					LOGGER(ls::INFO) << "parse failed..." << ls::endl;
					delete request;
					return ec;
				}
			}
			if(request -> getMethod() == "GET")
				return ec;
			LOGGER(ls::INFO) << "request with body..." << ls::endl;
			auto contentLength = request -> getAttribute(ec, "Content-Length");
			if(ec < 0)
			{
				request -> setDefaultHeader();
				request -> getMethod() = "GET";
				request -> getURL() = "/error?code=411";
				return ec;
			}
			int len = stoi(contentLength);
		//	throw
			text = in.split(ec, len);
			if(ec < 0)
				return Exception::LS_ENOCOMPLETE;
			request -> setBody(new http::StringBody(text, ""));
			return ec;
		}

		int exec(rpc::Connection *connection) override
		{
			LOGGER(ls::INFO) << "map method" << ls::endl;
			auto request = (http::Request*)connection -> request;
			auto uri = http::Url(request -> getURL()).uri;
			auto method = methodMapper[request -> getMethod()].find(uri);
			if(method != methodMapper[request -> getMethod()].end())
			{
				connection -> response = method -> second(request);
				putString(connection);
				return Exception::LS_OK;
			}
			request -> getMethod() = "GET";
			request -> getURL() = "/error?code=404";
			connection -> response = methodMapper["GET"]["/error"](request);
			putString(connection);
			return Exception::LS_OK;
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

json::Object* json_error(int code)
{
	auto root = new json::Object();
	json::api.push(*root, "code", code);
	string message = Exception(code).what();
	json::api.push(*root, "message", message);
	json::Object data;
	json::api.push(*root, "data", data);
	return root;
}

json::Object* hello_json(json::Object *request)
{
	int ec;
	auto &root = *request;
	string name;
	json::Object data;
	if((ec = json::api.get(root, "name", name)) == 0)
		json::api.push(data, "name", name);	
	auto result = new json::Object();
	json::api.push(*result, "code", ec);
	string message = Exception(ec).what();
	json::api.push(*result, "message", message);
	json::api.push(*result, "data", data);
	return result;
}

http::Response* post_hello_http(http::Request *request)
{
	int ec;
	auto *body = request -> getBody();
	string text;
	body -> getData(&text);
	json::Object root = json::api.decode(ec, text);
	unique_ptr<json::Object> jsonResponse;
	if(ec < 0)
		jsonResponse.reset(json_error(ec));
	else
		jsonResponse.reset(hello_json(&root));
	auto response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode("200");
	response -> setBody(new http::StringBody(jsonResponse -> toString(), "application/json"));
	return response;
}

http::Response* get_hello_http(http::Request *request)
{
	int ec;
	http::QueryString qs;
	auto queryText = http::Url(request -> getURL()).queryText;
	unique_ptr<json::Object> jsonResponse;
	if(queryText == "" || qs.parseFrom(queryText) < 0)
		jsonResponse.reset(json_error(Exception::LS_EFORMAT));
	else
	{
		json::Object root;
		for(auto &it : qs.getData().getMapper())
			it.second = string("\"") + it.second + "\"";
		root.getData() = qs.getData();
		jsonResponse.reset(hello_json(&root));
	}
	auto response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode("200");
	response -> setBody(new http::StringBody(jsonResponse -> toString(), "application/json"));
	return response;
}

http::Response *error(http::Request *request)
{
	int ec = Exception::LS_OK;
	http::QueryString qs;
	qs.parseFrom(http::Url(request -> getURL()).queryText);
	auto code = qs.getParameter(ec, "code");
	auto response = new http::Response();
	response -> setDefaultHeader(*request);
	response -> setCode(code);
	response -> setBody(new http::StringBody(code, "text/plain"));
	return response;
}

int main()
{
	JsonProtocol *jp = new JsonProtocol("json", 8081);
	HttpProtocol *hp = new HttpProtocol("http", 8083);
	jp -> add("hello_json", hello_json);
	hp -> add("POST", "/hello_http", post_hello_http);
	hp -> add("GET", "/hello_http", get_hello_http);
	hp -> add("GET", "/error", error);
	
	CASQueueFactory casqf;
	rpc::Tool tool(casqf);
	tool.push(jp);
	tool.push(hp);
	tool.run();
	return 0;
}
