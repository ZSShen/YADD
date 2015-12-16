
#ifndef _UTIL_SCOPED_FD_H_
#define _UTIL_SCOPED_FD_H_


#include "globals.h"
#include "macros.h"


// A smart pointer that closes the given fd on going out of scope.
class ScopedFd
{
  public:
    explicit ScopedFd(int fd)
      : fd_(fd)
    {}

    ~ScopedFd()
    {
        reset();
    }

    int get() const
    {
        return fd_;
    }

    int release() WARN_UNUSED
    {
        int hold_fd = fd_;
        fd_ = -1;
        return hold_fd;
    }

    void reset(int new_fd = -1)
    {
        if (fd_ != -1)
            close(fd_);
        fd_ = new_fd;
    }

private:
    int fd_;

    DISALLOW_COPY_AND_ASSIGN(ScopedFd);
};

#endif
