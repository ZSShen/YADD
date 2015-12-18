
#ifndef _UTIL_MACROS_H_
#define _UTIL_MACROS_H_


// Fire a warning when programmers forget to use the return value from callee method.
#define WARN_UNUSED __attribute__((warn_unused_result))

// Disallows the copy and operator= functions. It goes in the private:
// declarations in a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)              \
    TypeName(const TypeName&);                          \
    void operator=(const TypeName&)

// Hint compiler to generate optimized code for branch prediction.
#define LIKELY(x)       __builtin_expect((x), true)
#define UNLIKELY(x)     __builtin_expect((x), false)

// Return the number of leading zeros in x.
template<typename T>
static constexpr int CLZ(T x)
{
    return (sizeof(T) == sizeof(uint32_t))? __builtin_clz(x) : __builtin_clzll(x);
}

#endif