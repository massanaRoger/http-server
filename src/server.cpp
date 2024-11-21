#include "server.h"

HttpServer& HttpServer::getInstance() {
	static HttpServer instance;
	return instance;
}

void HttpServer::registerRoute(Method method, const std::string& path, Handler handler) {
	RequestKey req = { path, method };
	m_Routes[req] = handler;
}

void HttpServer::registerRoute(Method method, std::string&& path, Handler handler) {
	RequestKey req = { std::move(path), method };
	m_Routes[req] = handler;
}

void HttpServer::addRequest(const RequestKey& requestKey) {
	m_RequestQueue.push(requestKey);
}

void HttpServer::addRequest(RequestKey&& requestKey) {
	m_RequestQueue.push(std::move(requestKey));
}

RouteRegistrar::RouteRegistrar(const Method method, const std::string& path, HttpServer::Handler handler) {
	HttpServer::getInstance().registerRoute(method, std::move(path), std::move(handler));
}
