
#ifndef _UTIL_SCOPED_MAP_H_
#define _UTIL_SCOPED_MAP_H_


#include "globals.h"
#include "macros.h"


// A smart pointer that unmaps the given memory segments on going out of scope.
class ScopedMap
{
  public:
    explicit ScopedMap(byte* base, size_t size)
      : base_(base),
        size_(size)
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

    void transfer(ScopedMap* p_rhs)
    {
    	base_ = p_rhs->base_;
    	size_ = p_rhs->size_;
    	p_rhs->base_ = nullptr;
    	p_rhs->size_ = 0;
    }

    void reset(byte* new_base = nullptr, size_t new_size = 0)
    {
    	if (base_ != nullptr && size_ != 0)
    		munmap(base_, size_);
    	base_ = new_base;
    	size_ = new_size;
    }

private:
	byte* base_;
	size_t size_;

    DISALLOW_COPY_AND_ASSIGN(ScopedMap);
};

#endif