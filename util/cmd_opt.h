
#ifndef _UTIL_CMD_OPT_H_
#define _UTIL_CMD_OPT_H_


#include "globals.h"


static const char* kOptLongGranularity      = "granularity";
static const char* kOptLongInput            = "input";
static const char* kOptLongOutput           = "output";

static const char kOptGranularity           = 'g';
static const char kOptInput                 = 'i';
static const char kOptOutput                = 'o';

static const char* kGranularityClass        = "class";
static const char* kGranularityMethod       = "method";
static const char* kGranularityInstruction  = "instruction";

static const char kGranuCodeClass           = 'c';
static const char kGranuCodeMethod          = 'm';
static const char kGranuCodeInstruction     = 'i';

bool ParseDumperOption(int argc, char **argv,
                       char *opt_granu, char **opt_in, char **opt_out);

#endif