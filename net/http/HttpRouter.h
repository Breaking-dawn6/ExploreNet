#pragma once

#include <unordered_map>
#include <string>
#include <functional>

class HttpResponse;
class HttpRequest;

using HttpHandler = std::function<void(HttpRequest, HttpResponse &)>;
using Router = std::unordered_map<std::string, std::unordered_map<std::string, HttpHandler>>;

class HttpRouter
{

public:
    HttpRouter();
    void GET(const std::string &url, HttpHandler handler);
    void POST(const std::string &url, HttpHandler handler);

    bool hasHandler(const std::string &method, const std::string &url);

    void execute(HttpRequest request, HttpResponse &response);

    void setDefaultHandler(HttpHandler handler) { defaultHandler_ = std::move(handler); }

private:
    Router router_;
    HttpHandler defaultHandler_;
};