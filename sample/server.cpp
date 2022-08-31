#include "ls/rpc/Tool.h"
#include "CASQueueFactory.h"
#include "ls/http/Response.h"
#include "ls/http/Request.h"
#include "ls/http/QueryString.h"
#include "ls/http/Url.h"
#include "ls/json/API.h"
#include "ls/DefaultLogger.h"
#include "ls/file/API.h"

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
			request = json::api.decode(ec, text);
			return ec;
		}

		void error(int code)
		{
			json::api.push(response, "code", code);
			json::api.push(response, "message", Exception(code).message());
			json::Object data;
		 	json::api.push(response, "data", data);	
		}

		int exec(rpc::Connection *connection) override
		{
			string cmd;
			int ec = json::api.get(request, "cmd", cmd);
			if(ec < 0 || (ec = methodMapper[cmd](request, response)) < 0)
				error(ec);
			putString(connection);
			return ec;
		}

		void putFile(rpc::Connection *connection) override
		{
			string filename;
			json::api.get(response, "file", filename);
			connection -> file.reset(file::api.get(filename));
		}

		void putString(rpc::Connection *connection)
		{
			string text = json::api.encode(response);
			connection -> sendBuffer.push(text);
			connection -> file.reset(nullptr);
		}

		file::File *getFile(rpc::Connection *connection)
		{
			return connection -> file.get();
		}

		void release(rpc::Connection *connection)
		{
			connection -> file.reset(nullptr);
			response.clear();
		}

		void add(const string &key, int(*func)(json::Object &, json::Object &))
		{
			methodMapper[key] = func;
		}
	protected:
		map<string, int(*)(json::Object &, json::Object &)> methodMapper;
		json::Object request;
		json::Object response;
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
		//	throw 
			if(request.getMethod() == "")
			{
				int ec = request.parseFrom(text);
				if(ec < 0)
				{
					LOGGER(ls::INFO) << "parse failed..." << ls::endl;
					return ec;
				}
			}
			if(request.getMethod() == "GET")
				return ec;
			LOGGER(ls::INFO) << "request with body..." << ls::endl;
			auto contentLength = request.getAttribute("Content-Length");
			if(contentLength == "")
			{
				request.setDefaultHeader();
				request.getMethod() = "GET";
				request.getURL() = "/error?code=411";
				return ec;
			}
			int len = stoi(contentLength);
		//	throw
			text = in.split(ec, len);
			if(ec < 0)
				return Exception::LS_ENOCOMPLETE;
			request.getBody() = text;
			return ec;
		}

		void error(const string &code)
		{
			response.setResponseLine(code, request.getVersion());
			response.setHeaderByRequest(request);
			response.setStringBody(code, "text/plain");
		}

		int exec(rpc::Connection *connection) override
		{
			LOGGER(ls::INFO) << "map method" << ls::endl;
			auto uri = http::Url(request.getURL()).uri;
			LOGGER(ls::INFO) << ls::endl;
			auto method = methodMapper[request.getMethod()].find(uri);
			LOGGER(ls::INFO) << ls::endl;
			response.reset(&connection -> sendBuffer);
			LOGGER(ls::INFO) << ls::endl;
			if(method != methodMapper[request.getMethod()].end())
			{
			LOGGER(ls::INFO) << ls::endl;
				method -> second(request, response);
			LOGGER(ls::INFO) << ls::endl;
			}
			else
				error("404");
			LOGGER(ls::INFO) << ls::endl;
			putString(connection);
			LOGGER(ls::INFO) << ls::endl;
			return Exception::LS_OK;
		}

		void putString(rpc::Connection *connection)
		{
			connection -> file.reset(nullptr);
		}

		void putFile(rpc::Connection *connection)
		{
			connection -> file.reset(response.getFile());
		}

		void release(rpc::Connection *connection) override
		{
			connection -> file.reset(nullptr);
			request.clear();
			response.clear();
		}
		
		file::File *getFile(rpc::Connection *connection)
		{
			return connection -> file.get();
		}
	
		void add(const string &method, const string &key, int(*func)(http::Request &, http::Response &))
		{
			methodMapper[method][key] = func;
		}
	protected:
		map<string, map<string, int(*)(http::Request &, http::Response &)>> methodMapper;
		http::Request request;
		http::Response response;
};

json::Object json_error(int code)
{
	json::Object response;
	json::api.push(response, "code", code);
	json::api.push(response, "message", Exception(code).message());
	json::Object data;
 	json::api.push(response, "data", data);
	return response;;
}

int hello_json(json::Object &request, json::Object &response)
{
	int ec;
	string name;
	json::Object data;
	if((ec = json::api.get(request, "name", name)) == 0)
		json::api.push(data, "name", name);
	else
		return Exception::LS_EFORMAT;
	json::api.push(response, "code", ec);
	json::api.push(response, "message", Exception(ec).message());
	json::api.push(response, "data", data);
	return Exception::LS_OK;
}

int post_http(http::Request &request, http::Response &response, int(*callback)(json::Object&, json::Object&))
{
	int ec;
	auto text = request.getBody();
	json::Object jsonRequest = json::api.decode(ec, text);
	json::Object jsonResponse;
	if(ec < 0 || (ec = callback(jsonRequest, jsonResponse)) < 0)
		jsonResponse = json_error(ec);
	response.setResponseLine("200", request.getVersion());
	response.setHeaderByRequest(request);
	response.setStringBody(jsonResponse.toString(), "application/json");
	return Exception::LS_OK;
}

int get_http(http::Request &request, http::Response &response, int(*callback)(json::Object &, json::Object &))
{
	int ec;
	http::QueryString qs;
	auto queryText = http::Url(request.getURL()).queryText;
	string name;
	json::Object jsonResponse;
	if(queryText == "" || qs.parseFrom(queryText) < 0 || (name = qs.getParameter("name")) == "")
		jsonResponse = json_error(Exception::LS_EFORMAT);
	else
	{
		json::Object jsonRequest;
		json::api.push(jsonRequest, "name", name);
		hello_json(jsonRequest, jsonResponse);
	}	
	response.setResponseLine("200", request.getVersion());
	response.setHeaderByRequest(request);
	response.setStringBody(jsonResponse.toString(), "application/json");
	return Exception::LS_OK;
}

int post_hello_http(http::Request &request, http::Response &response)
{
	return post_http(request, response, hello_json);
}

int get_hello_http(http::Request &request, http::Response &response)
{
	return get_http(request, response, hello_json);
}

int main()
{
	JsonProtocol *jp = new JsonProtocol("json", 8081);
	HttpProtocol *hp = new HttpProtocol("http", 8083);
	jp -> add("hello_json", hello_json);
	hp -> add("POST", "/hello_http", post_hello_http);
	hp -> add("GET", "/hello_http", get_hello_http);
	
	CASQueueFactory casqf;
	rpc::Tool tool(casqf);
	tool.push(jp);
	tool.push(hp);
	tool.run();
	return 0;
}
