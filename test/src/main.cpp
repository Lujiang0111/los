#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <map>
#include <string>

#include "test_file.h"
#include "test_log.h"
#include "test_util.h"
#include "test_socket.h"
#include "test_event.h"

enum class TestTypes
{
    kTestTypeListIngFiles = 0,
    kTestTypeCreateDir,
    kTestTypeRemoveFile,
    kTestSingleThreadLogging,
    kTestMultiThreadLogging,
    kTestSleepTime,
    kTestMultiThreadSleepTime,
    kTestSocketInCrease,
    kTestUdpClient,
    kTestUdpServer,
};

static constexpr struct TestTypeMaps
{
    TestTypes type;
    const char *detail;
}kTestTypeMaps[] = 
{
    {TestTypes::kTestTypeListIngFiles, "Test listing files"},
    {TestTypes::kTestTypeCreateDir, "Test create directory"},
    {TestTypes::kTestTypeRemoveFile, "Test remove file"},
    {TestTypes::kTestSingleThreadLogging, "Test single thread logging"},
    {TestTypes::kTestMultiThreadLogging, "Test multi thread logging"},
    {TestTypes::kTestSleepTime, "Test Sleep Time"},
    {TestTypes::kTestMultiThreadSleepTime, "Test Multithread Sleep Time"},
    {TestTypes::kTestSocketInCrease, "Test socket addr increase and decrease"},
    {TestTypes::kTestUdpClient, "Test udp client"},
    {TestTypes::kTestUdpServer, "Test udp server"},
};

bool b_app_start = true;

static void SigIntHandler(int sig_num)
{
    signal(SIGINT, SigIntHandler);
    b_app_start = false;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SigIntHandler);

    int test_no = 0;
    if (argc >= 2)
    {
        test_no = atoi(argv[1]);
    }
    else
    {
        printf("\nTest type list:\n");
        for (auto &&x : kTestTypeMaps)
        {
            printf("%d: %s\n", x.type, x.detail);
        }

        printf("\nInput test type:");
        scanf("%d", &test_no);
    }

    switch (static_cast<TestTypes>(test_no))
    {
    case TestTypes::kTestTypeListIngFiles:
        TestListingFiles(argc, argv);
        break;
    case TestTypes::kTestTypeCreateDir:
        TestCreateDir(argc, argv);
        break;
    case TestTypes::kTestTypeRemoveFile:
        TestRemoveFile(argc, argv);
        break;
    case TestTypes::kTestSingleThreadLogging:
        TestSingleThreadLogging(argc, argv);
        break;
    case TestTypes::kTestMultiThreadLogging:
        TestMultiThreadLogging(argc, argv);
        break;
    case TestTypes::kTestSleepTime:
        TestSleepTime(argc, argv);
        break;
    case TestTypes::kTestMultiThreadSleepTime:
        TestMultiThreadSleepTime(argc, argv);
        break;
    case TestTypes::kTestSocketInCrease:
        TestSocketIncrease(argc, argv);
        break;
    case TestTypes::kTestUdpClient:
        TestUdpClient(argc, argv);
        break;
    case TestTypes::kTestUdpServer:
        TestUdpServer(argc, argv);
        break;
    default:
        printf("Unspecified test type!\n");
        break;
    }

    return 0;
}
