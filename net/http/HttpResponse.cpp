#include "HttpResponse.h"
#include "Buffer.h"

HttpResponse::HttpResponse()
    : closeConnection_(false),
      statusCode_(HttpStatusCode::kUnknown)
{
}

void HttpResponse::writeToBuffer(Buffer *output)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %d ", version_.c_str(), static_cast<int>(statusCode_));
    output->append(buf, strlen(buf));
    output->append(statusMessage_);
    output->append("\r\n");

    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.length());
        output->append(buf, strlen(buf));
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}