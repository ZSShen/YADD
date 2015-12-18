
#include "dex_file.h"
//#include "dex_file-inl.h"


const byte DexFile::kDexMagic[] = { 'd', 'e', 'x', '\n' };
const byte DexFile::kDexMagicVersion[] = { '0', '3', '5', '\0' };


/*---------------------------------------------------------------------*
 *                 Implementation for Public Functions                 *
 *---------------------------------------------------------------------*/

DexFile::~DexFile()
{}

bool DexFile::IsMagicValid(const byte* magic)
{
    return (memcmp(magic, kDexMagic, sizeof(kDexMagic)) == 0);
}

bool DexFile::IsVersionValid(const byte* magic)
{
    return (memcmp(magic, kDexMagicVersion, sizeof(kDexMagicVersion)) == 0);
}


/*---------------------------------------------------------------------*
 *                Implementation for Private Functions                 *
 *---------------------------------------------------------------------*/

DexFile::DexFile(byte* base, size_t size, ScopedMap& mem_map)
  : begin_(base),
    size_(size),
    mem_map_(base, size, mem_map.GetAlignedSize()),
    header_(reinterpret_cast<const Header*>(base)),
    string_ids_(reinterpret_cast<const StringId*>(base + header_->string_ids_off_)),
    type_ids_(reinterpret_cast<const TypeId*>(base + header_->type_ids_off_)),
    field_ids_(reinterpret_cast<const FieldId*>(base + header_->field_ids_off_)),
    method_ids_(reinterpret_cast<const MethodId*>(base + header_->method_ids_off_)),
    proto_ids_(reinterpret_cast<const ProtoId*>(base + header_->proto_ids_off_)),
    class_defs_(reinterpret_cast<const ClassDef*>(base + header_->class_defs_off_))
{
    mem_map.release();
}

const DexFile* DexFile::OpenMemory(byte* base, size_t size, ScopedMap& mem_map)
{
    std::unique_ptr<DexFile> dex_file(new DexFile(base, size, mem_map));
    if (!IsMagicValid(dex_file->header_->magic_)) {
        ERROR("Invalid DEX magic.");
        return nullptr;
    }
    if (!IsVersionValid(dex_file->header_->magic_ + 4)) {
        ERROR("Invalid DEX magic version.");
        return nullptr;
    }
    return dex_file.release();
}

