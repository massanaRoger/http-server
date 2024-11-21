#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <cstddef>
#include <queue>
#include <utility>

enum class Method {
	GET,
	POST,
	PUT,
	PATCH,
	DELETE
};

class HttpServer {
public:
	using Handler = std::function<void()>;
	
	struct RequestKey {
		std::string url;
		Method method;
	};

	struct RouteKeyHash {
		size_t operator()(const RequestKey& key) const {
			return std::hash<std::string>()(key.url) ^ std::hash<Method>()(key.method);
		}
	};

	static HttpServer& getInstance() {
		static HttpServer instance;
		return instance;
	}

	void registerRoute(Method method, const std::string& path, Handler handler) {
		RequestKey req = { path, method };
		m_Routes[req] = handler;
	}

	void registerRoute(Method method, std::string&& path, Handler handler) {
		RequestKey req = { std::move(path), method };
		m_Routes[req] = handler;
	}

	void addRequest(const RequestKey& requestKey) {
		m_RequestQueue.push(requestKey);
	}

	void addRequest(RequestKey&& requestKey) {
		m_RequestQueue.push(std::move(requestKey));
	}

private:
	HttpServer() = default;
	HttpServer(const HttpServer&) = delete;
	HttpServer& operator=(const HttpServer&) = delete;

	std::unordered_map<RequestKey, Handler, RouteKeyHash> m_Routes;
	std::queue<RequestKey> m_RequestQueue;
};

class RouteRegistrar {
public:
	RouteRegistrar(const Method method, const std::string& path, HttpServer::Handler handler) {
		HttpServer::getInstance().registerRoute(method, std::move(path), std::move(handler));
	}
};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define GET(path, handler) \
	static RouteRegistrar MACRO_CONCAT(registrar, __LINE__)(Method::GET, path, handler)

#define POST(path, handler) \
	static RouteRegistrar MACRO_CONCAT(registrar, __LINE__)(Method::POST, path, handler)


