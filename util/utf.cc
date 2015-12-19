
#include "utf.h"


size_t CountModifiedUtf8Chars(const char* utf8)
{
    size_t len = 0;
    int ic;
    while ((ic = *utf8++) != '\0') {
        len++;
        if ((ic & 0x80) == 0) {
            // one-byte encoding
            continue;
        }
        // two- or three-byte encoding
        utf8++;
        if ((ic & 0x20) == 0) {
            // two-byte encoding
            continue;
        }
        // three-byte encoding
        utf8++;
    }
    return len;
}

