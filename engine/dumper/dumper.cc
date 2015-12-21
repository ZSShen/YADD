
#include "globals.h"
#include "status.h"
#include "cmd_opt.h"

#include "scoped_fd.h"
#include "scoped_map.h"
#include "stringprintf.h"

#include "dex_file.h"
#include "dex_instruction.h"


void SkipAllFields();
void DumpDexFile(std::ostream&, const DexFile&);
void DumpDexClass(std::ostream&, const DexFile&, const DexFile::ClassDef&);
void DumpDexCode(std::ostream&, const DexFile&, const DexFile::CodeItem*);


int main(int argc, char** argv)
{
    char *opt_granu, *opt_in, *opt_out;

    if (!ParseDumperOption(argc, argv, &opt_granu, &opt_in, &opt_out))
        return kExitError;

    // Calculate the to be mapped space size.
    ScopedFd fd(open(opt_in, O_RDONLY, 0));
    size_t size = static_cast<size_t>(lseek(fd.get(), 0, SEEK_END));
    div_t result = div(size, kPageSize);
    size_t algn_size = (result.rem != 0)? (kPageSize * (result.quot + 1)) :
                                          (kPageSize * result.quot);

    // Map the file into memory.
    byte* base = reinterpret_cast<byte*>(mmap(nullptr, size, PROT_READ,
                                              MAP_PRIVATE, fd.get(), 0));
    if (base == MAP_FAILED) {
        ERROR("Fail to map the dex file into memory.");
        return kExitError;
    }

    ScopedMap mem_map(base, size, algn_size);
    std::unique_ptr<const DexFile> dex_file(DexFile::OpenMemory(mem_map));
    if (dex_file.get() == nullptr)
        return kExitError;

    if (!opt_out)
        DumpDexFile(std::cout, *dex_file.get());
    else {
        std::ofstream ofs(opt_out, std::ofstream::out);
        if (!ofs.good())
            return kExitError;
        DumpDexFile(ofs, *dex_file.get());
    }
    return kExitSucc;
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
        os << StringPrintf("%d: %s\n", class_def_idx,
                           PrettyClass(class_def_idx, dex_file).c_str());
        const DexFile::ClassDef& class_def = dex_file.GetClassDef(class_def_idx);
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