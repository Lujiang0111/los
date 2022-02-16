#include "sock/sockaddr4.h"

#include <string.h>
#include <assert.h>

#if defined(WIN32) || defined(_WINDLL)
#include <ws2ipdef.h>
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

Sockaddr4::Sockaddr4(const char *ip, uint16_t port, bool is_local) : if_num_(0)
{
    assert(ip);
    ip_ = ip;
    port_ = port;

    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip);
    addr_.sin_port = htons(port);

    if (is_local)
    {
        SetLocalArgs();
    }
}

Sockaddr4::Sockaddr4(sockaddr_in *p_addr, bool is_local) : if_num_(0)
{
    memcpy(&addr_, p_addr, sizeof(addr_));
    char ip[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &addr_.sin_addr, ip, INET_ADDRSTRLEN);
    ip_ = ip;
    port_ = ntohs(addr_.sin_port);

    if (is_local)
    {
        SetLocalArgs();
    }
}

Sockaddr4::~Sockaddr4()
{

}

bool Sockaddr4::JoinMulticastGroup(int fd, SockaddrInterface *group_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    if (!group_addr4)
    {
        los::logs::Printfln("Wrong type between group addr and local addr");
        return false;
    }

    struct ip_mreq mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_ADD_MEMBERSHIP fail! group ip=%s, local ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::DropMulticastGroup(int fd, SockaddrInterface *group_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    if (!group_addr4)
    {
        los::logs::Printfln("Wrong type between group addr and local addr");
        return false;
    }

    struct ip_mreq mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_DROP_MEMBERSHIP fail! group ip=%s, local ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::AddMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    Sockaddr4 *source_addr4 = dynamic_cast<Sockaddr4 *>(source_addr);
    if ((!group_addr4) || (!source_addr4))
    {
        los::logs::Printfln("Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr4->addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_ADD_SOURCE_MEMBERSHIP fail! group ip=%s, local ip=%s, source ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), source_addr4->ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::DropMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    Sockaddr4 *source_addr4 = dynamic_cast<Sockaddr4 *>(source_addr);
    if ((!group_addr4) || (!source_addr4))
    {
        los::logs::Printfln("Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr4->addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_DROP_SOURCE_MEMBERSHIP fail! group ip=%s, local ip=%s, source ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), source_addr4->ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::BlockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    Sockaddr4 *source_addr4 = dynamic_cast<Sockaddr4 *>(source_addr);
    if ((!group_addr4) || (!source_addr4))
    {
        los::logs::Printfln("Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr4->addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_BLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_BLOCK_SOURCE fail! group ip=%s, local ip=%s, source ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), source_addr4->ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::UnblockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr)
{
    Sockaddr4 *group_addr4 = dynamic_cast<Sockaddr4 *>(group_addr);
    Sockaddr4 *source_addr4 = dynamic_cast<Sockaddr4 *>(source_addr);
    if ((!group_addr4) || (!source_addr4))
    {
        los::logs::Printfln("Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_addr4->addr_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = addr_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr4->addr_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_UNBLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        los::logs::Printfln("IP_UNBLOCK_SOURCE fail! group ip=%s, local ip=%s, source ip=%s, error=%d",
            group_addr4->ip_.c_str(), ip_.c_str(), source_addr4->ip_.c_str(), los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::UdpBind(int fd, SockaddrInterface *local_addr, bool is_recv)
{
    Sockaddr4 *local_addr4 = dynamic_cast<Sockaddr4 *>(local_addr);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&on), sizeof(on));

#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&on), sizeof(on));
#endif

    if (is_recv)
    {
#if defined (WIN32) || defined (_WINDLL)
        if ((IsMulticast()) && (local_addr4))
        {
            if (bind(fd, reinterpret_cast<const struct sockaddr *>(&local_addr4->addr_), sizeof(local_addr4->addr_)) < 0)
            {
                los::logs::Printfln("bind fail! local ip=%s, local port=%hu, error=%d", local_addr4->ip_.c_str(), local_addr4->port_, los::socks::GetLastErrorCode());
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

        if ((local_addr4) && (local_addr4->if_name_.length() > 0))
        {
            if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, local_addr4->if_name_.c_str(), local_addr4->if_name_.size()) < 0)
            {
                los::logs::Printfln("SO_BINDTODEVICE fail! local ip=%s, interface=%s, error=%d",
                    local_addr4->ip_.c_str(), local_addr4->if_name_.c_str(), los::socks::GetLastErrorCode());
                return false;
            }
        }
#endif
    }
    else
    {
        if (local_addr4)
        {
            if (bind(fd, reinterpret_cast<const struct sockaddr *>(&local_addr4->addr_), sizeof(local_addr4->addr_)) < 0)
            {
                los::logs::Printfln("bind fail! local ip=%s, local port=%hu, error=%d", local_addr4->ip_.c_str(), local_addr4->port_, los::socks::GetLastErrorCode());
                return false;
            }

            if (IsMulticast())
            {
                if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<const char *>(&local_addr4->addr_.sin_addr), sizeof(local_addr4->addr_.sin_addr)) < 0)
                {
                    los::logs::Printfln("IP_MULTICAST_IF fail! local ip=%s, interface=%s, error=%d",
                        local_addr4->ip_.c_str(), local_addr4->if_name_.c_str(), los::socks::GetLastErrorCode());
                    return false;
                }
            }
        }
    }

    return true;
}

bool Sockaddr4::Bind(int fd)
{
    if (bind(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
    {
        los::logs::Printfln("bind fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

bool Sockaddr4::Connect(int fd)
{
    if (connect(fd, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_)) < 0)
    {
        los::logs::Printfln("connect fail! ip=%s, port=%hu, error=%d", ip_.c_str(), port_, los::socks::GetLastErrorCode());
        return false;
    }

    return true;
}

int Sockaddr4::Sendto(int fd, const void *buf, int len)
{
    if (0 == len)
    {
        return 0;
    }

    return sendto(fd, static_cast<const char *>(buf), len, 0, reinterpret_cast<const struct sockaddr *>(&addr_), sizeof(addr_));
}

void *Sockaddr4::GetNative()
{
    return &addr_;
}

bool Sockaddr4::IsMulticast() const
{
    return IN_MULTICAST(ntohl(addr_.sin_addr.s_addr));
}

const char *Sockaddr4::GetIp() const
{
    return ip_.c_str();
}

uint16_t Sockaddr4::GetPort() const
{
    return port_;
}

const char *Sockaddr4::GetInterfaceName() const
{
    return if_name_.c_str();
}

Types Sockaddr4::GetType() const
{
    return kIpv4;
}

static int Ipv4Cmp(struct sockaddr_in *lhs, struct sockaddr_in *rhs)
{
    for (int index = 0; index < 4; index++)
    {
        if ((reinterpret_cast<uint8_t *>(&(lhs->sin_addr.s_addr)))[index] > (reinterpret_cast<uint8_t *>(&(rhs->sin_addr.s_addr)))[index])
        {
            return 1;
        }
        else if ((reinterpret_cast<uint8_t *>(&(lhs->sin_addr.s_addr)))[index] < (reinterpret_cast<uint8_t *>(&(rhs->sin_addr.s_addr)))[index])
        {
            return -1;
        }
    }

    return 0;
}

void Sockaddr4::SetLocalArgs()
{
#if defined(WIN32) || defined(_WINDLL)
#else
    struct ifaddrs *ifa = nullptr;
    getifaddrs(&ifa);
    for (struct ifaddrs *node = ifa; node; node = node->ifa_next)
    {
        if ((nullptr != node->ifa_addr) && (AF_INET == node->ifa_addr->sa_family))
        {
            if (0 == Ipv4Cmp(&addr_, reinterpret_cast<struct sockaddr_in *>(node->ifa_addr)))
            {
                if_name_ = (node->ifa_name) ? node->ifa_name : "";
                if_num_ = if_nametoindex(node->ifa_name);
                break;
            }
        }
    }
    freeifaddrs(ifa);
#endif
}

}   // namespace sockaddrs
}   // namespace los
