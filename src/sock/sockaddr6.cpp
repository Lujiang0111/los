#include "sock/sockaddr6.h"

#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif

#include "los/logs.h"

namespace los {
namespace sockaddrs {

Sockaddr6::Sockaddr6(const char *ip, uint16_t port, bool is_local) : if_num_(0), scope_id_(0)
{
    assert(ip);
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, addr_.sin6_addr.s6_addr);
    addr_.sin6_port = htons(port);

    char ip_buf[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &addr_.sin6_addr, ip_buf, INET6_ADDRSTRLEN);
    ip_ = ip_buf;
    port_ = ntohs(addr_.sin6_port);

    if (is_local)
    {
        SetLocalArgs();
    }
}

Sockaddr6::Sockaddr6(sockaddr_in6 *p_addr, bool is_local) : if_num_(0), scope_id_(0)
{
    memcpy(&addr_, p_addr, sizeof(addr_));
    char ip_buf[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &addr_.sin6_addr, ip_buf, INET6_ADDRSTRLEN);
    ip_ = ip_buf;
    port_ = ntohs(addr_.sin6_port);

    if (is_local)
    {
        SetLocalArgs();
    }
}

Sockaddr6::~Sockaddr6()
{
}

static int Ipv6Cmp(struct sockaddr_in6 *lhs, struct sockaddr_in6 *rhs)
{
    for (int index = 0; index < 16; index++)
    {
        if ((reinterpret_cast<uint8_t *>(&(lhs->sin6_addr)))[index] > (reinterpret_cast<uint8_t *>(&(rhs->sin6_addr)))[index])
        {
            return 1;
        }
        else if ((reinterpret_cast<uint8_t *>(&(lhs->sin6_addr)))[index] < (reinterpret_cast<uint8_t *>(&(rhs->sin6_addr)))[index])
        {
            return -1;
        }
    }
    return 0;
}

int Sockaddr6::Compare(SockaddrInterface *rhs)
{
    Sockaddr6 *rhs6 = dynamic_cast<Sockaddr6 *>(rhs);
    if (!rhs6)
    {
        los::logs::Printfln("Wrong type between group addr and local addr");
        return 2;
    }

    return Ipv6Cmp(&addr_, &rhs6->addr_);
}

void Sockaddr6::IpIncrease()
{
    for (int idx = 15; (idx >= 0) && (0 == ++reinterpret_cast<uint8_t *>(&addr_.sin6_addr)[idx]); --idx);
    char ip_buf[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &addr_.sin6_addr, ip_buf, INET6_ADDRSTRLEN);
    ip_ = ip_buf;
}

void Sockaddr6::IpDecrease()
{
    for (int idx = 15; (idx >= 0) && (0xff == --reinterpret_cast<uint8_t *>(&addr_.sin6_addr)[idx]); --idx);
    char ip_buf[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &addr_.sin6_addr, ip_buf, INET6_ADDRSTRLEN);
    ip_ = ip_buf;
}

bool Sockaddr6::JoinMulticastGroup(int fd, SockaddrInterface *group_addr)
{
    Sockaddr6 *group_addr6 = dynamic_cast<Sockaddr6 *>(group_addr);
    if (!group_addr6)
    {
        los::logs::Printfln("Wrong type between group addr and local addr");
        return false;
    }

    struct ipv6_mreq mreq6;
    mreq6.ipv6mr_multiaddr = group_addr6->addr_.sin6_addr;
    mreq6.ipv6mr_interface = if_num_;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
    {
        los::logs::Printfln("IPV6_ADD_MEMBERSHIP fail! group ip=%s, local ip=%s, error=%d",
            group_addr6->ip_.c_str(), ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr6::DropMulticastGroup(int fd, SockaddrInterface *group_addr)
{
    Sockaddr6 *group_addr6 = dynamic_cast<Sockaddr6 *>(group_addr);
    if (!group_addr6)
    {
        los::logs::Printfln("Wrong type between group addr and local addr");
        return false;
    }

    struct ipv6_mreq mreq6;
    mreq6.ipv6mr_multiaddr = group_addr6->addr_.sin6_addr;
    mreq6.ipv6mr_interface = if_num_;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
    {
        los::logs::Printfln("IPV6_ADD_MEMBERSHIP fail! group ip=%s, local ip=%s, error=%d",
            group_addr6->ip_.c_str(), ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr6::AddMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    // centos暂时没找到实现
    return true;
}

bool Sockaddr6::DropMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    // centos暂时没找到实现
    return true;
}

bool Sockaddr6::BlockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    // centos暂时没找到实现
    return true;
}

bool Sockaddr6::UnblockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    // centos暂时没找到实现
    return true;
}

bool Sockaddr6::UdpBind(int fd, SockaddrInterface *local_addr, bool is_recv)
{
    Sockaddr6 *local_addr6 = dynamic_cast<Sockaddr6 *>(local_addr);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&on), sizeof(on));

#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&on), sizeof(on));
#endif

    if (is_recv)
    {
#if defined(_WIN32)
        if ((IsMulticast()) && (local_addr6))
        {
            if (bind(fd, reinterpret_cast<const struct sockaddr *>(&local_addr6->addr_), sizeof(local_addr6->addr_) < 0))
            {
                los::logs::Printfln("bind fail! local ip=%s, local port=%hu, error=%d", local_addr6->ip_.c_str(), local_addr6->port_, los::socks::GetLastErrorCode());
                return false;
            }
        }
        else
        {
            if (bind(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
            {
                los::logs::Printfln("bind fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
                return false;
            }
        }
#else
        if (bind(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
        {
            los::logs::Printfln("bind fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
            return false;
        }
#endif
    }
    else
    {
        if (IsMulticast() && (local_addr6))
        {
            if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<const char *>(&local_addr6->if_num_), sizeof(local_addr6->if_num_)) < 0)
            {
                los::logs::Printfln("IPV6_MULTICAST_IF fail! local ip=%s, if num=%d, error=%d", local_addr6->ip_.c_str(), local_addr6->if_num_, los::socks::GetLastErrorCode());
                return false;
            }
        }
    }

    if (local_addr6)
    {
        scope_id_ = local_addr6->scope_id_;
    }

    return true;
}

bool Sockaddr6::Bind(int fd)
{
    if (bind(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
    {
        los::logs::Printfln("bind fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr6::Connect(int fd)
{
    if (connect(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
    {
        los::logs::Printfln("connect fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

int Sockaddr6::Sendto(int fd, const void *buf, int len)
{
    if (0 == len)
    {
        return 0;
    }

    return sendto(fd, static_cast<const char *>(buf), len, 0, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_));
}

void *Sockaddr6::GetNative()
{
    return &addr_;
}

bool Sockaddr6::IsMulticast() const
{
    return IN6_IS_ADDR_MULTICAST(&(addr_.sin6_addr));
}

const char *Sockaddr6::GetIp() const
{
    return ip_.c_str();
}

uint16_t Sockaddr6::GetPort() const
{
    return port_;
}

const char *Sockaddr6::GetInterfaceName() const
{
    return if_name_.c_str();
}

Types Sockaddr6::GetType() const
{
    return kIpv6;
}

void Sockaddr6::SetLocalArgs()
{
#if defined(_WIN32)
    int fd = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
    Connect(fd);
    sockaddr_storage sock_addr;
    socklen_t sock_addr_len = sizeof(sockaddr_storage);
    if (0 == getsockname(fd, (struct sockaddr *)&sock_addr, &sock_addr_len))
    {
        scope_id_ = (reinterpret_cast<struct sockaddr_in6 *>(&sock_addr))->sin6_scope_id;
        addr_.sin6_scope_id = scope_id_;
    }
    closesocket(fd);
#else
    struct ifaddrs *ifa = NULL;
    getifaddrs(&ifa);
    for (struct ifaddrs *node = ifa; node; node = node->ifa_next)
    {
        if ((nullptr != node->ifa_addr) &&
            (AF_INET6 == node->ifa_addr->sa_family) &&
            (0 == Ipv6Cmp(&addr_, reinterpret_cast<struct sockaddr_in6 *>(node->ifa_addr))))
        {
            if_name_ = (node->ifa_name) ? node->ifa_name : "";
            if_num_ = if_nametoindex(node->ifa_name);
            scope_id_ = (reinterpret_cast<struct sockaddr_in6 *>(node->ifa_addr))->sin6_scope_id;
            addr_.sin6_scope_id = scope_id_;
            break;
        }
    }
    freeifaddrs(ifa);
#endif
}

}   // namespace sockaddrs
}   // namespace los
