

#include "globals.h"
#include "scoped_fd.h"
#include "scoped_map.h"
#include "dex_file.h"


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
    size_t algn_size = (result.rem != 0)? (kPageSize * (result.quot + 1)) :
                                          (kPageSize * result.quot);

    // Map the file into memory.
    byte* base = reinterpret_cast<byte*>(mmap(nullptr, size, PROT_READ,
                                               MAP_PRIVATE, fd.get(), 0));
    if (base == MAP_FAILED) {
        ERROR("Fail to map the dex file into memory.");
        return -1;
    }

    ScopedMap mem_map(base, size, algn_size);
    std::unique_ptr<const DexFile> dex_file(DexFile::OpenMemory(mem_map));
    if (dex_file.get() == nullptr)
        return -1;

    return 0;
}