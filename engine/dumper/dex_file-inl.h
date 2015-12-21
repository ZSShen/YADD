
#ifndef _ART_DEX_FILE_INL_H_
#define _ART_DEX_FILE_INL_H_


#include "dex_file.h"


inline int32_t DexFile::GetStringLength(const StringId& string_id) const
{
    const byte* ptr = begin_ + string_id.string_data_off_;
    return DecodeUnsignedLeb128(&ptr);
}

inline const char* DexFile::GetStringDataAndUtf16Length(const StringId& string_id,
                                                        uint32_t* utf16_length) const
{
    CHECK(utf16_length != nullptr);
    const byte* ptr = begin_ + string_id.string_data_off_;
    *utf16_length = DecodeUnsignedLeb128(&ptr);
    return reinterpret_cast<const char*>(ptr);
}

inline const Signature DexFile::GetMethodSignature(const MethodId& method_id) const
{
    return Signature(this, GetProtoId(method_id.proto_idx_));
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

inline bool Signature::operator==(const Signature& rhs) const
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
        CHECK_EQ((params == nullptr), (rhs_params == nullptr));
        if (params != nullptr) {
            uint32_t params_size = params->Size();
            CHECK_EQ(params_size, rhs_params->Size());  // Parameter list size must match.
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

#endif