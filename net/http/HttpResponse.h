#pragma once

#include <string>
#include <unordered_map>

class Buffer;

enum class HttpStatusCode
{
    kUnknown = 0,
    k200OK = 200,
    k201Created = 201,
    k204NoContent = 204,
    k301MovedPermanently = 301,
    k302Found = 302,
    k304NotModified = 304,
    k400BadRequest = 400,
    k401Unauthorized = 401,
    k403Forbidden = 403,
    k404NotFound = 404,
    k405MethodNotAllowed = 405,
    k429TooManyRequests = 429,
    k500InternalServerError = 500,
    k502BadGateway = 502,
    k503ServiceUnavailable = 503,
    k504GatewayTimeout = 504,
};

class HttpResponse
{
public:
    HttpResponse();
    // ~HttpResponse();

    void setStatusCode(HttpStatusCode statusCode) { statusCode_ = statusCode; }
    void setStatusMessage(const std::string &message) { statusMessage_ = message; }
    void setVersion(const std::string &version) { version_ = version; }
    void setHeaders(const std::unordered_map<std::string, std::string> &headers) { headers_ = headers; }
    void addHeaders(const std::string &key, const std::string &value) { headers_.insert_or_assign(key, value); }
    void setBody(std::string body) { body_ = std::move(body); }
    void setCloseConnection(bool close) { closeConnection_ = close; }

    void writeToBuffer(Buffer *output);

private:
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    bool closeConnection_;
};
