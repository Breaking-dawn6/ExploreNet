#include "HttpContext.h"

HttpContext::HttpContext()
{
    llhttp_settings_init(&settings_);

    settings_.on_url = url_complete;
    settings_.on_header_field = header_field_complete;
    settings_.on_header_value = header_value_complete;
    settings_.on_message_complete = message_complete_callback;

    llhttp_init(&parser_, HTTP_BOTH, &settings_);
    parser_.data = this;
    parseIsComplete_ = false;
}

bool HttpContext::parse(const char *data, size_t len)
{
    llhttp_errno_t err = llhttp_execute(&parser_, data, len);

    if (err != HPE_OK)
    {
        LOG_ERROR("Parse error : %s", llhttp_get_error_reason(&parser_));
        return false;
    }
    return true;
}

void HttpContext::reset()
{
    llhttp_reset(&parser_);
    request_ = HttpRequest();
    parseIsComplete_ = false;
    headerField_.clear();
}

int HttpContext::url_complete(llhttp_t *parser, const char *data, size_t length)
{
    HttpContext *context = static_cast<HttpContext *>(parser->data);

    context->request_.url.assign(data, length);
    context->request_.method = llhttp_method_name((llhttp_method_t)parser->method);

    return 0;
}

int HttpContext::header_field_complete(llhttp_t *parser, const char *data, size_t length)
{
    HttpContext *context = static_cast<HttpContext *>(parser->data);

    context->headerField_.assign(data, length);

    return 0;
}

int HttpContext::header_value_complete(llhttp_t *parser, const char *data, size_t length)
{
    HttpContext *context = static_cast<HttpContext *>(parser->data);

    context->request_.headers.insert({context->headerField_, std::string(data, length)});

    return 0;
}

int HttpContext::message_complete_callback(llhttp_t *parser)
{
    HttpContext *context = static_cast<HttpContext *>(parser->data);
    context->parseIsComplete_ = true;
    return 0;
}