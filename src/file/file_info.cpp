#include "file/file_info.h"

namespace los {
namespace files {

FileInfo::FileInfo()
{
    total_size_ = 0;
    modify_timestamp_ = 0;
    mode_ = kUnspecified;
}

const char *FileInfo::GetName() const
{
    return name_.c_str();
}

const char *FileInfo::GetFullName() const
{
    return full_name_.c_str();
}

Modes FileInfo::GetMode() const
{
    return mode_;
}

size_t FileInfo::GetTotalSize() const
{
    return total_size_;
}

int64_t FileInfo::GetModifyTimestamp() const
{
    return modify_timestamp_;
}

size_t FileInfo::GetChildsSize() const
{
    return childs_.size();
}

const IFileInfo *FileInfo::GetChild(size_t idx) const
{
    if (idx >= childs_.size())
    {
        return nullptr;
    }
    return childs_[idx].get();
}

}   // namespace files
}   // namespace los
