﻿#ifndef LOS_INTERNAL_SOCK_SOCKADDR4_H_
#define LOS_INTERNAL_SOCK_SOCKADDR4_H_

#include <string>

#if defined(WIN32) || defined(_WINDLL)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include "los/sockaddrs.h"

namespace los {
namespace sockaddrs {

class Sockaddr4 : public SockaddrInterface
{
public:
    Sockaddr4(const char *ip, uint16_t port, bool is_local);
    Sockaddr4(sockaddr_in *p_addr, bool is_local);
    virtual ~Sockaddr4();

    virtual bool JoinMulticastGroup(int fd, SockaddrInterface *group_addr);

    virtual bool DropMulticastGroup(int fd, SockaddrInterface *group_addr);

    virtual bool AddMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr);

    virtual bool DropMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr);

    virtual bool BlockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr);

    virtual bool UnblockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr);

    virtual bool UdpBind(int fd, SockaddrInterface *local_addr, bool is_recv);

    virtual bool Bind(int fd);

    virtual bool Connect(int fd);

    virtual int Sendto(int fd, const void *buf, int len);

    virtual void *GetNative();

    virtual bool isMulticast() const;

    virtual const char *GetIp() const;

    virtual uint16_t GetPort() const;

    virtual const char *GetInterfaceName() const;

    virtual Types GetType() const;

private:
    void SetLocalArgs();

private:
    sockaddr_in addr_;
    std::string ip_;
    uint16_t port_;
    std::string if_name_;
    int if_num_;
};

}   // namespace sockaddrs
}   // namespace los

#endif // !LOS_INTERNAL_SOCK_SOCKADDR4_H_