#include "los/socks.h"

#if defined(WIN32) || defined(_WINDLL)
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <mstcpip.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#endif

#include "los/logs.h"

namespace los {
namespace socks {

void GlobalInit()
{
#if defined(WIN32) || defined(_WINDLL)
    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    WORD wVersionRequested = MAKEWORD(2, 2);

    WSADATA wsaData;
    int ret_val = WSAStartup(wVersionRequested, &wsaData);
    if (ret_val != 0)
    {
        // Tell the user that we could not find a usable Winsock DLL
        los::logs::Printfln("WSAStartup failed with error: %d\n", ret_val);
        return;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        // Tell the user that we could not find a usable Winsock DLL
        los::logs::Printfln("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return;
    }
#else
#endif
}

void GlobalDeinit()
{
#if defined(WIN32) || defined(_WINDLL)
    WSACleanup();
#else
#endif
}

int GetLastErrorCode()
{
#if defined(WIN32) || defined(_WINDLL)
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool SetBlockMode(int fd, bool is_block)
{
#if defined(WIN32) || defined(_WINDLL)
    u_long argp = (is_block) ? 0 : 1;
    return (0 == ioctlsocket(fd, FIONBIO, &argp));
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags)
    {
        los::logs::Printfln("Unable to get fd mode, error:%d\n", GetLastErrorCode());
        return false;
    }

    flags = (is_block) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (0 == fcntl(fd, F_SETFL, flags));
#endif
}

bool SetTtl(int fd, int ttl)
{
    setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char *)&ttl, sizeof(ttl));
    return true;
}

bool SetKeepAlive(int fd, int timeout_ms)
{
#if defined(WIN32) || defined(_WINDLL)
    tcp_keepalive alive;
    if (0 == timeout_ms)
    {
        alive.onoff = 0;
    }
    else
    {
        alive.keepalivetime = timeout_ms / 2;            // 一半超时时间后进入探测
        alive.keepaliveinterval = timeout_ms / 20;       // 系统默认探测次数为10次,所以时间为一半超时时间的1/10
        alive.onoff = 1;
}

    DWORD ulBytesReturn = 0;
    if (SOCKET_ERROR == WSAIoctl(fd, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), nullptr, 0, &ulBytesReturn, nullptr, nullptr))
    {
        los::logs::Printfln("WSAIoctl failed with error:%d\n", GetLastErrorCode());
        return false;
    }

    return true;
#else
    // linux下只能按秒设置心跳，并且最小值为2s
    int timeout = timeout_ms / 1000;
    if (timeout < 2)
    {
        int val = 0;
        if (0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&val), sizeof val))
        {
            los::logs::Printfln("Unable to disable SO_KEEPALIVE, error:%d\n", GetLastErrorCode());
            return false;
        }
    }
    else
    {
        int val = 1;
        if (0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&val), sizeof val))
        {
            los::logs::Printfln("Unable to enable SO_KEEPALIVE, error:%d\n", GetLastErrorCode());
            return false;
        }

        // 一半时间后进入keepalive状态
        val = timeout - timeout / 2;
        if (0 != setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, reinterpret_cast<const char *>(&val), sizeof val))
        {
            los::logs::Printfln("Unable to set TCP_KEEPIDLE, use system defalut, error:%d", GetLastErrorCode());
            return false;
        }

        // keepalive状态后每1s检测一次心跳
        val = 1;
        if (0 != setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, reinterpret_cast<const char *>(&val), sizeof val))
        {
            los::logs::Printfln("Unable to set TCP_KEEPINTVL, use system defalut, error:%d", GetLastErrorCode());
            return false;
        }

        // 检测次数为一半时间/检测间隔
        val = timeout / 2;
        if (0 != setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, reinterpret_cast<const char *>(&val), sizeof val))
        {
            los::logs::Printfln("Unable to set TCP_KEEPCNT, use system defalut, error:%d", GetLastErrorCode());
            return false;
        }
    }

    return true;
#endif
}

}   // namespace socks
}   // namespace los
