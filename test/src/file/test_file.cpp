#include "test_file.h"

#include <inttypes.h>
#include <vector>
#include <string>

#include "los/files.h"

using FileInfoInterface = los::files::FileInfoInterface;

static constexpr struct FileSortMaps
{
    los::files::Sorts sort_type;
    const char *detail;
}kTestTypeMaps[] =
{
    {los::files::Sorts::kNone, "Do not sort"},
    {los::files::Sorts::kByName, "Sort by name"},
    {los::files::Sorts::kByModifyTime, "Sort by modfify time"},
};

static void PresentFileInfo(const FileInfoInterface *file_info)
{
    if (file_info)
    {
        printf("name:%s size:%zu modify time:%lld\n",
            file_info->GetFullName(), file_info->GetTotalSize(), file_info->GetModifyTimestamp());

        for (size_t i = 0; i < file_info->GetChildsSize(); ++i)
        {
            PresentFileInfo(file_info->GetChild(i));
        }
    }
}

void TestListingFiles(int argc, char **argv)
{
    std::vector<char> buf(65536);
    std::string name;
    if (argc >= 3)
    {
        name = argv[2];
    }
    else
    {
        printf("Input file name:");
        scanf("%s", &buf[0]);
        name = &buf[0];
    }

    los::files::Sorts file_sort = los::files::Sorts::kNone;
    if (argc >= 4)
    {
        file_sort = static_cast<los::files::Sorts>(atoi(argv[3]));
    }
    else
    {
        printf("\nSort type list:\n");
        for (auto &&x : kTestTypeMaps)
        {
            printf("%d: %s\n", x.sort_type, x.detail);
        }
        printf("Input file sort type:");
        scanf("%d", reinterpret_cast<int *>(&file_sort));
    }

    auto file_info = los::files::GetFileInfo(name.c_str(), file_sort);
    PresentFileInfo(file_info.get());
}

void TestCreateDir(int argc, char **argv)
{
    std::vector<char> buf(65536);
    std::string name;
    if (argc >= 3)
    {
        name = argv[2];
    }
    else
    {
        printf("Input dir name:");
        scanf("%s", &buf[0]);
        name = &buf[0];
    }

    los::files::CreateDir(name.c_str(), 0);
}

void TestRemoveFile(int argc, char **argv)
{
    std::vector<char> buf(65536);
    std::string name;
    if (argc >= 3)
    {
        name = argv[2];
    }
    else
    {
        printf("Input file name:");
        scanf("%s", &buf[0]);
        name = &buf[0];
    }

    los::files::RemoveFile(name.c_str());
}
