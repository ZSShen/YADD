
#ifndef _UTIL_UTF_H_
#define _UTIL_UTF_H_


#include "globals.h"


/*
 * All UTF-8 in art is actually modified UTF-8. Mostly, this distinction
 * doesn't matter.
 *
 * See http://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8 for the details.
 */

// Returns the number of UTF-16 characters in the given modified UTF-8 string.
size_t CountModifiedUtf8Chars(const char* utf8);

/*
 * Retrieve the next UTF-16 character from a UTF-8 string.
 *
 * Advances "*utf8_data_in" to the start of the next character.
 *
 * WARNING: If a string is corrupted by dropping a '\0' in the middle
 * of a 3-byte sequence, you can end up overrunning the buffer with
 * reads (and possibly with the writes if the length was computed and
 * cached before the damage). For performance reasons, this function
 * assumes that the string being parsed is known to be valid (e.g., by
 * already being verified). Most strings we process here are coming
 * out of dex files or other internal translations, so the only real
 * risk comes from the JNI NewStringUTF call.
 */
uint16_t GetUtf16FromUtf8(const char** utf8_data_in);

#endif