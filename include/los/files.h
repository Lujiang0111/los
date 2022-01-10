#ifndef LOS_INCLUDE_LOS_FILES_H_
#define LOS_INCLUDE_LOS_FILES_H_

#include "los.h"

namespace los {
namespace files {

enum Modes
{
    kUnspecified,   // unspecified
    kRegular,       // regular file
    kDirectory,     // directory
};

enum Sorts
{
    kNone = 0,      // do no sort
    kByName,        // sort by name
    kByModifyTime,  // sort by modfify time
};

class LOS_API FileInfoInterface
{
public:
    virtual ~FileInfoInterface() = default;

    virtual const char *GetName() const = 0;
    virtual const char *GetFullName() const = 0;
    virtual Modes GetMode() const = 0;
    virtual size_t GetTotalSize() const = 0;
    virtual int64_t GetModifyTimestamp() const = 0;

    virtual size_t GetChildsSize() const = 0;
    virtual const FileInfoInterface *GetChild(size_t idx) const = 0;
};

LOS_API std::shared_ptr<FileInfoInterface> GetFileInfo(const char *name, Sorts sort_type);

LOS_API bool CreateDir(const char *name, bool create_parent_only);

LOS_API bool RemoveFile(const char *name);

}   // namespace files
}   // namespace los

#endif // !LOS_INCLUDE_LOS_FILES_H_
