
#include "dex_file.h"
#include "dex_file-inl.h"


const byte DexFile::kDexMagic[] = { 'd', 'e', 'x', '\n' };
const byte DexFile::kDexMagicVersion[] = { '0', '3', '5', '\0' };


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

std::string Signature::ToString() const
{
    if (dex_file_ == nullptr) {
        assert(proto_id_ == nullptr);
        return "<no signature>";
    }
    const DexFile::TypeList* params = dex_file_->GetProtoParameters(*proto_id_);
    std::string result;
    if (params == nullptr)
        result += "()";
    else {
        result += "(";
        for (uint32_t i = 0; i < params->Size(); ++i)
            result += dex_file_->StringByTypeIdx(params->GetTypeItem(i).type_idx_);
        result += ")";
    }
    result += dex_file_->StringByTypeIdx(proto_id_->return_type_idx_);
    return result;
}

bool Signature::operator==(const StringPiece& rhs) const
{
    if (dex_file_ == nullptr)
        return false;
    StringPiece tail(rhs);
    if (!tail.starts_with("("))
        return false;  // Invalid signature
    tail.remove_prefix(1);  // "(";
    const DexFile::TypeList* params = dex_file_->GetProtoParameters(*proto_id_);
    if (params != nullptr) {
        for (uint32_t i = 0; i < params->Size(); ++i) {
            StringPiece param(dex_file_->StringByTypeIdx(params->GetTypeItem(i).type_idx_));
            if (!tail.starts_with(param))
                return false;
            tail.remove_prefix(param.length());
        }
    }
    if (!tail.starts_with(")"))
        return false;
    tail.remove_prefix(1);  // ")";
    return tail == dex_file_->StringByTypeIdx(proto_id_->return_type_idx_);
}


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
        LOG(ERROR) << "Invalid DEX magic.";
        return nullptr;
    }
    if (!IsVersionValid(dex_file->header_->magic_ + 4)) {
        LOG(ERROR) << "Invalid DEX magic version.";
        return nullptr;
    }
    return dex_file.release();
}

// Decodes the header section from the class data bytes.
void ClassDataItemIterator::ReadClassDataHeader()
{
    assert(ptr_pos_ != nullptr);
    header_.static_fields_size_ = DecodeUnsignedLeb128(&ptr_pos_);
    header_.instance_fields_size_ = DecodeUnsignedLeb128(&ptr_pos_);
    header_.direct_methods_size_ = DecodeUnsignedLeb128(&ptr_pos_);
    header_.virtual_methods_size_ = DecodeUnsignedLeb128(&ptr_pos_);
}

void ClassDataItemIterator::ReadClassDataField()
{
    field_.field_idx_delta_ = DecodeUnsignedLeb128(&ptr_pos_);
    field_.access_flags_ = DecodeUnsignedLeb128(&ptr_pos_);
    if (last_idx_ != 0 && field_.field_idx_delta_ == 0)
        LOG(WARNING) << "Duplicated field.";
}

void ClassDataItemIterator::ReadClassDataMethod()
{
    method_.method_idx_delta_ = DecodeUnsignedLeb128(&ptr_pos_);
    method_.access_flags_ = DecodeUnsignedLeb128(&ptr_pos_);
    method_.code_off_ = DecodeUnsignedLeb128(&ptr_pos_);
    if (last_idx_ != 0 && method_.method_idx_delta_ == 0)
        LOG(WARNING) << "Duplicated field.";
}
