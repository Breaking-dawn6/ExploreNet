#pragma once

#include "llhttp.h"
#include "Logger.h"

#include <string>
#include <unordered_map>

struct HttpRequest
{
    std::string method;
    std::string url;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpContext
{
public:
    HttpContext();
    HttpContext(const HttpContext &) = delete;
    HttpContext &operator=(const HttpContext &) = delete;
    // ~HttpContext();
    bool parse(const char *data, size_t len);
    bool isComplete() { return parseIsComplete_; }
    void reset();

    HttpRequest &request() { return request_; }

private:
    llhttp_t parser_;
    llhttp_settings_t settings_;
    HttpRequest request_;
    std::string headerField_; // 解析时先解析字段名，再解析字段值，因为Tcp报文可能分包，所以要先保存解析到的字段名，后续组成完成字段
    bool parseIsComplete_;

    // 回调函数，分别负责在url、字段名、字段值解析完成时将数据填入request,在整体解析完成时更新状态
    static int url_complete(llhttp_t *parser, const char *data, size_t length);
    static int header_field_complete(llhttp_t *parser, const char *data, size_t length);
    static int header_value_complete(llhttp_t *parser, const char *data, size_t length);
    static int message_complete_callback(llhttp_t *parser);
};
