

#include "globals.h"
#include "scoped_fd.h"
#include "scoped_map.h"


int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("Please specify the input filename.\n");
        return -1;
    }
    const char* filename = argv[1];

    // Calculate the to be mapped space size.
    ScopedFd fd(open(filename, O_RDONLY, 0));
    size_t size = static_cast<size_t>(lseek(fd.get(), 0, SEEK_END));
    div_t result = div(size, kPageSize);
    if (result.rem != 0)
        size = kPageSize * (result.quot + 1);

    // Map the file into memory.
    byte* begin = reinterpret_cast<byte*>(mmap(nullptr, size, PROT_READ,
                                               MAP_PRIVATE, fd.get(), 0));
    if (begin == MAP_FAILED) {
        printf("Fail to map the dex file into memory.\n");
        return -1;
    }

    ScopedMap mem_map(begin, size);

    return 0;
}