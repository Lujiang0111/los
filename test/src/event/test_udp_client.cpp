#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#define closesocket(x)  close(x)
#endif

#include "test_event.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
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

class UdpClient
{
public:
    UdpClient();
    ~UdpClient();

    bool Init(int argc, char **argv);
    void Run();

private:
    bool InitSocket();
    void WorkThread();

    static void HandlerCallbackEntry(void *priv_data, int trigger_events);
    void HandlerCallback(int trigger_events);

private:
    los::events::MultiplexTypes multiplex_type_;
    std::string dst_ip_;
    uint16_t dst_port_;
    std::string local_ip_;

    int send_fd_;
    std::shared_ptr<los::sockaddrs::ISockaddr> dst_addr_;
    std::shared_ptr<los::events::IIo> io_;
    std::vector<char> recv_buf_;
    std::deque<std::string> send_msgs_cur_;

    bool has_send_str_;
    std::deque<std::string> send_msgs_store_;
    std::mutex send_mutex_;

    std::thread work_thread_;
};

UdpClient::UdpClient() :
    multiplex_type_(los::events::MultiplexTypes::kAuto),
    dst_port_(0),
    send_fd_(-1),
    recv_buf_(kRecvBufSize),
    has_send_str_(false)
{
    los::socks::GlobalInit();
}

UdpClient::~UdpClient()
{
    if (work_thread_.joinable())
    {
        work_thread_.join();
    }

    if (send_fd_ >= 0)
    {
        closesocket(send_fd_);
        send_fd_ = -1;
    }

    los::socks::GlobalDeinit();
}

bool UdpClient::Init(int argc, char **argv)
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
        dst_ip_ = argv[3];
    }
    else
    {
        los::logs::Printf("Input dst ip:");
        std::cin >> dst_ip_;
    }

    if (argc >= 5)
    {
        dst_port_ = static_cast<uint16_t>(atoi(argv[4]));
    }
    else
    {
        los::logs::Printf("Input dst port:");
        std::cin >> dst_port_;
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

bool UdpClient::InitSocket()
{
    // 创建发送fd
    switch (los::sockaddrs::GetIpType(dst_ip_.c_str()))
    {
    case los::sockaddrs::kIpv4:
        send_fd_ = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
        break;
    case los::sockaddrs::kIpv6:
        send_fd_ = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
        break;
    default:
        break;
    }

    if (send_fd_ < 0)
    {
        los::logs::Printfln("create fd fail! ip=%s, port=%hu", dst_ip_.c_str(), dst_port_);
        return false;
    }

    // 设置socket收发缓冲区
    int opt = 1 << 24;  // 16MB
    setsockopt(send_fd_, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));
    setsockopt(send_fd_, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));

    // 设置为非阻塞模式
    los::socks::SetBlockMode(send_fd_, false);

    // 创建目的地址
    dst_addr_ = los::sockaddrs::CreateSockaddr(dst_ip_.c_str(), dst_port_, false);
    if (!dst_addr_)
    {
        los::logs::Printfln("create dst sockaddr fail! ip=%s, port=%hu", dst_ip_.c_str(), dst_port_);
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

    if (!dst_addr_->UdpBind(send_fd_, (local_addr) ? local_addr.get() : nullptr, false))
    {
        los::logs::Printfln("bind local sockaddr fail! fd=%d, local ip=%s, local port=%hu, err=%d",
            send_fd_, local_ip_.c_str(), local_port, los::socks::GetLastErrorCode());
        return false;
    }

    io_ = los::events::CreateIo(100, multiplex_type_);
    io_->RegisterHandler(send_fd_, &UdpClient::HandlerCallbackEntry, this, los::events::kRead);

    los::logs::Printfln("Work thread start! dst=%s:%hu, local=%s:%hu", dst_ip_.c_str(), dst_port_, local_ip_.c_str(), local_port);
    work_thread_ = std::thread(&UdpClient::WorkThread, this);
    return true;
}

void UdpClient::WorkThread()
{
    while (b_app_start)
    {
        if ((0 == send_msgs_cur_.size()) && (has_send_str_))
        {
            std::lock_guard<std::mutex> lock(send_mutex_);
            send_msgs_cur_.swap(send_msgs_store_);
            has_send_str_ = false;
            io_->EnableEvent(send_fd_, los::events::kWrite);
        }

        if (io_->Execute() < 0)
        {
            break;
        }
    }
    los::logs::Printfln("Work thread stop!");
}

void UdpClient::HandlerCallbackEntry(void *priv_data, int trigger_events)
{
    UdpClient *h = static_cast<UdpClient *>(priv_data);
    return h->HandlerCallback(trigger_events);
}

void UdpClient::HandlerCallback(int trigger_events)
{
    if (trigger_events & los::events::kRead)
    {
        int recv_len = recv(send_fd_, &recv_buf_[0], kRecvBufSize, 0);
        if (recv_len > 0)
        {
            recv_buf_[recv_len] = 0;
            los::logs::Printfln("recv %d bytes: %s", recv_len, &recv_buf_[0]);
        }
    }

    if (trigger_events & los::events::kWrite)
    {
        if (send_msgs_cur_.size() > 0)
        {
            dst_addr_->Sendto(send_fd_, send_msgs_cur_.front().c_str(), static_cast<int>(send_msgs_cur_.front().length()));
            send_msgs_cur_.pop_front();
        }
        else
        {
            io_->DisableEvent(send_fd_, los::events::kWrite);
        }
    }
}

void UdpClient::Run()
{
    std::string msg;
    while (b_app_start)
    {
        los::logs::Printfln("Input message:");
        std::cin >> msg;

        {
            std::lock_guard<std::mutex> lock(send_mutex_);
            send_msgs_store_.push_back(msg);
            has_send_str_ = true;
        }
    }
}

void TestUdpClient(int argc, char **argv)
{
    std::shared_ptr<UdpClient> h = std::make_shared<UdpClient>();
    if (!h->Init(argc, argv))
    {
        return;
    }

    h->Run();
}
