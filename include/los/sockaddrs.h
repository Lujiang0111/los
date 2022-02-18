#ifndef LOS_INCLUDE_LOS_SOCKADDRS_H_
#define LOS_INCLUDE_LOS_SOCKADDRS_H_

#include <memory>

#include "los/socks.h"

namespace los {
namespace sockaddrs {

enum Types
{
    kUnknown = 0,
    kIpv4,
    kIpv6,
};

class LOS_API SockaddrInterface
{
public:
    virtual ~SockaddrInterface() = default;

    /***************************************************************************//**
    * 比较
    * rhs           [in]        比较地址
    * @note     this指针指向的是被比较的地址
    * @return   >0  this > rhs
    *           =0  this = rhs
    *           <0  this < rhs
     ******************************************************************************/
    virtual int Compare(SockaddrInterface *rhs) = 0;

    /***************************************************************************//**
    * Ip地址自增
     ******************************************************************************/
    virtual void IpIncrease() = 0;

    /***************************************************************************//**
    * Ip地址自减
     ******************************************************************************/
    virtual void IpDecrease() = 0;

    /***************************************************************************//**
    * 本机地址加入组播组
    * group_addr    [in]    组播地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool JoinMulticastGroup(int fd, SockaddrInterface *group_addr) = 0;

    /***************************************************************************//**
    * 本机地址离开组播组
    * group_addr    [in]    组播地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool DropMulticastGroup(int fd, SockaddrInterface *group_addr) = 0;

    /***************************************************************************//**
    * 组播组添加源(白名单）
    * group_addr    [in]    组播地址
    * source_addr   [in]    源地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool AddMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr) = 0;

    /***************************************************************************//**
    * 组播组删除源（白名单）
    * group_addr    [in]    组播地址
    * source_addr   [in]    源地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool DropMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr) = 0;

    /***************************************************************************//**
    * 组播组锁定源(黑名单）
    * group_addr    [in]    组播地址
    * source_addr   [in]    源地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool BlockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr) = 0;

    /***************************************************************************//**
    * 组播组解锁源（白名单）
    * group_addr    [in]    组播地址
    * source_addr   [in]    源地址
    * @note     this指针指向的是对应的本机网卡地址
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool UnblockMulticastSource(int fd, SockaddrInterface *group_addr, SockaddrInterface *source_addr) = 0;

    /***************************************************************************//**
    * Udp发送/接收前设置，执行必须的bind函数
    * fd            [in]    套接字
    * local_addr    [in]    本机网卡地址，若为空则不绑定网卡
    * is_recv       [in]    是否用作接收，true为接收，false为发送
    * @note         this指针指向的是目的地址
    * @return       true/false  成功/失败
     ******************************************************************************/
    virtual bool UdpBind(int fd, SockaddrInterface *local_addr, bool is_recv) = 0;

    /***************************************************************************//**
    * socket bind封装
    * fd        [in]    套接字
    * @return   true/false  成功/失败
    * @note     存粹的bind封装，一般用Udp/TCPSetup即可
     ******************************************************************************/
    virtual bool Bind(int fd) = 0;

    /***************************************************************************//**
    * socket connect封装
    * fd        [in]    套接字
    * @return   true/false  成功/失败
     ******************************************************************************/
    virtual bool Connect(int fd) = 0;

    /***************************************************************************//**
    * socket sendto封装
    * fd        [in]    套接字
    * buf       [in]    发送缓冲区
    * len       [in]    缓冲区字节数
    * @return   同sendto()返回
     ******************************************************************************/
    virtual int Sendto(int fd, const void *buf, int len) = 0;

    /***************************************************************************//**
    * 获取原生的sockaddr *句柄
    * @note     注意不要修改ip，端口等参数
    * @return   sockaddr *句柄
     ******************************************************************************/
    virtual void *GetNative() = 0;

    /***************************************************************************//**
    * 判断地址是否为组播地址
     ******************************************************************************/
    virtual bool IsMulticast() const = 0;

    /***************************************************************************//**
    * 获取Ip地址
    * @return   Ip地址
     ******************************************************************************/
    virtual const char *GetIp() const = 0;

    /***************************************************************************//**
    * 获取端口
    * @return   端口
     ******************************************************************************/
    virtual uint16_t GetPort() const = 0;

    /***************************************************************************//**
    * 获取网卡名
    * @note     仅限local地址使用
    * @return   网卡名
     ******************************************************************************/
    virtual const char *GetInterfaceName() const = 0;

    /***************************************************************************//**
    * 获取地址类型
    * @return  地址类型
     ******************************************************************************/
    virtual Types GetType() const = 0;
};

/***************************************************************************//**
* 创建一个sockaddr
* host      [in]    ip/域名
* port      [in]    端口
* is_local  [in]    是否为本机地址
* @return   nullptr 创建失败
*           other   sockaddr句柄     
 ******************************************************************************/
LOS_API std::shared_ptr<SockaddrInterface> CreateSockaddr(const char *host, uint16_t port, bool is_local);

/***************************************************************************//**
* accept()封装
* fd        [in]    套接字
* remote_fd [out]   accept()返回的对端fd
* @return   对端地址句柄
 ******************************************************************************/
LOS_API std::shared_ptr<SockaddrInterface> Accept(int fd, int &remote_fd);

/***************************************************************************//**
* recvfrom()封装
* fd        [in]        套接字
* buf       [in]        接收缓冲区
* len       [in/out]    输入为缓冲区最大字节数，输出为接收字节数
* @return   recvfrom()返回的对端地址句柄
 ******************************************************************************/
LOS_API std::shared_ptr<SockaddrInterface> RecvFrom(int fd, void *buf, int &len);

/***************************************************************************//**
* getsockname()封装
* fd        [in]    套接字
* @return   getsockname()返回的对端地址句柄
 ******************************************************************************/
LOS_API std::shared_ptr<SockaddrInterface> Getsockname(int fd);

/***************************************************************************//**
* 获取ip地址类型
* ip        [in]    ip地址
* @return  地址类型
 ******************************************************************************/
LOS_API Types GetIpType(const char *ip);

}   // namespace sockaddrs
}   // namespace los

#endif // !LOS_INCLUDE_LOS_SOCKADDRS_H_
