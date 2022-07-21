#ifndef LOS_INTERNAL_SOCK_SOCKADDR4_H_
#define LOS_INTERNAL_SOCK_SOCKADDR4_H_

#include <string>

#if defined(_WIN32)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include "los/sockaddrs.h"

namespace los {
namespace sockaddrs {

class Sockaddr4 : public ISockaddr
{
public:
    Sockaddr4(const char *ip, uint16_t port, bool is_local);
    Sockaddr4(sockaddr_in *p_addr, bool is_local);
    virtual ~Sockaddr4();

    virtual int Compare(ISockaddr *rhs);

    virtual void IpIncrease();

    virtual void IpDecrease();

    virtual bool JoinMulticastGroup(int fd, ISockaddr *group_addr);

    virtual bool DropMulticastGroup(int fd, ISockaddr *group_addr);

    virtual bool AddMulticastSource(int fd, ISockaddr *group_addr, ISockaddr *source_addr);

    virtual bool DropMulticastSource(int fd, ISockaddr *group_addr, ISockaddr *source_addr);

    virtual bool BlockMulticastSource(int fd, ISockaddr *group_addr, ISockaddr *source_addr);

    virtual bool UnblockMulticastSource(int fd, ISockaddr *group_addr, ISockaddr *source_addr);

    virtual bool UdpBind(int fd, ISockaddr *local_addr, bool is_recv);

    virtual bool Bind(int fd);

    virtual bool Connect(int fd);

    virtual int Sendto(int fd, const void *buf, int len);

    virtual void *GetNative();

    virtual bool IsMulticast() const;

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
