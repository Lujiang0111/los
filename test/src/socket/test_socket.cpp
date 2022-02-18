#include "test_socket.h"

#include <string>
#include <iostream>

#include "los/sockaddrs.h"

void TestSocketIncrease(int argc, char **argv)
{
    std::string ip;
    if (argc >= 3)
    {
        ip = argv[2];
    }
    else
    {
        printf("Input ip:");
        std::cin >> ip;
    }

    auto addr = los::sockaddrs::CreateSockaddr(ip.c_str(), 0, false);
    addr->IpIncrease();
    std::cout << "Increase: " << addr->GetIp() << std::endl;
    addr->IpDecrease();
    addr->IpDecrease();
    std::cout << "Decrease: " << addr->GetIp() << std::endl;
}
