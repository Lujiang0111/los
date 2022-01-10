#ifndef LOS_INTERNAL_FILE_FILE_INFO_H_
#define LOS_INTERNAL_FILE_FILE_INFO_H_

#include <vector>

#include "cores.h"
#include "los/files.h"

namespace los {
namespace files {

class FileInfo : public FileInfoInterface
{
public:
    FileInfo();
    virtual ~FileInfo() = default;

    virtual const char *GetName() const;
    virtual const char *GetFullName() const;
    virtual Modes GetMode() const;
    virtual size_t GetTotalSize() const;
    virtual int64_t GetModifyTimestamp() const;

    virtual size_t GetChildsSize() const;
    virtual const FileInfoInterface *GetChild(size_t idx) const;

public:
    std::string name_;
    std::string full_name_;
    size_t total_size_;
    int64_t modify_timestamp_;
    Modes mode_;

    std::vector<std::shared_ptr<FileInfo>> childs_;
};

}   // namespace files
}   // namespace los

#endif // !LOS_INTERNAL_FILE_FILE_INFO_H_
