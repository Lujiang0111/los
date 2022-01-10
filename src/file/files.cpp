#include "file/file_info.h"

#include <vector>

#if defined(WIN32) || defined(_WINDLL)
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "fmt/format.h"

namespace los {
namespace files {

#if defined(WIN32) || defined(_WINDLL)
static void GetFileInfoTraverse(FileInfo *parent)
{
    // Prepare string for use with FindFile functions.  First, copy the
    // string to a buffer, then append '\*' to the directory name.
    std::string search_name = parent->full_name_ + "\\*";

    // Find the first file in the directory.
    WIN32_FIND_DATA ffd;
    HANDLE file_handle = FindFirstFile(search_name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return;
    }

    do
    {
        // Find first file will always return "." and ".." as the first two directories.
        if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
        {
            auto node = std::make_shared<FileInfo>();
            node->name_ = ffd.cFileName;
            node->full_name_ = fmt::format("{}{}{}", parent->full_name_, kDirSep, ffd.cFileName);
            node->total_size_ = (static_cast<size_t>(ffd.nFileSizeHigh) << 32) | static_cast<size_t>(ffd.nFileSizeLow);

            uint64_t totalUs = ((static_cast<uint64_t>(ffd.ftLastWriteTime.dwHighDateTime) << 32) |
                (static_cast<uint64_t>(ffd.ftLastWriteTime.dwLowDateTime))) / 10 - 11644473600000000ull;
            node->modify_timestamp_ = static_cast<int64_t>(totalUs / 1000000);

            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                node->mode_ = Modes::kDirectory;
                GetFileInfoTraverse(node.get());
            }
            else
            {
                node->mode_ = Modes::kRegular;
            }

            parent->total_size_ += node->total_size_;
            parent->childs_.push_back(std::move(node));
        }
    } while (FindNextFile(file_handle, &ffd)); //Find the next file.
    FindClose(file_handle);
}

static std::shared_ptr<FileInfo> GetFileInfoWindows(const std::string &name)
{
    WIN32_FIND_DATA ffd;
    HANDLE file_handle = FindFirstFile(name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return nullptr;
    }

    auto node = std::make_shared<FileInfo>();
    node->name_ = ffd.cFileName;
    node->full_name_ = name;
    node->total_size_ = (static_cast<size_t>(ffd.nFileSizeHigh) << 32) | static_cast<size_t>(ffd.nFileSizeLow);

    uint64_t totalUs = ((static_cast<uint64_t>(ffd.ftLastWriteTime.dwHighDateTime) << 32) |
        (static_cast<uint64_t>(ffd.ftLastWriteTime.dwLowDateTime))) / 10 - 11644473600000000ull;
    node->modify_timestamp_ = static_cast<int64_t>(totalUs / 1000000);

    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        node->mode_ = Modes::kDirectory;
        GetFileInfoTraverse(node.get());
    }
    else
    {
        node->mode_ = Modes::kRegular;
    }
    return node;
}

#else
static std::shared_ptr<FileInfo> GetFileInfoLinux(const std::string &name)
{
    struct stat file_stat;
    if (0 != lstat(name.c_str(), &file_stat))
    {
        fmt::print("lstat file [{}] fail! errno={}\n", name, strerror(errno));
        return nullptr;
    }

    std::shared_ptr<FileInfo> node = std::make_shared<FileInfo>();
    const char *p_split = strrchr(name.c_str(), kDirSep);
    if (p_split)
    {
        node->name_ = (p_split + 1);
    }
    else
    {
        node->name_ = name;
    }

    node->full_name_ = name;
    node->total_size_ = file_stat.st_size;
    node->modify_timestamp_ = file_stat.st_ctime;

    if (S_ISDIR(file_stat.st_mode))
    {
        node->mode_ = Modes::kDirectory;

        DIR *dir = opendir(name.c_str());
        if (!dir)
        {
            fmt::print("open dir [{}] fail! errno={}\n", name, strerror(errno));
            return nullptr;
        }

        dirent *p_dirent = nullptr;
        while (nullptr != (p_dirent = readdir(dir)))
        {
            if ((p_dirent->d_name) && (0 != strcmp(p_dirent->d_name, ".")) && (0 != strcmp(p_dirent->d_name, "..")))
            {
                std::string full_name = fmt::format("{}{}{}", name, kDirSep, p_dirent->d_name);
                auto child = GetFileInfoLinux(full_name);
                node->total_size_ += child->total_size_;
                node->childs_.push_back(std::move(child));
            }
        }
        closedir(dir);
    }
    else
    {
        node->mode_ = Modes::kRegular;
    }

    return node;
}
#endif


void SortFileInfo(FileInfo *file_info , Sorts sort_type)
{
    if ((!file_info) || (Sorts::kNone == sort_type))
    {
        return;
    }

    // sort subdir first
    for (auto &&child : file_info->childs_)
    {
        SortFileInfo(child.get(), sort_type);
    }

    // sort
    std::sort(file_info->childs_.begin(), file_info->childs_.end(),
        [sort_type](const std::shared_ptr<FileInfo> &lhs, const std::shared_ptr<FileInfo> &rhs) {
            switch (sort_type)
            {
            case Sorts::kByName:
                return (lhs->name_.compare(rhs->name_) < 0);
            case Sorts::kByModifyTime:
                return (lhs->modify_timestamp_ < rhs->modify_timestamp_);
            default:
                break;
            }
            return true;
        });
}

std::shared_ptr<FileInfoInterface> GetFileInfo(const char *name, Sorts sort_type)
{
    if (!name)
    {
        fmt::print("LGetFileInfo : name not exist!\n");
        return nullptr;
    }

    std::string file_name = name;
    AdjustFilePath(file_name);

#if defined(WIN32) || defined(_WINDLL)
    auto file_info = GetFileInfoWindows(file_name);
#else
    auto file_info = GetFileInfoLinux(file_name);
#endif

    SortFileInfo(file_info.get(), sort_type);
    return file_info;
}

static inline bool mkdir_inline(const std::string &name)
{
#if defined(WIN32) || defined(_WINDLL)
    return ((0 == mkdir(name.c_str())) || (EEXIST == errno));
#else
    return ((mkdir(name.c_str(), mode_t(0755)) == 0) || (EEXIST == errno));
#endif
}

bool CreateDir(const char *name, bool create_parent_only)
{
    if (!name)
    {
        return false;
    }

    std::string file_name = name;
    AdjustFilePath(file_name);

    size_t search_offset = 0;
#if defined(WIN32) || defined(_WINDLL)
    if ((file_name.length() >= 3) && (':' == file_name[1]) && (kDirSep == file_name[2]))
    {
        search_offset = 3;
    }
#else
    if (kDirSep == file_name[0])
    {
        search_offset = 1;
    }
#endif

    do
    {
        auto token_pos = file_name.find_first_of(kDirSep, search_offset);
        // treat the entire path as a folder if no folder separator not found
        if (std::string::npos == token_pos)
        {
            if (create_parent_only)
            {
                break;
            }

            token_pos = file_name.length();
        }

        auto subdir = file_name.substr(0, token_pos);
        if ((!subdir.empty()) && (!mkdir_inline(subdir)))
        {
            fmt::print("create dir [{}] fail! errno={}\n", subdir, strerror(errno));
            return false;
        }
        search_offset = token_pos + 1;
    } while (search_offset < file_name.length());

    return true;
}

static void RemoveFileTraverse(const std::string &name, bool is_top)
{
#if defined(WIN32) || defined(_WINDLL)
    std::string search_name = name;
    if (!is_top)
    {
        // Prepare string for use with FindFile functions.  First, copy the
        // string to a buffer, then append '\*' to the directory name.
        search_name += "\\*";
    }

    // Find the first file in the directory.
    WIN32_FIND_DATA ffd;
    HANDLE file_handle = FindFirstFile(search_name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return;
    }

    do
    {
        // Find first file will always return "." and ".." as the first two directories.
        if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
        {
            std::string full_name = (is_top) ? fmt::format("{}", name) : fmt::format("{}{}{}", name, kDirSep, ffd.cFileName);
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                RemoveFileTraverse(full_name, false);
                rmdir(full_name.c_str());
            }
            else
            {
                remove(full_name.c_str());
            }
        }
    } while (FindNextFile(file_handle, &ffd)); //Find the next file.
    FindClose(file_handle);
#else
    struct stat file_stat;
    if (0 != lstat(name.c_str(), &file_stat))
    {
        return;
    }

    if (S_ISDIR(file_stat.st_mode))
    {
        DIR *dir = opendir(name.c_str());
        if (!dir)
        {
            return;
        }

        dirent *p_dirent = nullptr;
        while (nullptr != (p_dirent = readdir(dir)))
        {
            if ((p_dirent->d_name) && (0 != strcmp(p_dirent->d_name, ".")) && (0 != strcmp(p_dirent->d_name, "..")))
            {
                std::string full_name = fmt::format("{}{}{}", name, kDirSep, p_dirent->d_name);
                RemoveFileTraverse(full_name, false);
            }
        }
        closedir(dir);

        rmdir(name.c_str());
    }
    else
    {
        remove(name.c_str());
    }
#endif
}

bool RemoveFile(const char *name)
{
    std::string file_name = name;
    AdjustFilePath(file_name);

    RemoveFileTraverse(file_name, true);
    return true;
}

}   // namespace files
}   // namespace los
