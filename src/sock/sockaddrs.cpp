#include "los/sockaddrs.h"

#if defined(WIN32) || defined(_WINDLL)
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "sock/sockaddr4.h"
#include "sock/sockaddr6.h"
#include "los/logs.h"

namespace los {
namespace sockaddrs {

std::shared_ptr<SockaddrInterface> CreateSockaddr(const char *host, uint16_t port, bool is_local)
{
    std::shared_ptr<SockaddrInterface> h = nullptr;
    auto type = GetIpType(host);
    switch (type)
    {
    case kIpv4:
        h = std::make_shared<Sockaddr4>(host, port, is_local);
        break;
    case kIpv6:
        h = std::make_shared<Sockaddr6>(host, port, is_local);
        break;
    default:
        std::string port_str = std::to_string(port);
        struct addrinfo hints = { 0 };
        struct addrinfo *res = NULL;
        if (0 != getaddrinfo(host, port_str.c_str(), &hints, &res))
        {
            los::logs::Printfln("getaddrinfo fail! host=%s, port=%hu, error=%d", host, port, los::socks::GetLastErrorCode());
            break;
        }

        for (struct addrinfo *node = res; node; node = node->ai_next)
        {
            if (AF_INET == node->ai_family)
            {
                h = std::make_shared<Sockaddr4>(reinterpret_cast<sockaddr_in *>(node->ai_addr), is_local);
                break;
            }
            else if (AF_INET6 == res->ai_family)
            {
                h = std::make_shared<Sockaddr6>(reinterpret_cast<sockaddr_in6 *>(node->ai_addr), is_local);
                break;
            }
        }
        freeaddrinfo(res);
        break;
    }

    return std::move(h);
}

std::shared_ptr<SockaddrInterface> Accept(int fd, int &remote_fd)
{
    sockaddr_storage remote_addr = { 0 };
    socklen_t remote_addr_len = sizeof(sockaddr_storage);
    remote_fd = static_cast<int>(accept(fd, reinterpret_cast<sockaddr *>(&remote_addr), &remote_addr_len));
    if (remote_fd < 0)
    {
        return nullptr;
    }

    std::shared_ptr<SockaddrInterface> h = nullptr;
    if (AF_INET == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr4>(reinterpret_cast<sockaddr_in *>(&remote_addr), false);
    }
    else if (AF_INET6 == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr6>(reinterpret_cast<sockaddr_in6 *>(&remote_addr), false);
    }
    else
    {
        return nullptr;
    }

    return std::move(h);
}

std::shared_ptr<SockaddrInterface> RecvFrom(int fd, void *buf, int &len)
{
    sockaddr_storage remote_addr = { 0 };
    socklen_t remote_addr_len = sizeof(sockaddr_storage);
    int recv_len = recvfrom(fd, static_cast<char *>(buf), len, 0, reinterpret_cast<struct sockaddr *>(&remote_addr), &remote_addr_len);
    if (recv_len <= 0)
    {
        return nullptr;
    }

    std::shared_ptr<SockaddrInterface> h = nullptr;
    if (AF_INET == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr4>(reinterpret_cast<sockaddr_in *>(&remote_addr), false);
    }
    else if (AF_INET6 == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr6>(reinterpret_cast<sockaddr_in6 *>(&remote_addr), false);
    }
    else
    {
        return nullptr;
    }

    return std::move(h);
}

std::shared_ptr<SockaddrInterface> Getsockname(int fd)
{
    sockaddr_storage remote_addr = { 0 };
    socklen_t remote_addr_len = sizeof(sockaddr_storage);
    if (getsockname(fd, reinterpret_cast<sockaddr *>(&remote_addr), &remote_addr_len) < 0)
    {
        return nullptr;
    }

    std::shared_ptr<SockaddrInterface> h = nullptr;
    if (AF_INET == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr4>(reinterpret_cast<sockaddr_in *>(&remote_addr), true);
    }
    else if (AF_INET6 == remote_addr.ss_family)
    {
        h = std::make_shared<Sockaddr6>(reinterpret_cast<sockaddr_in6 *>(&remote_addr), true);
    }
    else
    {
        return nullptr;
    }

    return std::move(h);
}

Types GetIpType(const char *ip)
{
    // Ipv6为128位地址，需要16字节大小储存
    uint8_t buf[16] = { 0 };
    if (inet_pton(AF_INET, ip, buf) > 0)
    {
        return kIpv4;
    }
    else if (inet_pton(AF_INET6, ip, buf) > 0)
    {
        return kIpv6;
    }

    return kUnknown;
}

}   // namespace sockaddrs
}   // namespace los
