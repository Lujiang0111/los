#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#define closesocket(x)  close(x)
#endif

#include "test_event.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "los/events.h"
#include "los/sockaddrs.h"
#include "los/logs.h"

constexpr int kRecvBufSize = 65536;

extern bool b_app_start;

constexpr struct kMultiplexTypeMaps
{
    los::events::MultiplexTypes type;
    const char *detail;
}kTestTypeMaps[] =
{
    {los::events::MultiplexTypes::kAuto, "auto"},
    {los::events::MultiplexTypes::kEpoll, "epoll"},
    {los::events::MultiplexTypes::kSelect, "select"},
};

class UdpServer
{
public:
    UdpServer();
    ~UdpServer();

    bool Init(int argc, char **argv);
    void Run();

private:
    bool InitSocket();

    static void HandlerCallbackEntry(void *priv_data, int trigger_events);
    void HandlerCallback(int trigger_events);

private:
    los::events::MultiplexTypes multiplex_type_;
    std::string source_ip_;
    uint16_t source_port_;
    std::string local_ip_;

    int recv_fd_;
    std::shared_ptr<los::events::IIo> io_;
    std::vector<char> recv_buf_;
};

UdpServer::UdpServer() :
    multiplex_type_(los::events::MultiplexTypes::kAuto),
    source_port_(0),
    recv_fd_(-1),
    recv_buf_(kRecvBufSize)
{
    los::socks::GlobalInit();
}

UdpServer::~UdpServer()
{
    if (recv_fd_ > 0)
    {
        closesocket(recv_fd_);
        recv_fd_ = -1;
    }

    los::socks::GlobalDeinit();
}

bool UdpServer::Init(int argc, char **argv)
{
    int int_val = 0;
    if (argc >= 3)
    {
        multiplex_type_ = static_cast<los::events::MultiplexTypes>(atoi(argv[2]));
    }
    else
    {
        los::logs::Printf("\nTest type list:\n");
        for (auto &&x : kTestTypeMaps)
        {
            los::logs::Printf("%d: %s\n", x.type, x.detail);
        }

        los::logs::Printf("\nInput test type:");
        std::cin >> int_val;
        multiplex_type_ = static_cast<los::events::MultiplexTypes>(int_val);
    }

    if (argc >= 4)
    {
        source_ip_ = argv[3];
    }
    else
    {
        los::logs::Printf("Input source ip:");
        std::cin >> source_ip_;
    }

    if (argc >= 5)
    {
        source_port_ = static_cast<uint16_t>(atoi(argv[4]));
    }
    else
    {
        los::logs::Printf("Input source port:");
        std::cin >> source_port_;
    }

    if (argc >= 6)
    {
        local_ip_ = argv[5];
    }
    else
    {
        los::logs::Printf("Input local ip:");
        std::cin >> local_ip_;
    }

    if (!InitSocket())
    {
        return false;
    }

    return true;
}

void UdpServer::Run()
{
    while (b_app_start)
    {
        if (io_->Execute() < 0)
        {
            break;
        }
    }
}

bool UdpServer::InitSocket()
{
    // 创建接收fd
    switch (los::sockaddrs::GetIpType(source_ip_.c_str()))
    {
    case los::sockaddrs::kIpv4:
        recv_fd_ = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
        break;
    case los::sockaddrs::kIpv6:
        recv_fd_ = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
        break;
    default:
        break;
    }

    if (recv_fd_ < 0)
    {
        los::logs::Printfln("create fd fail! ip=%s, port=%hu", source_ip_.c_str(), source_port_);
        return false;
    }

    // 设置socket收发缓冲区
    int opt = 1 << 24;  // 16MB
    setsockopt(recv_fd_, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));
    setsockopt(recv_fd_, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));

    // 设置为非阻塞模式
    los::socks::SetBlockMode(recv_fd_, false);

    // 创建目的地址
    auto source_addr = los::sockaddrs::CreateSockaddr(source_ip_.c_str(), source_port_, false);
    if (!source_addr)
    {
        los::logs::Printfln("create source sockaddr fail! ip=%s, port=%hu", source_ip_.c_str(), source_port_);
        return false;
    }

    // 创建本机地址
    uint16_t local_port = 0;
    std::shared_ptr<los::sockaddrs::ISockaddr> local_addr = nullptr;
    if (local_ip_.length() > 0)
    {
        local_addr = los::sockaddrs::CreateSockaddr(local_ip_.c_str(), local_port, true);
        if (!local_addr)
        {
            los::logs::Printfln("create local sockaddr fail! local ip=%s, local port=%hu", local_ip_.c_str(), local_port);
            return false;
        }
    }

    if (!source_addr->UdpBind(recv_fd_, (local_addr) ? local_addr.get() : nullptr, true))
    {
        los::logs::Printfln("bind local sockaddr fail! fd=%d, local ip=%s, local port=%hu, err=%d",
            recv_fd_, local_ip_.c_str(), local_port, los::socks::GetLastErrorCode());
        return false;
    }

    if (source_addr->IsMulticast())
    {
        if (!local_addr)
        {
            los::logs::Printfln("No local addr with multicast!");
            return false;
        }

        if (!local_addr->JoinMulticastGroup(recv_fd_, source_addr.get()))
        {
            los::logs::Printfln("Join multicast group fail! fd=%d, ip=%s, port=%hu, local ip=%s, err=%d",
                recv_fd_, source_ip_.c_str(), source_port_, local_ip_.c_str(), los::socks::GetLastErrorCode());
            return false;
        }
    }

    io_ = los::events::CreateIo(100, multiplex_type_);
    io_->RegisterHandler(recv_fd_, &UdpServer::HandlerCallbackEntry, this, los::events::kRead);

    los::logs::Printfln("Recv start! source=%s:%hu, local=%s:%hu", source_ip_.c_str(), source_port_, local_ip_.c_str(), local_port);
    return true;
}

void UdpServer::HandlerCallbackEntry(void *priv_data, int trigger_events)
{
    UdpServer *h = static_cast<UdpServer *>(priv_data);
    return h->HandlerCallback(trigger_events);
}

void UdpServer::HandlerCallback(int trigger_events)
{
    if (trigger_events & los::events::kRead)
    {
        int recv_len = kRecvBufSize;
        auto remote_addr = los::sockaddrs::RecvFrom(recv_fd_, &recv_buf_[0], recv_len);
        if (recv_len > 0)
        {
            recv_buf_[recv_len] = 0;
            los::logs::Printfln("recv %d bytes: %s", recv_len, &recv_buf_[0]);
            remote_addr->Sendto(recv_fd_, &recv_buf_[0], recv_len);
        }
    }
}

void TestUdpServer(int argc, char **argv)
{
    std::shared_ptr<UdpServer> h = std::make_shared<UdpServer>();
    if (!h->Init(argc, argv))
    {
        return;
    }

    h->Run();
}
