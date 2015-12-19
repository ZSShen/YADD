
#ifndef _UTIL_MISC_H_
#define _UTIL_MISC_H_


#include "dex_file.h"


class DexFile;


static inline bool NeedsEscaping(uint16_t ch)
{
    return (ch < ' ' || ch > '~');
}

// Returns an ASCII string corresponding to the given UTF-8 string.
// Java escapes are used for non-ASCII characters.
std::string PrintableString(const char* utf8);

// Used to implement PrettyClass, PrettyField, PrettyMethod, and PrettyTypeOf,
// one of which is probably more useful to you.
// Returns a human-readable equivalent of 'descriptor'. So "I" would be "int",
// "[[I" would be "int[][]", "[Ljava/lang/String;" would be
// "java.lang.String[]", and so forth.
std::string PrettyDescriptor(const char* descriptor);

// Returns a human-readable signature for a field. Something like "a.b.C.f" or
// "int a.b.C.f" (depending on the value of 'with_type').
std::string PrettyField(uint32_t field_idx, const DexFile& dex_file,
                        bool with_type = true);

// Returns a human-readable signature for a method. Something like "a.b.C.m" or
// "a.b.C.m(II)V" (depending on the value of 'with_signature').
std::string PrettyMethod(uint32_t method_idx, const DexFile& dex_file,
						 bool with_signature = true);

// Returns a human-readable form of the type at an index in the specified dex file.
// Example outputs: char[], java.lang.String.
std::string PrettyType(uint32_t type_idx, const DexFile& dex_file);


#endif