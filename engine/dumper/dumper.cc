
#include "globals.h"
#include "scoped_fd.h"
#include "scoped_map.h"
#include "dex_file.h"
#include "dex_instruction.h"
#include "stringprintf.h"


void SkipAllFields();
void DumpDexFile(std::ostream&, const DexFile&);
void DumpDexClass(std::ostream&, const DexFile&, const DexFile::ClassDef&);
void DumpDexCode(std::ostream&, const DexFile&, const DexFile::CodeItem*);


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

    DumpDexFile(std::cout, *dex_file.get());

    return 0;
}


void SkipAllFields(ClassDataItemIterator& it)
{
    while (it.HasNextStaticField())
        it.Next();
    while (it.HasNextInstanceField())
        it.Next();
}

void DumpDexFile(std::ostream& os, const DexFile& dex_file)
{
    uint32_t num_class_def = dex_file.NumClassDefs();
    for (uint32_t class_def_idx = 0 ; class_def_idx < num_class_def ; ++class_def_idx) {
        const DexFile::ClassDef& class_def = dex_file.GetClassDef(class_def_idx);
        const char* descriptor = dex_file.GetClassDescriptor(class_def);
        os << StringPrintf("%d: %s\n", class_def_idx, descriptor);
        DumpDexClass(os, dex_file, class_def);
        os << '\n';
    }
}

void DumpDexClass(std::ostream& os, const DexFile& dex_file,
                  const DexFile::ClassDef& class_def)
{
    const byte* class_data = dex_file.GetClassData(class_def);
    if (class_data == nullptr)  // empty class such as a marker interface?
        return;

    ClassDataItemIterator it(dex_file, class_data);
    SkipAllFields(it);

    uint32_t class_method_idx = 0;
    while (it.HasNextDirectMethod()) {
        uint32_t dex_method_idx = it.GetMemberIndex();
        os << StringPrintf("\t%d: %s (dex_method_idx=%d)\n", class_method_idx,
                           PrettyMethod(dex_method_idx, dex_file, true).c_str(),
                           dex_method_idx);
        const DexFile::CodeItem* code_item = it.GetMethodCodeItem();
        DumpDexCode(os, dex_file, code_item);
        os << '\n';
        it.Next();
        ++class_method_idx;
    }

    while (it.HasNextVirtualMethod()) {
        uint32_t dex_method_idx = it.GetMemberIndex();
        os << StringPrintf("\t%d: %s (dex_method_idx=%d)\n", class_method_idx,
                           PrettyMethod(dex_method_idx, dex_file, true).c_str(),
                           dex_method_idx);
        const DexFile::CodeItem* code_item = it.GetMethodCodeItem();
        DumpDexCode(os, dex_file, code_item);
        os << '\n';
        it.Next();
        ++class_method_idx;
    }
    assert(!it.HasNext());
}

void DumpDexCode(std::ostream& os, const DexFile& dex_file,
                 const DexFile::CodeItem* code_item)
{
    if (!code_item)
        return;
    size_t inst_off = 0;
    while (inst_off < code_item->insns_size_in_code_units_) {
        const Instruction* instruction = Instruction::At(&code_item->insns_[inst_off]);
        os << StringPrintf("\t\t0x%04zx: %s\n", inst_off,
                           instruction->DumpString(&dex_file).c_str());
        inst_off += instruction->SizeInCodeUnits();
    }
}