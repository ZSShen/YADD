
#ifndef _UTIL_MACROS_H_
#define _UTIL_MACROS_H_


// WARN_UNUSED fires a warning when programmers forget to use the return value
// from callee method.
#define WARN_UNUSED __attribute__((warn_unused_result))

// DISALLOW_COPY_AND_ASSIGN disallows the copy and operator= functions.
// It goes in the private: declarations in a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)              \
    TypeName(const TypeName&);                          \
    void operator=(const TypeName&)

#endif