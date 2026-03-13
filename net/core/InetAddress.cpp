#include "InetAddress.h"

#include <string.h>
#include <strings.h>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr) : addr_(addr) {}

std::string InetAddress::toIP() const
{
    char buffer[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
    return buffer;
}

std::string InetAddress::toIpPort() const
{
    char buffer[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
    size_t end = strlen(buffer);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buffer + end, ":%u", port);
    return buffer;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

const sockaddr_in *InetAddress::getSockAddr() const { return &addr_; }

// #include <iostream>

// int main(void)
// {
//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;
//     return 0;
// }