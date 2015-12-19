

#include "globals.h"
#include "scoped_fd.h"
#include "scoped_map.h"
#include "dex_file.h"
#include "dex_instruction.h"


void SkipAllFields();
bool DumpDexFile(const DexFile& dex_file);
bool DumpDexClass(const DexFile& dex_file, const DexFile::ClassDef& class_def);


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

    DumpDexFile(*dex_file.get());

    return 0;
}


void SkipAllFields(ClassDataItemIterator& it)
{
    while (it.HasNextStaticField())
        it.Next();
    while (it.HasNextInstanceField())
        it.Next();
}

bool DumpDexFile(const DexFile& dex_file)
{
    size_t num_class_def = dex_file.NumClassDefs();
    for (size_t idx = 0 ; idx < num_class_def ; ++idx) {
        const DexFile::ClassDef& class_def = dex_file.GetClassDef(idx);
        const char* descriptor = dex_file.GetClassDescriptor(class_def);
        printf("%s\n", descriptor);
        DumpDexClass(dex_file, class_def);
    }
    return true;
}

bool DumpDexClass(const DexFile& dex_file, const DexFile::ClassDef& class_def)
{
    bool rc = true;
    const byte* class_data = dex_file.GetClassData(class_def);
    if (class_data == nullptr)  // empty class such as a marker interface?
        return rc;

    ClassDataItemIterator it(dex_file, class_data);
    SkipAllFields(it);
    //uint32_t class_method_index = 0;
    while (it.HasNextDirectMethod()) {
        uint32_t method_idx = it.GetMemberIndex();
        const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
        const DexFile::ProtoId& proto_id = dex_file.GetMethodPrototype(method_id);

        const char* method_name = dex_file.GetMethodName(method_id);
        const char* rtn_type = dex_file.GetReturnTypeDescriptor(proto_id);
        const DexFile::TypeList* params = dex_file.GetProtoParameters(proto_id);

        std::stringstream str_stream;
        str_stream << '(';
        if (params) {
            uint32_t param_count = params->Size();
            for (uint32_t idx = 0 ; idx < param_count - 1 ; ++idx) {
                str_stream << dex_file.StringByTypeIdx(params->GetTypeItem(idx).type_idx_);
                str_stream << ", ";
            }
            str_stream << dex_file.StringByTypeIdx(params->GetTypeItem(param_count - 1).type_idx_);
        }
        str_stream << ')';

        printf("\t%s %s %s\n", rtn_type, method_name, str_stream.str().c_str());
        //class_method_index++;
        it.Next();
    }
    while (it.HasNextVirtualMethod()) {
        uint32_t method_idx = it.GetMemberIndex();
        const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
        const DexFile::ProtoId& proto_id = dex_file.GetMethodPrototype(method_id);

        const char* method_name = dex_file.GetMethodName(method_id);
        const char* rtn_type = dex_file.GetReturnTypeDescriptor(proto_id);
        const DexFile::TypeList* params = dex_file.GetProtoParameters(proto_id);

        std::stringstream str_stream;
        str_stream << '(';
        if (params) {
            uint32_t param_count = params->Size();
            for (uint32_t idx = 0 ; idx < param_count - 1 ; ++idx) {
                str_stream << dex_file.StringByTypeIdx(params->GetTypeItem(idx).type_idx_);
                str_stream << ", ";
            }
            str_stream << dex_file.StringByTypeIdx(params->GetTypeItem(param_count - 1).type_idx_);
        }
        str_stream << ')';

        printf("\t%s %s %s\n", rtn_type, method_name, str_stream.str().c_str());
        //class_method_index++;
        it.Next();
    }
    assert(!it.HasNext());

    return rc;
}