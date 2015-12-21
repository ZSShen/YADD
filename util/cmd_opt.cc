
#include "cmd_opt.h"


static void PrintDumperUsage()
{
    const char *usage = \
    "Usage: dumper [options]\n"
    "    Example: dumper --granularity=instruction --input=/PATH/TO/MY/DEX --output=PATH/TO/MY/TXT\n\n"
    "  --granularity=(class|method|instruction): For data granularity\n"
    "    class      : List class names only\n"
    "    method     : List method signatures only\n"
    "    instruction: Full dump\n\n"
    "  --input=<classes.dex>: Specify the input dex pathname\n\n"
    "  --output=<dump.txt>: Specify the output dump pathname\n\n";
    std::cerr << usage;
}


bool ParseDumperOption(int argc, char **argv,
                       char *opt_granu, char **opt_in, char **opt_out)
{
    struct option opts[] = {
        {kOptLongGranularity, required_argument, 0, kOptGranularity},
        {kOptLongInput, required_argument, 0, kOptInput},
        {kOptLongOutput, required_argument, 0, kOptOutput},
    };

    char order[kBlahSizeTiny];
    memset(order, 0, sizeof(char) * kBlahSizeTiny);
    sprintf(order, "%c:%c:%c:", kOptGranularity, kOptInput, kOptOutput);

    char *granu_str = nullptr;
    *opt_in = *opt_out = nullptr;
    int opt, idx_opt;
    pid_t pid_zygote = 0;
    char *sz_app = NULL, *sz_path = NULL;
    while ((opt = getopt_long(argc, argv, order, opts, &idx_opt)) != -1) {
        switch (opt) {
          case kOptGranularity:
            granu_str = optarg;
            break;
          case kOptInput:
            *opt_in = optarg;
            break;
          case kOptOutput:
            *opt_out = optarg;
            break;
          default:
            PrintDumperUsage();
            return false;
        }
    }

    if (*opt_in == nullptr) {
        PrintDumperUsage();
        return false;
    }
    if (granu_str == nullptr)
        granu_str = const_cast<char*>(kGranularityInstruction);

    if (strcmp(granu_str, kGranularityClass) == 0)
        *opt_granu = kGranuCodeClass;
    else {
        if (strcmp(granu_str, kGranularityMethod) == 0)
            *opt_granu = kGranuCodeMethod;
        else {
            if (strcmp(granu_str, kGranularityInstruction) == 0)
                *opt_granu = kGranuCodeInstruction;
            else {
                PrintDumperUsage();
                return false;
            }
        }
    }
    return true;
}