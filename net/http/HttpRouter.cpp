#include "HttpRouter.h"
#include "HttpContext.h"
#include "HttpResponse.h"

HttpRouter::HttpRouter()
{
    defaultHandler_ = [](HttpRequest req, ResponseSender sender)
    {
        HttpResponse res = HttpResponse();
        res.setStatusCode(HttpStatusCode::k404NotFound);
        res.setStatusMessage("Not Found");
        res.setBody("404 Not Found");
        sender(res);
    };
}
void HttpRouter::GET(const std::string &url, HttpHandler handler)
{
    router_["GET"].insert_or_assign(url, std::move(handler));
}
void HttpRouter::POST(const std::string &url, HttpHandler handler)
{
    router_["POST"].insert_or_assign(url, std::move(handler));
}

bool HttpRouter::hasHandler(const std::string &method, const std::string &url)
{
    auto methodIt = router_.find(method);
    if (methodIt != router_.end())
    {
        return methodIt->second.find(url) != methodIt->second.end();
    }
    return false;
}

void HttpRouter::execute(HttpRequest request, ResponseSender responseSender)
{

    auto methodIt = router_.find(request.method);
    if (methodIt != router_.end())
    {
        auto pathIt = methodIt->second.find(request.url);
        if (pathIt != methodIt->second.end())
        {
            pathIt->second(std::move(request), std::move(responseSender));
            return;
        }
    }
    defaultHandler_(std::move(request), std::move(responseSender));
}