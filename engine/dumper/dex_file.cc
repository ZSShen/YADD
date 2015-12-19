
#include "dex_file.h"


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

const Signature DexFile::GetMethodSignature(const MethodId& method_id) const
{
    return Signature(this, GetProtoId(method_id.proto_idx_));
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

static inline bool DexFileStringEquals(const DexFile* df1, uint32_t sidx1,
                                       const DexFile* df2, uint32_t sidx2)
{
    uint32_t s1_len;  // Note: utf16 length != mutf8 length.
    const char* s1_data = df1->StringDataAndUtf16LengthByIdx(sidx1, &s1_len);
    uint32_t s2_len;
    const char* s2_data = df2->StringDataAndUtf16LengthByIdx(sidx2, &s2_len);
    return (s1_len == s2_len) && (strcmp(s1_data, s2_data) == 0);
}

bool Signature::operator==(const Signature& rhs) const
{
    if (dex_file_ == nullptr)
        return rhs.dex_file_ == nullptr;
    if (rhs.dex_file_ == nullptr)
        return false;
    if (dex_file_ == rhs.dex_file_)
        return proto_id_ == rhs.proto_id_;

    uint32_t lhs_shorty_len;  // For a shorty utf16 length == mutf8 length.
    const char* lhs_shorty_data = dex_file_->StringDataAndUtf16LengthByIdx(proto_id_->shorty_idx_,
                                                                           &lhs_shorty_len);
    StringPiece lhs_shorty(lhs_shorty_data, lhs_shorty_len);
    {
        uint32_t rhs_shorty_len;
        const char* rhs_shorty_data =
            rhs.dex_file_->StringDataAndUtf16LengthByIdx(rhs.proto_id_->shorty_idx_,
                                                         &rhs_shorty_len);
        StringPiece rhs_shorty(rhs_shorty_data, rhs_shorty_len);
        if (lhs_shorty != rhs_shorty) {
        return false;  // Shorty mismatch.
        }
    }
    if (lhs_shorty[0] == 'L') {
        const DexFile::TypeId& return_type_id = dex_file_->GetTypeId(proto_id_->return_type_idx_);
        const DexFile::TypeId& rhs_return_type_id =
            rhs.dex_file_->GetTypeId(rhs.proto_id_->return_type_idx_);
        if (!DexFileStringEquals(dex_file_, return_type_id.descriptor_idx_,
                                 rhs.dex_file_, rhs_return_type_id.descriptor_idx_))
            return false;  // Return type mismatch.
    }
    if (lhs_shorty.find('L', 1) != StringPiece::npos) {
        const DexFile::TypeList* params = dex_file_->GetProtoParameters(*proto_id_);
        const DexFile::TypeList* rhs_params = rhs.dex_file_->GetProtoParameters(*rhs.proto_id_);
        // Both lists are empty or have contents, or else shorty is broken.
        assert((params == nullptr) == (rhs_params == nullptr));
        if (params != nullptr) {
            uint32_t params_size = params->Size();
            assert(params_size == rhs_params->Size());  // Parameter list size must match.
            for (uint32_t i = 0; i < params_size; ++i) {
                const DexFile::TypeId& param_id = dex_file_->GetTypeId(params->GetTypeItem(i).type_idx_);
                const DexFile::TypeId& rhs_param_id =
                    rhs.dex_file_->GetTypeId(rhs_params->GetTypeItem(i).type_idx_);
                if (!DexFileStringEquals(dex_file_, param_id.descriptor_idx_,
                                 rhs.dex_file_, rhs_param_id.descriptor_idx_))
                    return false;  // Parameter type mismatch.
            }
        }
    }
    return true;
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
        LOG("Duplicated field.");
}

void ClassDataItemIterator::ReadClassDataMethod()
{
    method_.method_idx_delta_ = DecodeUnsignedLeb128(&ptr_pos_);
    method_.access_flags_ = DecodeUnsignedLeb128(&ptr_pos_);
    method_.code_off_ = DecodeUnsignedLeb128(&ptr_pos_);
    if (last_idx_ != 0 && method_.method_idx_delta_ == 0)
        LOG("Duplicated method.");
}
