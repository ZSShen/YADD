
#ifndef _UTIL_SPEW_H_
#define _UTIL_SPEW_H_


#include "globals.h"


#define ERROR(...)      SpewMsg(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG(...)        SpewMsg(__VA_ARGS__)


void SpewMsg(const char*, const char*, const int32_t, const char*, ...);
void SpewMsg(const char*, ...);


#endif