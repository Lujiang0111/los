#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <map>
#include <string>

#include "test_file.h"
#include "test_log.h"
#include "test_util.h"

enum TestTypes
{
    kTestTypeListIngFiles = 0,
    kTestTypeCreateDir,
    kTestTypeRemoveFile,
    kTestSingleThreadLogging,
    kTestMultiThreadLogging,
    kTestSleepTime,
    kTestMultiThreadSleepTime,
};

static constexpr struct TestTypeMaps
{
    TestTypes type;
    const char *detail;
}kTestTypeMaps[] = 
{
    {kTestTypeListIngFiles, "Test listing files"},
    {kTestTypeCreateDir, "Test create directory"},
    {kTestTypeRemoveFile, "Test remove file"},
    {kTestSingleThreadLogging, "Test single thread logging"},
    {kTestMultiThreadLogging, "Test multi thread logging"},
    {kTestSleepTime, "Test Sleep Time"},
    {kTestMultiThreadSleepTime, "Test Multithread Sleep Time"},
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
    case kTestTypeListIngFiles:
        TestListingFiles(argc, argv);
        break;
    case kTestTypeCreateDir:
        TestCreateDir(argc, argv);
        break;
    case kTestTypeRemoveFile:
        TestRemoveFile(argc, argv);
        break;
    case kTestSingleThreadLogging:
        TestSingleThreadLogging(argc, argv);
        break;
    case kTestMultiThreadLogging:
        TestMultiThreadLogging(argc, argv);
        break;
    case kTestSleepTime:
        TestSleepTime(argc, argv);
        break;
    case kTestMultiThreadSleepTime:
        TestMultiThreadSleepTime(argc, argv);
        break;
    default:
        printf("Unspecified test type!\n");
        break;
    }

    return 0;
}
