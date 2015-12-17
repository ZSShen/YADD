
#ifndef _UTIL_SCOPED_MAP_H_
#define _UTIL_SCOPED_MAP_H_


#include "globals.h"
#include "macros.h"


// A smart pointer that unmaps the given memory segments on going out of scope.
class ScopedMap
{
  public:
    explicit ScopedMap(byte* base, size_t size, size_t algn_size)
      : base_(base),
        size_(size),
        algn_size_(algn_size)
    {}

    ~ScopedMap()
    {
        reset();
    }

    byte* GetBase() const
    {
        return base_;
    }

    size_t GetSize() const
    {
        return size_;
    }

    size_t GetAlignedSize() const
    {
        return algn_size_;
    }

    void release()
    {
        base_ = nullptr;
        size_ = 0;
        algn_size_ = 0;
    }

    void reset(byte* base = nullptr, size_t size = 0, size_t algn_size = 0)
    {
        if (base_ != nullptr && size_ != 0 && algn_size_ != 0)
            munmap(base_, algn_size_);
        base_ = base;
        size_ = size;
        algn_size_ = algn_size;
    }

private:
    byte* base_;
    size_t size_, algn_size_;

    DISALLOW_COPY_AND_ASSIGN(ScopedMap);
};

#endif